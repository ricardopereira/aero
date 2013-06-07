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
        sendRequestWithMessage(pipeName,req,command,argv,argc,&resposta);
    }
    return 0;
}