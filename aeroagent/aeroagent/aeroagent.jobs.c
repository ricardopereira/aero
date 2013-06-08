#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "aero.common.h"

int doJob(char *command, char *argv[], int *argc, pRequest req)
{
    Action resposta;
    char pipeName[MAXPIPE];
    sprintf(pipeName,"%d",req->pid);
    
    if (strcmp(argv[0],"lista") == 0)
    {
        sendRequestWithExtendedText(pipeName,req,command,&resposta);
    }
    else if (strcmp(argv[0],"mudapass") == 0)
    {
        sendRequestWithFail(pipeName,req,command,&resposta);
        return PASSCHANGED;
    }
    else if (strcmp(argv[0],"pesquisa") == 0)
    {
        sendRequestWithExtendedText(pipeName,req,command,&resposta);
    }
    else if (strcmp(argv[0],"marca") == 0)
    {
        sendRequestWithFail(pipeName,req,command,&resposta);
    }
    else if (strcmp(argv[0],"desmarca") == 0)
    {
        sendRequestWithFail(pipeName,req,command,&resposta);
    }
    return 0;
}