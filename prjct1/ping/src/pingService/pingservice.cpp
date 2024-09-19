#include "pingservice.h"

std::mutex streamMutex{};

void check_command_line_arguments(int argc) {
    if (argc < 2) {
        throw ExceptionService("Destination address does not exist");
    }
}

void wsa_startup(WSADATA &wsaData) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        throw ExceptionService(
            "WSAStartup() in wsa_startup() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void create_socket(SOCKET &socket) {
    socket = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0,
                       WSA_FLAG_OVERLAPPED);
    if (socket == INVALID_SOCKET) {
        throw ExceptionService(
            "WSASocket() in create_socket() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void set_socket_timeout(SOCKET socket, int timeout, int timeoutType) {
    int setsockoptReturnValue = setsockopt(socket, SOL_SOCKET, timeoutType,
                                           (char *)&timeout, sizeof(timeout));
    if (setsockoptReturnValue == SOCKET_ERROR) {
        throw ExceptionService(
            "setsockopt() in set_socket_timeout() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void set_sockaddr_in(SOCKADDR_IN &address, char *hostName) {
    ZeroMemory(&address, sizeof(address));
    address.sin_family = AF_INET;
    if ((address.sin_addr.s_addr = inet_addr(hostName)) == INADDR_NONE) {
        HOSTENT *hostInfo = gethostbyname(hostName);
        if (hostInfo)
            memcpy(&(address.sin_addr), hostInfo->h_addr, hostInfo->h_length);
        else {
            throw ExceptionService(
                "gethostbyname() in set_destination_address() failed with "
                "error " +
                std::to_string(WSAGetLastError()));
        }
    }
}

void set_socket_routing(SOCKET socket) {
    //  to bypass the standard routing mechanisms

    BOOL optVal = TRUE;
    int setsockoptReturnValue = setsockopt(socket, SOL_SOCKET, SO_DONTROUTE,
                                           (char *)&optVal, sizeof(BOOL));
    if (setsockoptReturnValue == SOCKET_ERROR) {
        throw ExceptionService(
            "setsockopt() in set_socket_routing() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void ping(SOCKET &socket, SOCKADDR_IN &destinationAddress,
          char *addressString) {
    char requestPacket[1024]{}, responsePacket[1024]{};
    SOCKADDR_IN sourceAddress;
    for (int tryNumber = 1; tryNumber <= 4; tryNumber++) {
        int quit = 0;
        auto startTime = std::chrono::high_resolution_clock::now();
        for (int timeToLive = 1; timeToLive <= 128; timeToLive++) {
            set_time_to_live(socket, timeToLive);
            set_request_packet(requestPacket, sizeof(ICMPHeader),
                               timeToLive - 1);
            int sentBytes = send_request_packet(
                socket, requestPacket, sizeof(ICMPHeader), destinationAddress);
            if (sentBytes == -2) {
                if (timeToLive == 128) {
                    std::lock_guard<std::mutex> guard(streamMutex);
                    std::cout << "\ttime to send packet to " << addressString
                              << " has been exceeded" << std::endl;
                }
                continue;
            }
            int receivedBytes = receive_response_packet(socket, responsePacket,
                                                        1024, sourceAddress);
            if (receivedBytes == -2) {
                if (timeToLive == 128) {
                    std::lock_guard<std::mutex> guard(streamMutex);
                    std::cout << "\ttime to receive packet from"
                              << addressString << " has been exceeded"
                              << std::endl;
                }
                continue;
            }
            quit = parse_response_packet(responsePacket, receivedBytes,
                                         &sourceAddress, timeToLive);
            if (quit || timeToLive == 128) {
                auto finishTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> time = finishTime - startTime;
                std::chrono::milliseconds ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        finishTime - startTime);
                std::lock_guard<std::mutex> guard(streamMutex);
                std::cout << "\tReply from " << addressString
                          << ": TTL=" << timeToLive << " time=" << ms.count()
                          << "ms" << std::endl;
                break;
            }
        }
    }
}

void set_time_to_live(SOCKET socket, int timeToLive) {
    int setsockoptReturnValue = setsockopt(
        socket, IPPROTO_IP, IP_TTL, (LPSTR)&timeToLive, sizeof(timeToLive));
    if (setsockoptReturnValue == SOCKET_ERROR) {
        throw ExceptionService(
            "setsockopt() in set_time_to_live() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void set_request_packet(char *requestPacket, int dataSize, int sequenceNumber) {
    ICMPHeader *icmpHeader = (ICMPHeader *)requestPacket;
    icmpHeader->messageType = 8;
    icmpHeader->messageCode = 0;
    icmpHeader->identificator = (uint16_t)GetCurrentProcessId();
    icmpHeader->messageCheckSum = 0;
    icmpHeader->sequenceNumber = sequenceNumber;
    icmpHeader->messageCheckSum =
        calculate_check_sum((uint16_t *)requestPacket, dataSize);
}

USHORT calculate_check_sum(USHORT *data, int dataSize) {
    ULONG checkSum = 0;
    while (dataSize > 1) {
        checkSum += *data++;
        dataSize -= sizeof(unsigned short);
    }
    if (dataSize) {
        checkSum += *(unsigned char *)data;
    }
    checkSum = (checkSum >> 16) + (checkSum & 0xffff);
    checkSum += (checkSum >> 16);
    return (USHORT)(~checkSum);
}

int send_request_packet(SOCKET &socket, char *requestPacket, int dataSize,
                        SOCKADDR_IN &addressTo) {
    int sentBytes = sendto(socket, requestPacket, dataSize, 0,
                           (SOCKADDR *)&addressTo, sizeof(addressTo));
    if (sentBytes == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            return -2;
        }
        throw ExceptionService("sendto() failed with error " +
                               std::to_string(WSAGetLastError()));
    }
    return sentBytes;
}

int receive_response_packet(SOCKET &socket, char *responsePacket, int dataSize,
                            SOCKADDR_IN &addressFrom) {
    int addressFromSize = sizeof(addressFrom);
    int receivedBytes =
        recvfrom(socket, responsePacket, dataSize, 0,
                 (struct sockaddr *)&addressFrom, &addressFromSize);
    if (receivedBytes == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            return -2;
        }
        throw ExceptionService("recvfrom() failed with error " +
                               std::to_string(WSAGetLastError()));
    }
    return receivedBytes;
}

int parse_response_packet(char *responsePacket, int receivedBytes,
                          SOCKADDR_IN *addressFrom, int timeToLive) {
    IPHeader *ipHeader = (IPHeader *)responsePacket;
    USHORT ipHeaderLength = ipHeader->headerLength * 4;
    ICMPHeader *icmpHeader = (ICMPHeader *)(responsePacket + ipHeaderLength);
    std::string responseAddress = std::string(inet_ntoa(addressFrom->sin_addr));
    return !(icmpHeader->messageType);
}
