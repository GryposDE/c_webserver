#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>




// ##################### RESOURCES #######################
    char const* const teapot = "HTTP/1.1 418 I'm a teapot\r\n\r\n";

    typedef struct
    {
        char* src;
        char* type;
    } sResource_t;

    typedef struct
    {
        sResource_t const* const psResource;
        uint32_t u32Size;
    } sResourceEntry_t;

    static sResource_t const html_src[] = {
        {"../webpage/html/index.html", "text/html"},
        {"../webpage/html/projects.html", "text/html"},
    };

    static sResource_t const css_src[] = {
        {"../webpage/css/index.css", "text/css"},
        {"../webpage/css/projects.css", "text/css"},
    };

    static sResource_t const js_src[] = {
        {"../webpage/js/index.js", "text/js"},
        {"../webpage/js/projects.js", "text/js"},
    };

    static sResource_t const image_src[] = {
        {"../webpage/icon/index.svg", "image/svg+xml"},
        {"../webpage/images/chris.png", "image/png"},    
    };

    static sResourceEntry_t const psResources[] = {
        {html_src,  (sizeof(html_src) /sizeof(*html_src))  },
        {css_src,   (sizeof(css_src)  /sizeof(*css_src))   },
        {js_src,    (sizeof(js_src)   /sizeof(*js_src))    },
        {image_src, (sizeof(image_src)/sizeof(*image_src)) },
    };
// ########################################################

sResource_t const* const psGetResource(uint8_t u8ResourceType, uint8_t u8ResourceId);
int8_t s8CheckRequest(uint8_t u8ResourceType, uint8_t u8ResourceId);
int8_t s8CheckFileType(uint8_t u8ResourceType);


int main(int argc, char **argv)
{
    for(int i=0; i<(sizeof(psResources)/sizeof(*psResources)); ++i)
    {
        printf("psResources[%d] entries: %d\n", i, psResources[i].u32Size);
    }

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

    char recv_buffer[500];
    printf("Starting server...\n");
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

        printf("\nclient connected: \n");
        uint32_t const u32ConnectionTimeout = 1600;
        setsockopt(msg_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&u32ConnectionTimeout, sizeof(u32ConnectionTimeout));
        if(recvfrom(msg_sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr*)&client_addr, &addr_len) <= 0)
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
        int request_length = strlen(readPointer);
        if(request_length < 3U)
        {
            printf("resource not found: respond with index.html\n");
            readPointer[1] = 0x30;
            readPointer[2] = 0x30;
        }

        sResource_t const* const psFileRequest = psGetResource(readPointer[1], readPointer[2]);
        if(psFileRequest == NULL)
        {
            printf("teapot2!\n");
            send(msg_sock, teapot, strlen(teapot), 0);
            closesocket(msg_sock);
            continue;
        }

        FILE* fp = fopen(psFileRequest->src, "rb");
        fseek(fp, 0, SEEK_END);
        int file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        uint32_t header_size = 0;
        char pResponseHeader[100];

        header_size = snprintf(pResponseHeader, sizeof(pResponseHeader),
        "HTTP/1.1 200 OK\r\n" 
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n\r\n", psFileRequest->type, (file_size)
        );

        printf("header size: %d\n", header_size);
        printf("file size: %d\n", file_size);

        int packet_size = (file_size + header_size);
        printf("allocated size: %d\n\n", (sizeof(char) * packet_size));

        char* pResponse = (char*) malloc(sizeof(char) * packet_size);
        memcpy(pResponse, pResponseHeader, header_size);
        for(int i=0; i<file_size; ++i)
        {
            pResponse[i + header_size] = fgetc(fp);
        }
        fclose(fp);

        send(msg_sock, pResponse, (file_size + header_size), 0);
        Sleep(10);

        free(pResponse);
        closesocket(msg_sock);
    }

}


sResource_t const* const psGetResource(uint8_t u8ResourceType, uint8_t u8ResourceId)
{
    if(s8CheckRequest(u8ResourceType, u8ResourceId) != 0)
    {
        return NULL;
    }
    u8ResourceType -= 0x30;
    u8ResourceId   -= 0x30;

    printf(">> psResources[%d].psResource[%d] = %s\n", u8ResourceType, u8ResourceId, psResources[u8ResourceType].psResource[u8ResourceId].src);
    return &psResources[u8ResourceType].psResource[u8ResourceId];
}


int8_t s8CheckRequest(uint8_t u8ResourceType, uint8_t u8ResourceId)
{
    if(s8CheckFileType(u8ResourceType) != 0)
    {
        return -1;
    }
    u8ResourceType -= 0x30;

    // check for illegal id
    if(u8ResourceId < 0x30 || u8ResourceId > 0x7A)
    {
        return -1;
    }
    u8ResourceId -= 0x30;

    // check for boundaries
    if(u8ResourceId >= psResources[u8ResourceType].u32Size)
    {
        return -1;
    }

    return 0;
}

int8_t s8CheckFileType(uint8_t u8ResourceType)
{
    // check for illegal type
    if(u8ResourceType < 0x30 || u8ResourceType > 0x7A)
    {
        return -1;
    }
    u8ResourceType -= 0x30;

    // check for boundaries
    if(u8ResourceType >= (sizeof(psResources)/sizeof(*psResources)))
    {
        return -1;
    }

    return 0;
}


