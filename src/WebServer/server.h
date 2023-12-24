


#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include "../server_status.h"
#include <winsock2.h>

#define WEBSERVER_MAX_DATA_LENGTH   500U


typedef struct
{
    struct sockaddr_in server_addr;
    SOCKET server_socket;

    void* args;

    uint8_t au8InternalDataBuffer[WEBSERVER_MAX_DATA_LENGTH];

} sWebServer_t;

typedef struct
{
    struct sockaddr_in client_addr;
    SOCKET client_socket;

} sWebClient_t;


/* *** structure functions *** */

/**
 * @brief   : Initializes a given sWebServer_t with the given configuration.
 *             This initialized sWebServer_t can than be used in further functions.
 * 
 * @param psWebServer : The sWebServer_t that should get initialized.
 * @param server_ip   : The IPv4-Address of the WebServer.
 * @param server_port : The Port of the WebServer.
 * 
 * @return eServer_Status: SERVER_OK   : sWebServer_t successfully initialized.
 *                         SERVER_FAIL : An error occurred.
 */
eServer_Status eWebServer_Init(sWebServer_t* const psWebServer, char const* const server_ip, uint32_t server_port);

/**
 * @brief   : Starts a Initialized sWebServer_t.
 * 
 * @param psWebServer: The initialized sWebServer_t that should get started.
 * 
 * @return eServer_Status: SERVER_OK   : sWebServer_t started successfully.
 *                         SERVER_FAIL : An error occurred.
 */
eServer_Status eWebServer_Start(sWebServer_t* const psWebServer);

/**
 * @brief   : Processes the sWebServer_t.
 *             This function should be called inside a endless while loop inside the main function.
 *             If data gets received from a client, the callback function @vWebServer_ReceivedData_Callback will be called.
 * 
 * @param psWebServer           : The running sWebServer_t.
 * @param u32ReceiveDataTimeout : The maximum amount of ms the server waits for data of a connected client.
 *
 * @return eServer_Status: SERVER_OK   : Data got successfully received.
 *                         SERVER_FAIL : No data got received or a timeout happend.
 */
eServer_Status eWebServer_Process(sWebServer_t* const psWebServer, uint32_t u32ReceiveDataTimeout);



/**
 * @brief   : Sends data to a given sWebClient_t.
 *             The user needs to close the connection to the client himself via the @vWebClient_CloseConnection function.
 * 
 * @param psWebClient   : The sWebClient_t that should receive the given data.
 * @param pu8Data       : The data that should get transmitted to sWebClient_t.
 * @param u32DataLength : The length of the data.
 * 
 * @return eServer_Status: SERVER_OK   : Successfully send data to the client.
 *                         SERVER_FAIL : An error occurred.
 */
eServer_Status eWebClient_SendData(sWebClient_t* const psWebClient, uint8_t const* const pu8Data, uint32_t u32DataLength);

/**
 * @brief   : Closes a connection to a client.
 * 
 * @param psWebClient: The sWebClient connection that should get closed.
 */
void vWebClient_CloseConnection(sWebClient_t* const psWebClient);


/* *** Callback functions *** */

/**
 * @brief   : This function get called from @eWebServer_Process, if data from a connecting client got received. 
 * 
 * @param psWebServer   : The sWebServer_t that received data.
 * @param psWebClient    : The sWebClient that had send the data to the server. 
 * @param pu8Data       : The received data. 
 * @param u32DataLength : The length of data that got received. 
 */
void vWebServer_ReceivedData_Callback(sWebServer_t* const psWebServer, sWebClient_t* const psWebClient, uint8_t* pu8Data, uint32_t u32DataLength);


#endif
