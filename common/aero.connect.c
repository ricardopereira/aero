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
    ssize_t len;
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
    len = read(client,resposta.text,sizeof(resposta.text)-1);
    if (len >= 0) resposta.text[len] = 0;
    //Tratamento da resposta
    if (resposta.idAction == LOGIN_OK)
    {
        printf("Sessão iniciada: %s\n",resposta.text);
    }
    else
    {
        unlink(pipeName);
        free(req);
        req = NULL;
        printf("Login inválido: %s\n",resposta.text);
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
    int client;
    ssize_t len;
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
    len = read(client,resp->text,sizeof(resp->text)-1);
    if (len >= 0) resp->text[len] = 0;
    //Fechar pipe Cliente
    close(client);
}