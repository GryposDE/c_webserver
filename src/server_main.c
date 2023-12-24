

#include "WebServer/HTTP_Server/http_server.h"
#include <stdio.h>


// ##################### RESOURCES #######################

sHTTP_Resource_t psServerResources[] = {
    { .szResourceId = "index.html", .szResourcePath = "../webpage/html/index.html", .szResourceType = "text/html" },
    { .szResourceId = "index.css", .szResourcePath = "../webpage/css/index.css", .szResourceType = "text/css" },
    { .szResourceId = "index.js", .szResourcePath = "../webpage/js/index.js", .szResourceType = "text/js" },
    { .szResourceId = "index.svg", .szResourcePath = "../webpage/icon/index.svg", .szResourceType = "image/svg+xml" },
    { .szResourceId = "chris.png", .szResourcePath = "../webpage/images/chris.png", .szResourceType = "image/png" },

    { .szResourceId = "projects.html", .szResourcePath = "../webpage/html/projects.html", .szResourceType = "text/html" },
};

// ########################################################


int main(int argc, char **argv)
{
    sHTTP_Server_Config_t sHTTP_Server_Config = {
        .server_ip = "127.0.0.1",
        .server_port = 2018,
        .psResources = psServerResources,
        .u32ResourceCount = (sizeof(psServerResources)/sizeof(sHTTP_Resource_t)),
    };

    sHTTP_Server_t sHTTP_Server;
    if(eHTTP_Server_Init(&sHTTP_Server, &sHTTP_Server_Config) != SERVER_OK)
    {
        printf("Initializing Server failed..\n");
        return -1;
    }

    if(eHTTP_Server_Start(&sHTTP_Server) != SERVER_OK)
    {
        printf("Starting Server failed..\n");
        return -1;
    }

    printf("Started the server!\n IP: %s:%d\n", sHTTP_Server_Config.server_ip, sHTTP_Server_Config.server_port);

    static int counter = 0;
    while (1)
    {
        if(eHTTP_Server_Process(&sHTTP_Server, 1000) == SERVER_OK)
        {
            ++counter;
            printf("\n Received %d server requests in total!\n", counter);
        }
    }

}

