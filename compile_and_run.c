#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("ERROR opening socket");
        WSACleanup();
        exit(1);
    }

    if ((server = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    ZeroMemory(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    CopyMemory((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        perror("ERROR connecting");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    char buffer[256];
    printf("Please enter the message: ");
    if (fgets(buffer, 255, stdin) == NULL) {
        perror("ERROR reading from stdin");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    // Null-terminate the buffer
    buffer[strcspn(buffer, "\n")] = 0;

    int bytes_sent = 0;
    int bytes_to_send = strlen(buffer);
    while (bytes_sent < bytes_to_send) {
        int bytes = send(sockfd, buffer + bytes_sent, bytes_to_send - bytes_sent, 0);
        if (bytes == SOCKET_ERROR) {
            perror("ERROR writing to socket");
            closesocket(sockfd);
            WSACleanup();
            exit(1);
        }
        bytes_sent += bytes;
    }

    int n;
    ZeroMemory(buffer, 256);
    while ((n = recv(sockfd, buffer, 255, 0)) > 0) {
        printf("%.*s", n, buffer);
        if (n < 255) {
            break;
        }
    }

    if (n == SOCKET_ERROR) {
        perror("ERROR reading from socket");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
