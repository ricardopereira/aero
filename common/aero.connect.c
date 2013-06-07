#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "aero.common.h"

int checkServer()
{
    //Verificar servidor
    if (access(SERVER,W_OK) == -1)
    {
        printf("Servidor não está em execução\n");
        return 0;
    }
    return 1;
}

void initRequest(pRequest p)
{
    p->pid = 0;
    p->isAdmin = 0;
    strcpy(p->command,"");
}

pRequest newRequest()
{
    pRequest new = malloc(sizeof(Request));
    initRequest(new);
    return new;
}

pRequest createClientPipe(int pid, int isAdmin)
{
    char pipeName[MAXPIPE];
    pRequest req = newRequest();
    req->pid = getpid();
    if (!isAdmin)
    {
        sprintf(pipeName,"%d",req->pid);
        mkfifo(pipeName,0644);
    }
    else
    {
        mkfifo(ADMIN,0644);
        req->isAdmin = 1;
    }
    return req;
}

void destroyClientPipe(pRequest req)
{
    char pipeName[MAXPIPE];
    if (!req) return;
    sprintf(pipeName,"%d",req->pid);
    unlink(pipeName);
    free(req);
}

void destroyAdminPipe(pRequest req)
{
    unlink(ADMIN);
    if (!req) return;
    free(req);
}

void sendRequestToServer(pRequest p)
{
    int server = open(SERVER,O_WRONLY);
    write(server,p,sizeof(Request));
    close(server);
}

pRequest doLogin(char *username, char *password)
{
    pRequest req;
    int client;
    Action resposta;
    char pipeName[MAXPIPE];
    //Verificar ligação ao servidor
    if (checkServer() == 0)
      return NULL;
    
    //Criar pipe actual
    req = createClientPipe(getpid(),strcmp(username,ADMIN) == 0);
    
    //Dados para o pedido
    strcpy(req->command,"login");
    strcpy(req->username,username);
    strcpy(req->password,password);

    //Enviar pedido ao servidor
    sendRequestToServer(req);
    
    //Obter resposta do servidor
    resposta.idAction = 0;
    if (strcmp(username,ADMIN) == 0)
        sprintf(pipeName,"%s",username);
    else
        sprintf(pipeName,"%d",req->pid);
    
    client = open(pipeName,O_RDONLY);
    //Ler resposta
    read(client,&resposta.idAction,sizeof(int));
    read(client,&resposta.hasText,sizeof(int));
    read(client,resposta.message,sizeof(resposta.message));
    //Tratamento da resposta
    if (resposta.idAction == LOGIN_OK)
    {
        printf("Sessão iniciada: %s\n",resposta.message);
    }
    else
    {
        unlink(pipeName);
        free(req);
        req = NULL;
        printf("Login inválido: %s\n",resposta.message);
    }
    close(client);
    return req;
}

int doLogout(pRequest req)
{
    if (!req) return 0;
    //Dados para o pedido
    strcpy(req->command,"logout");
    //Enviar pedido ao servidor
    sendRequestToServer(req);
    return 1;
}

void sendRequest(char *pipeClient, pRequest req, char *command, char *argv[], int *argc, pAction resp)
{
    int client, i;
    char text[255];
    int nrLinhas = 0;
    char **linhas;
    
    //Dados para o pedido
    strcpy(req->command,command);
    //Enviar pedido ao servidor
    sendRequestToServer(req);
    //Obter resposta do servidor
    resp->idAction = 0;
    //Abrir pipe Cliente
    client = open(pipeClient,O_RDONLY);
    //Ler resposta
    read(client,&resp->idAction,sizeof(int));
    read(client,&resp->hasText,sizeof(int));
    read(client,resp->message,sizeof(resp->message));
    
    //Tem conteúdo extenso para devolver
    if (resp->hasText)
    {
        printf("Has extended text %d\n",resp->hasText);
        //Número de Linhas
        read(client,&nrLinhas,sizeof(int));
        
        linhas = malloc(nrLinhas * sizeof(char*));
        for (i = 0; i < nrLinhas; i++)
        {
            linhas[i] = malloc(sizeof(text));
            read(client,linhas[i],sizeof(text));
        }
    }
    //Fechar pipe Cliente
    close(client);
    
    if (resp->hasText)
        for (i = 0; i < nrLinhas; i++)
            printf("%s",linhas[i]);
}

void sendRequestWithStatus(char *pipeClient, pRequest req, char *command, char *argv[], int *argc, pAction resp)
{
    sendRequest(pipeClient,req,command,argv,argc,resp);
    //Resposta
    if (resp->idAction == SUCCESS_REQ)
        printf("%s\n",MSG_COMMANDSUCCESS);
}

void sendRequestWithMessage(char *pipeClient, pRequest req, char *command, char *argv[], int *argc, pAction resp)
{
    sendRequest(pipeClient,req,command,argv,argc,resp);
    //Resposta
    if (resp->idAction == SUCCESS_REQ)
        printf("%s\n",resp->message);
}

void sendRequestWithFail(char *pipeClient, pRequest req, char *command, char *argv[], int *argc, pAction resp)
{
    sendRequest(pipeClient,req,command,argv,argc,resp);
    //Resposta
    switch (resp->idAction) {
        case SUCCESS_REQ:
            printf("%s\n",MSG_COMMANDSUCCESS);
            break;
        case FAILED_REQ:
            printf("%s:\n %s\n",MSG_COMMANDFAILED,resp->message);
            break;
        default:
            printf("idAction %d não implementado\n",resp->idAction);
            break;
    }
}