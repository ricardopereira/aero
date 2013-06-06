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
        //Resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("%s\n",MSG_COMMANDSUCCESS);
        }
    }
    else if (strcmp("info",argv[0]) == 0)
    {
        sendRequest(ADMIN,req,command,argv,argc,&resposta);
        //Resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("%s\n",resposta.text);
        }
    }
    else if (strcmp("addcity",argv[0]) == 0)
    {
        sendRequest(ADMIN,req,command,argv,argc,&resposta);
        //Resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("%s\n",MSG_COMMANDSUCCESS);
        }
    }
    else if (strcmp("mudadata",argv[0]) == 0)
    {
        sendRequest(ADMIN,req,command,argv,argc,&resposta);
        //Resposta
        switch (resposta.idAction) {
            case SUCCESS_REQ:
                printf("%s\n",MSG_COMMANDSUCCESS);
                break;
            case FAILED_REQ:
                printf("%s:\n%s\n",MSG_COMMANDFAILED,resposta.text);
                break;
            default:
                printf("idAction %d não implementado\n",resposta.idAction);
                break;
        }
    }
    return 0;
}