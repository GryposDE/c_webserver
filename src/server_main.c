#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int addr_len;
    struct sockaddr_in local, client_addr;

    SOCKET sock, msg_sock;
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
    {
        printf("WSAStartup()\n");
        return -1;
    }

    // Fill in the address structure
    local.sin_family        = AF_INET;
    local.sin_addr.s_addr   = INADDR_ANY;
    local.sin_port          = htons(80);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == INVALID_SOCKET)
    {
        printf("socket()\n");
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind()\n");
        return -1;
    }

    // ----

    static int counter = 0;

    if (listen(sock, 10) == SOCKET_ERROR)
    {
        printf("listen\n");
        return -1;
    }


    static char response[] = 
    "HTTP/1.1 200 OK\n"
    "Content-Type: text/html\n"
    "Content-Length: 10\n"
    "\n"
    "<h1>0</h1>";

    while (1)
    {
        int addr_len = sizeof(client_addr);
        SOCKET msg_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);

        response[sizeof(response)-7] = (0x30 + counter);
        if(++counter > 9)
        {
            counter = 0;
        }

        printf("send response! %d\n", counter);

        if(send(msg_sock, response, sizeof(response), 0) == 0)
        {
            closesocket(msg_sock);
        }

        // Sleep(10);

    }

}