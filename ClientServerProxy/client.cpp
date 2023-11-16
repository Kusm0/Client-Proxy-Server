//client.cpp

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int PORT = 12345;

int main() {
    int clientSocket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (true) {
        std::string message;
        std::cout << "Enter message to send (or 'exit' to quit): ";
        std::getline(std::cin, message);

        if (message == "exit") {
            break;
        }

        ssize_t bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0,
                                   (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        if (bytesSent == -1) {
            std::cerr << "Error sending data\n";
            continue;
        }

        std::cout << "Message sent to server\n";
    }

    close(clientSocket);
    return 0;
}
