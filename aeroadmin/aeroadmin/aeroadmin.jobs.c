#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "aero.common.h"

int doJob(char *command, char *argv[], int *argc, pRequest req)
{
    Action resposta;
    
    /**/ if (strcmp("shutdown",argv[0]) == 0)
    {
        sendRequest(ADMIN,req,command,argv,argc,&resposta);
    }
    else if (strcmp("info",argv[0]) == 0)
    {
        sendRequestWithMessage(ADMIN,req,command,argv,argc,&resposta);
    }
    else if (strcmp("addcity",argv[0]) == 0)
    {
        sendRequestWithStatus(ADMIN,req,command,argv,argc,&resposta);
    }
    else if (strcmp("mudadata",argv[0]) == 0)
    {
        sendRequestWithFail(ADMIN,req,command,argv,argc,&resposta);
    }
    else if (strcmp("getdata",argv[0]) == 0)
    {
        sendRequestWithMessage(ADMIN,req,command,argv,argc,&resposta);
    }
    else if (strcmp("addvoo",argv[0]) == 0)
    {
        sendRequestWithFail(ADMIN,req,command,argv,argc,&resposta);
    }
    else if (strcmp("cancel",argv[0]) == 0)
    {
        sendRequestWithFail(ADMIN,req,command,argv,argc,&resposta);
    }
    else if (strcmp("lista",argv[0]) == 0)
    {
        sendRequestWithMessage(ADMIN,req,command,argv,argc,&resposta);
    }
    return 0;
}