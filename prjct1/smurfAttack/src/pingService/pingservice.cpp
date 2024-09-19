#include "pingservice.h"

#include <synchapi.h>
#include <winsock2.h>

#include <cstdint>

void check_command_line_arguments(int argc) {
    if (argc < 4) {
        std::string exceptionInfo =
            "There are not enough command line arguments: ";
        std::string exceptionReasons = "";
        switch (argc) {
            // case 4:
            //     exceptionReasons = ", packet amount for each thread";
            case 3:
                exceptionReasons = ", thread amount" + exceptionReasons;
            case 2:
                exceptionReasons = ", destination address" + exceptionReasons;
            case 1:
                exceptionReasons = "source address" + exceptionReasons;
        }
        throw ExceptionService(exceptionInfo + exceptionReasons);
    }
}

void wsa_startup(WSADATA& wsaData) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        throw ExceptionService(
            "WSAStartup() in wsa_startup() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void create_socket(SOCKET& _socket) {
    _socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (_socket == INVALID_SOCKET) {
        throw ExceptionService(
            "socket() in create_socket() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void set_formating_ip_header(SOCKET& socket) {
    int opt = 1;
    if (setsockopt(socket, IPPROTO_IP, IP_HDRINCL, (const char*)&opt,
                   sizeof(opt)) == SOCKET_ERROR) {
        throw ExceptionService(
            "setsockopt() in set_formating_ip_header() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void set_broadcast_option(SOCKET socket) {
    BOOL optval = TRUE;
    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&optval,
                   sizeof(optval)) == SOCKET_ERROR) {
        throw ExceptionService(
            "setsockopt() in set_broadcast_option() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void set_socket_timeout(SOCKET socket, int timeout, int timeoutType) {
    int setsockoptReturnValue = setsockopt(socket, SOL_SOCKET, timeoutType,
                                           (char*)&timeout, sizeof(timeout));
    if (setsockoptReturnValue == SOCKET_ERROR) {
        throw ExceptionService(
            "setsockopt() in set_socket_timeout() failed with error " +
            std::to_string(WSAGetLastError()));
    }
}

void set_sockaddr_in(SOCKADDR_IN& address, char* hostName) {
    ZeroMemory(&address, sizeof(address));
    address.sin_family = AF_INET;
    if ((address.sin_addr.s_addr = inet_addr(hostName)) == INADDR_NONE) {
        throw ExceptionService(
            "inet_addr() in set_destination_address() failed with "
            "error " +
            std::to_string(WSAGetLastError()));
    }
}

void ping(SOCKET& socket, SOCKADDR_IN& sourceAddress,
          SOCKADDR_IN& destinationAddress, int packetAmount) {
    char requestPacket[sizeof(struct IPHeader) + sizeof(struct ICMPHeader)]{};

    set_request_packet(requestPacket, sourceAddress, destinationAddress);
    while (1) {
        int sentBytes = send_request_packet(
            socket, requestPacket, sizeof(IPHeader) + sizeof(ICMPHeader),
            destinationAddress);
        if (sentBytes == -2) {
            std::cout << "\ttime to send packet has been exceeded\n";
        }
        Sleep(100);
    }
}

void set_request_packet(char* requestPacket, SOCKADDR_IN& sourceAddress,
                        SOCKADDR_IN& destinationAddress) {
    memset(requestPacket, 0, sizeof(*requestPacket));

    struct IPHeader* ipHeader = (struct IPHeader*)requestPacket;
    struct ICMPHeader* icmpHeader =
        (struct ICMPHeader*)(requestPacket + sizeof(struct IPHeader));

    memset(ipHeader, 0, sizeof(IPHeader));
    ipHeader->versionAndHeaderLength =
        (4 << 4) |
        (sizeof(IPHeader) / sizeof(unsigned long));  // IPv4 и длина заголовка
    ipHeader->typeOfService = 0;  // Обычный тип сервиса
    ipHeader->totalLength = htons(sizeof(IPHeader));  // Полная длина пакета
    ipHeader->identification = htons(54321);  // Идентификатор пакета
    ipHeader->flagsAndFragmentOffset =
        0;  // Флаги (DF = 0, MF = 0) и смещение фрагмента
    ipHeader->timeToLive = 128;         // Время жизни (TTL)
    ipHeader->protocol = IPPROTO_ICMP;  // Протокол ICMP
    ipHeader->sourceAddress = sourceAddress.sin_addr.s_addr;
    ipHeader->destinationAddress = destinationAddress.sin_addr.s_addr;
    ipHeader->headerChecksum = calculate_check_sum(
        (unsigned short*)&ipHeader,
        sizeof(IPHeader));  // Рассчитываем контрольную сумму

    memset(icmpHeader, 0, sizeof(ICMPHeader));
    icmpHeader->messageType = 8;  // ICMP Echo Request
    icmpHeader->messageCode = 0;  // Код сообщения
    icmpHeader->identificator =
        htons((uint16_t)GetCurrentProcessId());  // Идентификатор
    icmpHeader->sequenceNumber = htons(1);  // Порядковый номер
    icmpHeader->messageCheckSum =
        calculate_check_sum((uint16_t*)icmpHeader, sizeof(ICMPHeader));
}

uint16_t calculate_check_sum(uint16_t* data, int dataSize) {
    uint32_t checkSum = 0;
    while (dataSize > 1) {
        checkSum += *data++;
        dataSize -= sizeof(uint16_t);
    }
    if (dataSize == 1) {
        checkSum += htons(*(uint8_t*)data);
    }
    checkSum = (checkSum >> 16) + (checkSum & 0xffff);
    checkSum += (checkSum >> 16);
    return (uint16_t)(~checkSum);
}

int send_request_packet(SOCKET& socket, char* requestPacket,
                        int requestPacketSize, SOCKADDR_IN& addressTo) {
    int sentBytes = sendto(socket, requestPacket, requestPacketSize, 0,
                           (SOCKADDR*)&addressTo, sizeof(addressTo));
    if (sentBytes == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAETIMEDOUT) {
            return -2;
        }
        throw ExceptionService("sendto() failed with error " +
                               std::to_string(WSAGetLastError()));
    }
    return sentBytes;
}