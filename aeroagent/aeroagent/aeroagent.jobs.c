#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "aero.common.h"

int doJob(char *command, char *argv[], int *argc, pRequest req)
{
    Action resposta;
    int i;
    char pipeName[MAXPIPE];
    sprintf(pipeName,"%d",req->pid);
    
    if (strcmp(argv[0],"lista") == 0)
    {
        sendRequest(pipeName,req,command,argv,argc,&resposta);
        //Resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("%s",resposta.message);
            //Verificar resposta extendida
            if (resposta.hasText)
            {
                for (i = 0; i < resposta.totalLines; i++)
                    printf("%s",resposta.textLines[i]);
                printf("\n");
            }
        }
    }
    return 0;
}