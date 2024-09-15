#include "threadservice.h"

void run_thread(int argc, char** argv) {
    SOCKET socket = INVALID_SOCKET;
    SOCKADDR_IN sourceAddress;
    try {
        check_command_line_arguments(argc);
        WSADATA wsaData;
        wsa_startup(wsaData);
        create_socket(socket);
        set_formating_ip_header(socket);
        set_broadcast_option(socket);
        set_socket_timeout(socket, 1000, SO_SNDTIMEO);
        SOCKADDR_IN sourceAddress;
        set_sockaddr_in(sourceAddress, argv[1]);
        SOCKADDR_IN destinationAddress;
        set_sockaddr_in(destinationAddress, argv[2]);
        std::string packetAmountForThreadString = std::string(argv[3]);
        int packetAmountForThread = std::stoi(packetAmountForThreadString);
        std::cout << "\nPing to " << argv[1] << "\n";
        ping(socket, sourceAddress, destinationAddress, 0);
        std::cout << "\nPing finished\n";
    } catch (ExceptionService exceptionService) {
        std::cerr << exceptionService.what() << std::endl;
    } catch (const std::exception& exception) {
        std::cerr << "\nStandard exception: " << exception.what() << "\n";
    } catch (...) {
        std::cerr << "\nSomething went wrong\n";
    }
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
    }
    WSACleanup();
    std::cout << "\nThread is joinable\n";
}