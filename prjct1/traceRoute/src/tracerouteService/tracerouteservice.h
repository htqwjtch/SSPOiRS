#include <stdint.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <chrono>
#include <iomanip>
#include <map>
#include <mutex>
#include <string>

#include "../exceptionService/exceptionservice.h"

#pragma comment(lib, "ws2_32.lib")

extern std::mutex streamMutex;

struct IPHeader {
    uint8_t headerLength : 4;
    uint8_t ipVersion : 4;
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

void set_socket_timeout(SOCKET, int, int);

void set_sockaddr_in(SOCKADDR_IN &, char *);

void set_socket_routing(SOCKET);

void traceroute(SOCKET &, SOCKADDR_IN &, char *);

void set_time_to_live(SOCKET, int);

void set_request_packet(char *, int, int);

uint16_t calculate_check_sum(uint16_t *, int);

int send_request_packet(SOCKET &, char *, int, SOCKADDR_IN &);

int receive_response_packet(SOCKET &, char *, int, SOCKADDR_IN &);

int parse_response_packet(char *, int, SOCKADDR_IN *, int, char *, long long);