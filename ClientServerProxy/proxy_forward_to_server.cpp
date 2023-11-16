#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

const int SERVER_PORT = 12345;
const int PROXY_PORT = 54321;

//TCP header structure
struct CustomTcpHeader {
    u_int16_t th_sport; /* source port */
    u_int16_t th_dport; /* destination port */
    u_int32_t th_seq;   /* sequence number */
    u_int32_t th_ack;   /* acknowledgement number */
    u_int8_t th_off;    /* data offset, rsvd */
    u_int8_t th_flags;  /* control flags */
    u_int16_t th_win;   /* window */
    u_int16_t th_sum;   /* checksum */
    u_int16_t th_urp;   /* urgent pointer */
};

void forwardPacket(int proxySocket, const sockaddr_in& serverAddr, const char* buffer, ssize_t bytesRead) {
    ssize_t bytesSent = sendto(proxySocket, buffer, bytesRead, 0,
                               (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if (bytesSent == -1) {
        std::cerr << "Error forwarding data to server\n";
    }
}

int main() {
    int proxySocket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (proxySocket == -1) {
        std::cerr << "Error creating proxy socket\n";
        return -1;
    }

    sockaddr_in proxyAddr{};
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(PROXY_PORT);
    proxyAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(proxySocket, (struct sockaddr*)&proxyAddr, sizeof(proxyAddr)) == -1) {
        std::cerr << "Error binding proxy socket\n";
        close(proxySocket);
        return -1;
    }

    std::cout << "Proxy listening on port " << PROXY_PORT << "...\n";

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (true) {
        char buffer[4096] = {0};
        struct sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        ssize_t bytesRead = recvfrom(proxySocket, buffer, sizeof(buffer), 0,
                                     (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesRead == -1) {
            std::cerr << "Error receiving data in proxy\n";
            continue;
        }

        CustomTcpHeader* tcpHeader = reinterpret_cast<CustomTcpHeader*>(buffer);

        // Check if the source port matches the port used by Clientt
        if (ntohs(tcpHeader->th_sport) == 12345) {
            std::cout << "Proxy received message from client:\n";
            forwardPacket(proxySocket, serverAddr, buffer, bytesRead);
        }
    }

    close(proxySocket);
    return 0;
}
