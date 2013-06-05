#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "aero.common.h"

int doJob(char *command, char *argv[], int *argc, pRequest req)
{
    int client;
    Action resposta;
    char pipeName[MAXPIPE];
    ssize_t len;
    
    if (strcmp(argv[0],"voos") == 0)
    {
        //Dados para o pedido
        strcpy(req->command,command);
        
        //Enviar pedido ao servidor
        sendRequestToServer(req);
        
        //Obter resposta do servidor
        resposta.idAction = 0;
        client = open(pipeName,O_RDONLY);
        //Ler resposta
        read(client,&resposta.idAction,sizeof(int));
        len = read(client,resposta.text,sizeof(resposta.text)-1);
        if (len >= 0) resposta.text[len] = 0;
        //Tratamento da resposta
        if (resposta.idAction == SUCCESS_REQ)
        {
            printf("Comando com sucesso: %s\n",resposta.text);
        }
        else
        {
            printf("Comando inválido: %s\n",resposta.text);
        }
        close(client);
    }
    return 0;
}