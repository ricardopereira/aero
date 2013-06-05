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
        sendRequest(ADMIN,req,command,argv,argc,resposta);
        //Resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("Comando executado com sucesso: %s\n",resposta.text);
        }
    }
    else if (strcmp("info",argv[0]) == 0)
    {
        sendRequest(ADMIN,req,command,argv,argc,resposta);
        //Resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("Comando executado com sucesso:\n%s\n",resposta.text);
        }
    }
    else if (strcmp("addcity",argv[0]) == 0)
    {
        sendRequest(ADMIN,req,command,argv,argc,resposta);
        //Resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("Comando executado com sucesso:\n%s\n",resposta.text);
        }
    }
    return 0;
}