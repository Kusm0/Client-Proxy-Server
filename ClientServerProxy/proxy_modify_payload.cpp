#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// checksum calculation
unsigned short calculateChecksum(unsigned short *buf, int nwords) {
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main() {
    int proxySocket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (proxySocket == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in proxyAddr{};
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(9090);
    proxyAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(proxySocket, (struct sockaddr*)&proxyAddr, sizeof(proxyAddr)) == -1) {
        perror("bind");
        close(proxySocket);
        return 1;
    }

    int serverSocket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (serverSocket == -1) {
        perror("socket");
        close(proxySocket);
        return 1;
    }

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;

    while (true) {
        char buffer[4096];
        struct sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);

        ssize_t packetSize = recvfrom(proxySocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);

        struct iphdr* ipHeader = (struct iphdr*)buffer;
        struct tcphdr* tcpHeader = (struct tcphdr*)(buffer + sizeof(struct iphdr));
        char* payload = buffer + sizeof(struct iphdr) + tcpHeader->th_off * 4;

        if (packetSize > sizeof(struct iphdr) + tcpHeader->th_off * 4) {
            // Modify payload
            payload[0] = 'X';

            // Recalculate  checksum
            tcpHeader->th_sum = 0;
            tcpHeader->th_sum = calculateChecksum((unsigned short*)tcpHeader, tcpHeader->th_off * 4 + packetSize - sizeof(struct iphdr) - tcpHeader->th_off * 4);
        }

        serverAddr.sin_port = tcpHeader->th_dport;
        serverAddr.sin_addr.s_addr = ipHeader->daddr;

        if (ntohs(tcpHeader->th_sport) == 12345) {
            // Forward the modified packet to the server
            sendto(serverSocket, buffer, packetSize, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        }
    }

    close(serverSocket);
    close(proxySocket);
    return 0;
}
