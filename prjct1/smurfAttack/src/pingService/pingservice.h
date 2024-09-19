#pragma once

#include <stdint.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>

#include "../exceptionService/exceptionservice.h"

#pragma comment(lib, "ws2_32.lib")

struct IPHeader {
    uint8_t versionAndHeaderLength;
    uint8_t typeOfService;
    uint16_t totalLength;
    uint16_t identification;
    uint16_t flagsAndFragmentOffset;
    uint8_t timeToLive;
    uint8_t protocol;
    uint16_t headerChecksum;
    uint32_t sourceAddress;
    uint32_t destinationAddress;
};

struct ICMPHeader {
    uint8_t messageType;
    uint8_t messageCode;
    uint16_t messageCheckSum;
    uint16_t identificator;
    uint16_t sequenceNumber;
};

void check_command_line_arguments(int);

void wsa_startup(WSADATA &);

void create_socket(SOCKET &);

void set_formating_ip_header(SOCKET &);

void set_broadcast_option(SOCKET);

void set_socket_timeout(SOCKET, int, int);

void set_sockaddr_in(SOCKADDR_IN &, char *);

void ping(SOCKET &, SOCKADDR_IN &, SOCKADDR_IN &, int);

void set_request_packet(char *, SOCKADDR_IN &, SOCKADDR_IN &);

USHORT calculate_check_sum(uint16_t *, int);

int send_request_packet(SOCKET &, char *, int, SOCKADDR_IN &);