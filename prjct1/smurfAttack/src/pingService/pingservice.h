#pragma once

#include <stdint.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>

#include "../exceptionService/exceptionservice.h"


#pragma comment(lib, "ws2_32.lib")

struct IPHeader {
    unsigned char version_IHL;  // Версия и длина заголовка
    unsigned char typeOfService;    // Тип сервиса
    unsigned short totalLength;     // Полная длина пакета
    unsigned short identification;  // Идентификатор
    unsigned short flags_fragmentOffset;  // Флаги и смещение фрагмента
    unsigned char timeToLive;  // Время жизни (TTL)
    unsigned char protocol;    // Протокол (TCP, UDP и т.д.)
    unsigned short headerChecksum;  // Контрольная сумма заголовка
    unsigned long sourceAddress;       // IP-адрес отправителя
    unsigned long destinationAddress;  // IP-адрес получателя
};

struct ICMPHeader {
    uint8_t messageType;       // 8 bits
    uint8_t messageCode;       // 8 bits
    uint16_t messageCheckSum;  // 16 bits
    uint16_t identificator;    // 16 bits
    uint16_t sequenceNumber;   // 16 bits
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