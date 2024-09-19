#include "threadservice.h"

void run_thread(int argc, char **argv, int threadIndex) {
    SOCKET socket = INVALID_SOCKET;
    try {
        check_command_line_arguments(argc);
        WSADATA wsaData;
        wsa_startup(wsaData);
        create_socket(socket);
        set_socket_timeout(socket, 1000, SO_RCVTIMEO);
        set_socket_timeout(socket, 1000, SO_SNDTIMEO);
        SOCKADDR_IN destinationAddress;
        set_sockaddr_in(destinationAddress, argv[threadIndex]);
        set_socket_routing(socket);
        traceroute(socket, destinationAddress, argv[threadIndex]);
    } catch (ExceptionService exceptionService) {
        std::cerr << exceptionService.what() << std::endl;
    } catch (const std::exception &exception) {
        std::cerr << "Standard exception: " << exception.what() << std::endl;
    } catch (...) {
        std::cerr << "Something went wrong" << std::endl;
    }
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
    }
    WSACleanup();
}