#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
    serverAddress.sin_port = htons(8080);

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    std::string login, password;
    std::cout << "Login: ";
    std::cin >> login;
    std::cout << "Password: ";
    std::cin >> password;

    std::string sendData = login + ":" + password;
    send(connectSocket, sendData.c_str(), sendData.length(), 0);

    char recvbuf[1024];
    int recvbuflen = 1024;

    int bytesReceived = recv(connectSocket, recvbuf, recvbuflen, 0);
    if (bytesReceived > 0) {
        recvbuf[bytesReceived] = '\0';
        std::string response(recvbuf);

        if (response == "success") {
            std::cout << "Authorization successful!" << std::endl;
        }
        else if (response == "failure") {
            std::cout << "Invalid login or password." << std::endl;
        }
        else {
            std::cout << "Error occurred on server." << std::endl;
        }
    }

    closesocket(connectSocket);
    WSACleanup();
    return 0;
}
