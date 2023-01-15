#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>
#include<ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")


int main(int argc, char** argv) {

    //get file size
    FILE* fileHandle;
    size_t fileSize;
    fopen_s(&fileHandle, argv[1], "rb");
    if (fileHandle == NULL) {
        fprintf(stderr, "FILE READING FAILURE");
    }
    fseek(fileHandle, 0, SEEK_END);
    fileSize = ftell(fileHandle);
    
    //allocate buffer, write into it, and close file handle
    void* fileBuffer = malloc(fileSize + 1);
    fread(fileBuffer, fileSize, 1, fileHandle);
    fclose(fileHandle);

    //init winsock
    WSADATA winSocketData;
    int startupResult;
    startupResult = WSAStartup(MAKEWORD(2, 2), &winSocketData);
    if (startupResult != 0) {
        fprintf(stderr, "STARTUP ERROR");
        WSACleanup();
    }

    //get addr info
    struct addrinfo* result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int addrInfoResult = getaddrinfo("127.0.0.1", "7777", &hints, &result);
    if (addrInfoResult != 0) {
        fprintf(stderr, "INVALID ADDRINFO");
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //init socket
    SOCKET bindSock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (bindSock == INVALID_SOCKET) {
        fprintf(stderr, "INVALID SOCKET");
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //bind socket
    printf("BINDING SOCKET...");
    int bindResult = bind(bindSock, result->ai_addr, (int)result->ai_addrlen);
    if (bindResult == SOCKET_ERROR) {
        fprintf(stderr, "BIND FAILURE");
        closesocket(bindSock);
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    
    }
    freeaddrinfo(result);

    //accept connection
    SOCKET connectionSock;
    struct sockaddr_in clientInfo;
    int clientInfoSize = sizeof(clientInfo);
    while (1) {
        connectionSock = accept(bindSock, (struct sockaddr*)&clientInfo, &clientInfoSize);
        if (connectionSock == INVALID_SOCKET) {
            fprintf(stderr, "CONNECTION FAILURE");
        }
        else {
            printf("CONNECTED TO CHARON INSTANCE");
            //send file length
            send(connectionSock, (char*)&fileSize, sizeof(fileSize), 0);

            //ack the ack
            size_t ackSize;
            int ackSizeResult = recv(connectionSock, (char*)&ackSize, sizeof(ackSize), 0);
            if (ackSizeResult != 0) {
                printf("SIZE ACK FAILURE");
                closesocket(connectionSock);
                break;
            }

            //verify ack
            if (ackSize != fileSize) {
                fprintf(stderr, "ACK SIZE MISMATCH");
                closesocket(connectionSock);
                break;
            }

            //send code
            int sendResult;
            printf("SENDING CODE");
            sendResult = send(connectionSock, (char*)&fileBuffer, fileSize, 0);
            if (sendResult != (int)fileSize) {
                fprintf(stderr, "SEND FAILURE");
                closesocket(connectionSock);
                break;
            }
        }
        
    }
    
    //cleanup and exit
    closesocket(connectionSock);
    closesocket(bindSock);
    free(fileBuffer);
    WSACleanup();

    return 0;
}