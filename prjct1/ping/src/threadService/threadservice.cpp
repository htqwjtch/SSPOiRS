#include "threadservice.h"

void run_thread(int argc, char** argv, int threadIndex) {
    SOCKET socket = INVALID_SOCKET;
    try {
        check_command_line_arguments(argc);
        WSADATA wsaData;
        wsa_startup(wsaData);
        create_socket(socket);
        set_socket_timeout(socket, 1000, SO_SNDTIMEO);
        set_socket_timeout(socket, 1000, SO_RCVTIMEO);
        SOCKADDR_IN destinationAddress;
        set_sockaddr_in(destinationAddress, argv[threadIndex]);
        ping(socket, destinationAddress, argv[threadIndex]);
    } catch (ExceptionService exceptionService) {
        std::lock_guard<std::mutex> guard(streamMutex);
        std::cerr << exceptionService.what() << std::endl;
    } catch (const std::exception& exception) {
        std::lock_guard<std::mutex> guard(streamMutex);
        std::cerr << "\nStandard exception: " << exception.what() << "\n";
    } catch (...) {
        std::lock_guard<std::mutex> guard(streamMutex);
        std::cerr << "\nSomething went wrong\n";
    }
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
    }
    WSACleanup();
}