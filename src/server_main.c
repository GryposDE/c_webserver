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

    while (1)
    {
        int addr_len = sizeof(client_addr);
        SOCKET msg_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
        if(msg_sock == 0)
        {
            printf("illegal client?!\n");
            continue;
        }

        FILE* fp = fopen("../html/index.html", "rb");
        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        char response_header[100];
        int header_size = snprintf(response_header, sizeof(response_header),
        "HTTP/1.1 200 OK\r\n" 
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n\r\n", (file_size)
        );

        printf("header size: %d\n", header_size);
        printf("file size: %d\n", file_size);

        int packet_size = (file_size + header_size);

        printf("allocated size: %d\n", (sizeof(char) * packet_size));

        char* response = (char*) malloc(sizeof(char) * packet_size);
        memcpy(response, response_header, header_size);
        for(int i=0; i<file_size; ++i)
        {
            response[i + header_size] = fgetc(fp);
        }

        fclose(fp);

        int res = 0;
        send(msg_sock, response, (file_size + header_size), 0);

        Sleep(1);

        free(response);
        closesocket(msg_sock);
    }

}