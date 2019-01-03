#include <gtest/gtest.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <functional>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void on_error(char *s, int *errCode = NULL)
{
    int err = (errCode) ? *errCode : WSAGetLastError();
    fprintf(stderr, "%s: %d\n", s, err);
    fflush(stderr);
    WSACleanup();
    FAIL() << s;
}

void printCallback(std::string name)
{
    printf("name: %s\n", name.c_str());
}

void runServer(std::function<void(std::string)> callback)
{
    // Initialize Winsock
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0)
        on_error("Error in WSAStartup", &err);

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    struct addrinfo *result = NULL;
    err = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (err != 0)
        on_error("getaddrinfo failed", &err);

    // Create a SOCKET for connecting to server
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
        on_error("socket failed");

    // Setup the TCP listening socket
    err = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (err == SOCKET_ERROR)
        on_error("bind failed");

    freeaddrinfo(result);

    err = listen(ListenSocket, SOMAXCONN);
    if (err == SOCKET_ERROR)
        on_error("listen failed");

    while (1)
    {
        // Accept a client socket
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
            on_error("accept failed");

        bool keepLooping = true;
        do
        {
            char recvbuf[DEFAULT_BUFLEN];
            int recvbuflen = DEFAULT_BUFLEN;

            int iRead = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iRead > 0)
            {
                printf("Bytes received: %d\n", iRead);
                {
                    std::string s(recvbuf, iRead);
                    std::thread([=](){
                        callback(s);
                    }).detach();
                }

                char *pBuf = recvbuf;
                do
                {
                    int iSend = send(ClientSocket, pBuf, iRead, 0);
                    if (iSend == SOCKET_ERROR)
                    {
                        on_error("send failed");
                        keepLooping = false;
                        break;
                    }

                    pBuf += iSend;
                    iRead -= iSend;
                } while (iRead > 0);
            }
            else if (iRead == 0)
            {
                printf("Connection closing...\n");
                break;
            }
            else
            {
                on_error("recv failed");
                break;
            }

        } while (keepLooping);

        closesocket(ClientSocket);
    }

    WSACleanup();
}

// https://docs.microsoft.com/en-us/windows/desktop/winsock/complete-server-code
// https://stackoverflow.com/questions/31461531/winsock-echo-server-port
TEST(winsock, server)
{
    runServer(printCallback);
}
