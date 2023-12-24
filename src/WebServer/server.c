

#include "server.h"
#include <assert.h>

#define WEBSERVER_UNUSED(x)     (void)x
#define __WEBSERVER_WEAK        __attribute__((weak))

#define WEBSERVER_MAX_CONNECTIONS   10U



eServer_Status eWebServer_Init(sWebServer_t* const psWebServer, char const* const server_ip, uint32_t server_port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
    {
        return SERVER_FAIL;
    }

    psWebServer->server_addr.sin_family = AF_INET;
    psWebServer->server_addr.sin_addr.s_addr = inet_addr(server_ip);
    psWebServer->server_addr.sin_port = htons(server_port);

    psWebServer->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(psWebServer->server_socket == INVALID_SOCKET)
    {
        return SERVER_FAIL;
    }

    return SERVER_OK;
}


eServer_Status eWebServer_Start(sWebServer_t* const psWebServer)
{
    if(bind(psWebServer->server_socket, (struct sockaddr *)&psWebServer->server_addr, sizeof(psWebServer->server_addr)) == SOCKET_ERROR)
    {
        return SERVER_FAIL;
    }

    if(listen(psWebServer->server_socket, WEBSERVER_MAX_CONNECTIONS) == SOCKET_ERROR)
    {
        return SERVER_FAIL;
    }

    return SERVER_OK;
}


eServer_Status eWebServer_Process(sWebServer_t* const psWebServer, uint32_t u32ReceiveDataTimeout)
{
    sWebClient_t sWebClient;
    uint32_t client_addr_size = sizeof(sWebClient.client_addr);
    sWebClient.client_socket = accept(psWebServer->server_socket, (struct sockaddr*)&sWebClient.client_addr, (int*)&client_addr_size);
    if(sWebClient.client_socket == 0)
    {
        return SERVER_FAIL;
    }

    setsockopt(sWebClient.client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&u32ReceiveDataTimeout, sizeof(u32ReceiveDataTimeout));
    int recv_len = recvfrom(sWebClient.client_socket, (char*)psWebServer->au8InternalDataBuffer, WEBSERVER_MAX_DATA_LENGTH, 0, (struct sockaddr*)&sWebClient.client_addr, (int*)&client_addr_size);
    if(recv_len <= 0)
    {
        closesocket(sWebClient.client_socket);
        return SERVER_TIMEOUT;
    }

    vWebServer_ReceivedData_Callback(psWebServer, &sWebClient, psWebServer->au8InternalDataBuffer, recv_len);
    return SERVER_OK;
}


eServer_Status eWebClient_SendData(sWebClient_t* const psWebClient, uint8_t const* const pu8Data, uint32_t u32DataLength)
{
    if(send(psWebClient->client_socket, (char const*)pu8Data, u32DataLength, 0) <= 0)
    {
        return SERVER_FAIL;
    }
    
    return SERVER_OK;
}


void vWebClient_CloseConnection(sWebClient_t* const psWebClient)
{
    closesocket(psWebClient->client_socket);
}


/* *** Callback functions *** */

__WEBSERVER_WEAK void vWebServer_ReceivedData_Callback(sWebServer_t* const psWebServer, sWebClient_t* const psWebClient, uint8_t* pu8Data, uint32_t u32DataLength)
{
    WEBSERVER_UNUSED(psWebServer);
    WEBSERVER_UNUSED(psWebClient);
    WEBSERVER_UNUSED(pu8Data);
    WEBSERVER_UNUSED(u32DataLength);
}