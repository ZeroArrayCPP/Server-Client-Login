#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

std::map<std::string, std::string> loadCredentials(const std::string& filename) {
    std::ifstream file(filename);
    std::map<std::string, std::string> credentials;
    std::string line;
    while (std::getline(file, line)) {
        size_t delimiter = line.find(':');
        if (delimiter != std::string::npos) {
            std::string login = line.substr(0, delimiter);
            std::string password = line.substr(delimiter + 1);
            credentials[login] = password;
        }
    }
    return credentials;
}

int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(8080);

    if (bind(listenSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::map<std::string, std::string> credentials = loadCredentials("login.txt");

    SOCKET clientSocket;
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);

    while ((clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddress, &clientAddressSize))) {
        char recvbuf[1024];
        int recvbuflen = 1024;

        int bytesReceived = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (bytesReceived > 0) {
            recvbuf[bytesReceived] = '\0';

            std::string receivedData(recvbuf);
            size_t delimiter = receivedData.find(':');
            if (delimiter != std::string::npos) {
                std::string login = receivedData.substr(0, delimiter);
                std::string password = receivedData.substr(delimiter + 1);

                if (credentials.find(login) != credentials.end() && credentials[login] == password) {
                    send(clientSocket, "success", 7, 0);
                }
                else {
                    send(clientSocket, "failure", 7, 0);
                }
            }
            else {
                send(clientSocket, "error", 5, 0);
            }
        }
        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
