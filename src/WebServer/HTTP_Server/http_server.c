


#include "http_server.h"
#include <assert.h>
#include <winsock2.h>
// #include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char* szResourceId;

} sHTTP_GET_Header_t;


typedef union
{
    sHTTP_GET_Header_t GET_HEADER;

} uHTTP_Header_t;


static eServer_Status eProcessRequest(sHTTP_Server_t* const psHTTP_Server, sWebClient_t* const psWebClient, uint8_t* const szRawData, uint32_t u32DataLength);
static eServer_Status eGetResource(sHTTP_Server_t* const psHTTP_Server, char const* const szResourceId, sHTTP_Resource_t** psRequestedResource);
static eServer_Status eSendResource(sWebClient_t* const psWebClient, sHTTP_Resource_t const* psRequestedResource);


eServer_Status eHTTP_Server_Init(sHTTP_Server_t* const psHTTP_Server, sHTTP_Server_Config_t* const psConfig)
{
    assert(psHTTP_Server);
    assert(psConfig);
    assert(psConfig->server_ip);
    assert(psConfig->psResources);
    assert(psConfig->u32ResourceCount);

    psHTTP_Server->psResources = psConfig->psResources;
    psHTTP_Server->u32ResourceCount = psConfig->u32ResourceCount;
    psHTTP_Server->base.args = (void*)psHTTP_Server;

    if(eWebServer_Init(&psHTTP_Server->base, psConfig->server_ip, psConfig->server_port) != SERVER_OK)
    {
        return SERVER_FAIL;
    }

    return SERVER_OK;
}


eServer_Status eHTTP_Server_Start(sHTTP_Server_t* const psHTTP_Server)
{
    return eWebServer_Start(&psHTTP_Server->base);
}


eServer_Status eHTTP_Server_Process(sHTTP_Server_t* const psHTTP_Server, uint32_t u32ReceiveDataTimeout)
{
    return eWebServer_Process(&psHTTP_Server->base, u32ReceiveDataTimeout);
}


static eServer_Status eProcessRequest(sHTTP_Server_t* const psHTTP_Server, sWebClient_t* const psWebClient, uint8_t* const szRawData, uint32_t u32DataLength)
{
    if(u32DataLength == 0)
    {
        return SERVER_FAIL;
    }

    char* szRequestMethod = strtok((char*) szRawData, " ");
    if(szRequestMethod == NULL)
    {
        return SERVER_FAIL;
    }

    if(strncmp(szRequestMethod, "GET", 3) == 0)
    {
        // should return the requested resource
        szRequestMethod = strtok(NULL, " ");
        if( (szRequestMethod == NULL) || (*szRequestMethod != '/') )
        {
            return SERVER_FAIL;
        }
        printf(">> %s\n", szRequestMethod);

        sHTTP_Resource_t* psRequestedResource;
        if(eGetResource(psHTTP_Server, &szRequestMethod[1], &psRequestedResource) != SERVER_OK)
        {
            printf("> no resource found, send standard http page!\n");
        }
        printf(">> return resource: %s\n", psRequestedResource->szResourceId);

        return eSendResource(psWebClient, psRequestedResource);
    }
    // else if(strncmp(szRequestMethod, "POST", 4) == 0)
    // {

    // }
    // else if(strncmp(szRequestMethod, "PUT", 3) == 0)
    // {

    // }
    // else if(strncmp(szRequestMethod, "DELETE", 6) == 0)
    // {

    // }
    else
    {
        return SERVER_FAIL;
    }
}

static eServer_Status eGetResource(sHTTP_Server_t* const psHTTP_Server, char const* const szResourceId, sHTTP_Resource_t** psRequestedResource)
{
    for(int i=0; i<psHTTP_Server->u32ResourceCount; ++i)
    {
        if(strcmp(szResourceId, psHTTP_Server->psResources[i].szResourceId) == 0)
        {
            *psRequestedResource = &psHTTP_Server->psResources[i];
            return SERVER_OK;
        }
    }

    *psRequestedResource = &psHTTP_Server->psResources[0]; // if no resource gets found, return default resource
    return SERVER_FAIL;
}

static eServer_Status eSendResource(sWebClient_t* const psWebClient, sHTTP_Resource_t const* psRequestedResource)
{
    // Prepare Resource
    FILE* fp = fopen(psRequestedResource->szResourcePath, "rb");
    if(fp == NULL)
    {
        printf("FILE OPEN FAILED!\n (%s)\n", psRequestedResource->szResourcePath);
        perror("--> ");
        return SERVER_FAIL;
    }
    
    fseek(fp, 0, SEEK_END);
    uint32_t u32FileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Prepare Header
    char pResponseHeader[100];
    uint32_t u32HeaderSize = snprintf(pResponseHeader, sizeof(pResponseHeader),
    "HTTP/1.1 200 OK\r\n" 
    "Content-Type: %s\r\n"
    "Content-Length: %d\r\n\r\n", psRequestedResource->szResourceType, (u32FileSize)
    );

    // Prepare response packet (header + resource)
    uint32_t const u32PacketSize = (u32FileSize + u32HeaderSize);
    char* pResponsePacket = (char*) malloc(sizeof(char) * u32PacketSize);

    // .. put the header into the result-packet
    memcpy(pResponsePacket, pResponseHeader, u32HeaderSize);

    // .. put the resource into the result-packet
    for(int i=0; i<u32FileSize; ++i)
    {
        pResponsePacket[u32HeaderSize + i] = fgetc(fp);
    }
    fclose(fp);

    // Send the response
    send(psWebClient->client_socket, pResponsePacket, u32PacketSize, 0);

    Sleep(10);
    free(pResponsePacket);
    closesocket(psWebClient->client_socket);

    printf("responded!\n");
    return SERVER_OK;
}


void vWebServer_ReceivedData_Callback(sWebServer_t* const psWebServer, sWebClient_t* const psWebClient, uint8_t* pu8Data, uint32_t u32DataLength)
{
    printf("got connection!\n");
    sHTTP_Server_t* psHTTP_Server = (sHTTP_Server_t*)psWebServer->args;
    
    // because @vWebServer_ReceivedData_Callback is no interrupt,
    //  we can do longer operations inside this function 
    if(eProcessRequest(psHTTP_Server, psWebClient, pu8Data, u32DataLength) != SERVER_OK)
    {
        closesocket(psWebClient->client_socket);
    }
}

