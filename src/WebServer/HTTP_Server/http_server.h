

#ifndef WEBSERVER_HTTPSERVER_H
#define WEBSERVER_HTTPSERVER_H

#include "../server.h"


typedef struct
{
    char* szResourceId;
    char* szResourcePath;
    char* szResourceType;

} sHTTP_Resource_t;



typedef struct
{
    char* server_ip;
    uint32_t server_port;

    uint32_t u32ResourceCount;
    sHTTP_Resource_t* const psResources;

} sHTTP_Server_Config_t;

typedef struct
{
    sWebServer_t base;

    uint32_t u32ResourceCount;
    sHTTP_Resource_t* psResources;

} sHTTP_Server_t;



eServer_Status eHTTP_Server_Init(sHTTP_Server_t* const psHTTP_Server, sHTTP_Server_Config_t* const psConfig);


eServer_Status eHTTP_Server_Start(sHTTP_Server_t* const psHTTP_Server);

eServer_Status eHTTP_Server_Process(sHTTP_Server_t* const psHTTP_Server, uint32_t u32ReceiveDataTimeout);


#endif
