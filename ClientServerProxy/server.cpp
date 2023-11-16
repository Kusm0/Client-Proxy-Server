// Server.cpp

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

const int PORT = 12345;

void printTcpHeader(const tcphdr* tcpHeader) {

    std::cout << "Source Port: " << ntohs(tcpHeader->th_sport) << "\n";
    std::cout << "Destination Port: " << ntohs(tcpHeader->th_dport) << "\n";

}


unsigned short calculateChecksum(const unsigned short* data, int length) {
    unsigned long sum = 0;

    while (length > 1) {
        sum += ntohs(*data++);
        length -= 2;
    }

    if (length > 0) {
        sum += *reinterpret_cast<const unsigned char*>(data);
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return static_cast<unsigned short>(~sum);
}

bool verifyChecksum(const tcphdr* tcpHeader, const char* payload, ssize_t payloadLength) {
    //  pseudo-header checksum
    unsigned short pseudoLength = htons(sizeof(tcphdr) + payloadLength);
    char pseudoPacket[pseudoLength];
    std::memcpy(pseudoPacket, tcpHeader, sizeof(tcphdr));
    std::memcpy(pseudoPacket + sizeof(tcphdr), payload, payloadLength);

    unsigned short pseudoChecksum = calculateChecksum(reinterpret_cast<unsigned short*>(pseudoPacket), pseudoLength);

    // Compare the calculated checksum with the received checksum
    return pseudoChecksum == tcpHeader->th_sum;
}


int main() {
    int serverSocket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket\n";
        close(serverSocket);
        return -1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        char buffer[4096] = {0};
        struct sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        ssize_t bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                                     (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesRead == -1) {
            std::cerr << "Error receiving data\n";
            continue;
        }

        tcphdr* tcpHeader = reinterpret_cast<tcphdr*>(buffer);
        char* payload = buffer + sizeof(tcphdr);

        // Verify checksum before processing
        if (!verifyChecksum(tcpHeader, payload, bytesRead - sizeof(tcphdr))) {
            std::cout << "Checksum of the packet has been changed:\n";
        }

        std::cout << "Received message from client:\n";
        std::cout << "Message: " << payload << "\n";
        printTcpHeader(tcpHeader);

      
    }

    close(serverSocket);
    return 0;
}
