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

    char const* const teapot = "HTTP/1.1 418 I'm a teapot\r\n\r\n";

// ##################### RESOURCES #######################
    typedef struct
    {
        char* src;
        char* type;
    } sResource_t;

    static sResource_t const resources[] = {
        {"../webpage/html/index.html", "text/html"},
        {"../webpage/css/index.css", "text/css"},
        {"../webpage/icon/index.svg", "image/svg+xml"},
        {"../webpage/js/index.js", "text/js"},
        {"../webpage/images/chris.png", "image/png"},    
    };
// ########################################################

    char recv_buffer[500];
    while (1)
    {
        int addr_len = sizeof(client_addr);
        SOCKET msg_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
        if(msg_sock == 0)
        {
            closesocket(msg_sock);
            printf("\n>>>>>>>>>>>>>> illegal client?!\n");
            continue;
        }

        int iTimeout = 1600;
        int iRet = setsockopt( msg_sock,
                        SOL_SOCKET,
                        SO_RCVTIMEO,
                        (const char *)&iTimeout,
                        sizeof(iTimeout) );

        printf("\nclient connected: \n");
        int rec = recvfrom(msg_sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
        printf("::%d, %d, %d, %d, len: %d\n", client_addr.sin_addr, client_addr.sin_family, client_addr.sin_port, client_addr.sin_zero, addr_len);
        if(rec <= 0)
        {
            closesocket(msg_sock);
            printf(">> timeout\n");
            continue;
        }

        printf("\n-------------------------------\n");
        for(int i=0; i<sizeof(recv_buffer); ++i)
        {
            printf("%c", recv_buffer[i]);
        }
        printf("\n-------------------------------\n");

        // Get http method
        char* readPointer = strtok(recv_buffer, " ");
        if(readPointer == NULL) return -1;
        if(strncmp(readPointer, "GET", 3U) != 0)
        {
            printf("teapot!\n");
            send(msg_sock, teapot, strlen(teapot), 0);
            closesocket(msg_sock);
            continue;
        }

        // Get what client wants
        readPointer = strtok(NULL, " ");
        if(readPointer == NULL) return -1;
        int get_req_len = strlen(readPointer);
        char resource_index = readPointer[1];
        printf("resource: %c\n", resource_index);

        if(resource_index < 0x30 || resource_index > 0x39)
        {
            printf("out of range 1\n");
            resource_index = 0x30;
        }

        resource_index -= 0x30;
        if(resource_index >= sizeof(resources))
        {
           printf("out of range 2!\n");
           resource_index = 0;
        }

        FILE* fp = fopen(resources[resource_index].src, "rb");
        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        int header_size = 0;
        char response_header[100];
        printf("resource index:: %d\n", resource_index);

        header_size = snprintf(response_header, sizeof(response_header),
        "HTTP/1.1 200 OK\r\n" 
        "Content-Type: %s\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n\r\n", resources[resource_index].type, (file_size)
        );

        printf("header size: %d\n", header_size);
        printf("file size: %d\n", file_size);

        int packet_size = (file_size + header_size);

        printf("allocated size: %d\n\n", (sizeof(char) * packet_size));

        char* response = (char*) malloc(sizeof(char) * packet_size);
        memcpy(response, response_header, header_size);
        for(int i=0; i<file_size; ++i)
        {
            response[i + header_size] = fgetc(fp);
        }

        fclose(fp);

        int res = 0;
        send(msg_sock, response, (file_size + header_size), 0);

        Sleep(15);

        free(response);
        closesocket(msg_sock);
    }

}