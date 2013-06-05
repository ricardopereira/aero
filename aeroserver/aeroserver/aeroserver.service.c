#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "aero.common.h"
#include "aeroserver.common.h"
#include "aeroserver.db.h"
#include "aeroserver.cidades.h"
#include "aeroserver.voos.h"
#include "aeroserver.users.h"
#include "aeroserver.clients.h"

pDatabase db;
int server;

int doJob(pDatabase db, pRequest req, char *pipe);
void shutdownClients(pDatabase db);

void initRequest(pRequest r)
{
    r->pid = 0;
    r->isAdmin = 0;
    strcpy(r->command,"");
}

void initAction(pAction a)
{
    a->idAction = 0;
    strcpy(a->text,"");
}

void stopServer(int sinal)
{
    fflush(stdout);
    //Terminar Clientes ligados ao Servidor
    shutdownClients(db);
    //pipe do Servidor
    close(server);
    //Remover named pipe do Servidor
    unlink(SERVER);
    //Libertar memória
    freeDB(db);
    //Fechar a aplicação
    exit(0);
}

int startServer(int modeBG)
{
    Request req;
    char pipeClient[15];
    int shutDown = 0, res;
    char *dbName;
    int f;
    
    //Verificar se o servidor já está a correr
    if (access(SERVER,W_OK) != -1)
    {
        printf("O servidor já está em execução\n");
        return 1;
    }
    //Verificar o caminho da base de dados
    dbName = getenv("SOFICHEIRO");
    if (!dbName)
    {
        printf("Configuração em falta: SOFICHEIRO\n");
    }
    //Verificar data atual
    if (!getenv("SODATA"))
    {
        printf("Configuração em falta: SODATA\n");
    }
    
    //Carregar dados do sistema
    db = loadDB("db.bin");
    db->inBackground = modeBG;
    if (!db) return 1;
    
    //Obter password do Administrador
    if (loadAdmin("SOADMPASS",db) != 0)
    {
        printf("Não foi possível obter Administrador: %s\n","SOADMPASS");
        return 1;
    }
    //Obter lista de utilizadores
    if (loadUsers("SOAGENTES",db) != 0)
    {
        printf("Não foi possível obter Utilizadores: %s\n","SOAGENTES");
        return 1;
    }
    else
    {
        loadUsers("SOAGENTES1",db);
    }
    
    //Sinal para parar o servidor
    signal(SIGUSR1,stopServer);
    //Criar pipe do Servidor
    mkfifo(SERVER,0644);
    
    server = open(SERVER,O_RDWR);
    do
    {
        initRequest(&req);
        //Ler pedidos
        read(server,&req,sizeof(Request));
        //Sem nada a tratar
        if (req.pid == 0)
            continue;
        //Recebido e preparar resposta
        if (req.isAdmin == 1)
            sprintf(pipeClient,"%s",ADMIN);
        else
            sprintf(pipeClient,"%d",req.pid);
        //Recebido e verificar comando
        if (strcmp(req.command,"") == 0)
        {
            if (!db->inBackground)
                printf("Sem comando: %s\n",pipeClient);
            continue;
        }
        else
            if (!db->inBackground)
                printf("(%s)comando recebido: %s\n",pipeClient,req.command);
        //Responder ao comando
        res = doJob(db,&req,pipeClient);
        if (res == SHUTDOWN) shutDown = 1;

    } while (shutDown == 0);
    stopServer(0);
    return 0;
}

int doJob(pDatabase db, pRequest req, char *pipe)
{
    int client, res = 0;
    Action action;
    void *ptr;
    
    //Verificar acesso ao pipe do cliente
    if (access(pipe,W_OK) == -1)
    {
        printf("Sem acesso: %s\n",pipe);
        return NOACCESS;
    }
    
    //Inicializar dados de resposta
    initAction(&action);
    //Verificar credenciais
    action.idAction = checkLogin(db,req->username,req->password);
    
    //Abrir pipe do ciente
    client = open(pipe,O_WRONLY);
    //Mensagens em Foreground do estado
    if (!db->inBackground)
    {
        printf("(%s)%s fez pedido: %s\n",pipe,req->username,req->command);
        if (action.idAction == LOGIN_FAILED)
            printf("(%s)falhou login: %s\nabort\n",pipe,req->username);
    }
    
    //COMANDOS
    /**/ if (strcmp("login",req->command) == 0)
    {
        if (action.idAction == LOGIN_OK)
        {
            if (!db->inBackground)
                printf("(%s)cliente adicionado: %s\n",pipe,req->username);
            addClientWithUser(db,req->pid,req->username);
        }
        sprintf(action.text,"%s",req->username);
    }
    else if (strcmp("logout",req->command) == 0 && action.idAction == LOGIN_OK)
    {
        removeClient(db,req->pid);
    }
    else if (strcmp("shutdown",req->command) == 0 && action.idAction == LOGIN_OK)
    {
        action.idAction = SUCCESS_REQ;
        sprintf(action.text,"%s","servidor a encerrar");
        res = SHUTDOWN;
    }
    else if (strcmp("info",req->command) == 0 && action.idAction == LOGIN_OK)
    {
        action.idAction = SUCCESS_REQ;
        //Resposta
        if (db->clients)
        {
            strcpy(action.text,"Clientes activos:\n");
            ptr = db->clients;
            while (ptr)
            {
                snprintf(action.text,sizeof(action.text),"%s %d - %s\n",action.text,
                         ((pClient)ptr)->pid,getClientUsername((pClient)ptr));
                
                ptr = ((pClient)ptr)->next;
            }
        }
    }
    else if (strcmp("addcity", req->command) == 0 && action.idAction == LOGIN_OK)
    {
        pCidade CidadeAux;
        action.idAction = SUCCESS_REQ;
        
        if (db->totalCidades<MaxCitys)
        {
            CidadeAux = addCidade(db, action.text, 0);
        }
    }
    else if (strcmp("addVoo", req->command) == 0 && action.idAction == LOGIN_OK)
    {
        pVoo VooAux;
        action.idAction = SUCCESS_REQ;
        
//        VooAux = addVoo(<#pDatabase db#>, <#pCidade origem#>, <#pCidade destino#>, <#int dia#>) addCidade(db, action.text, 0);
    }
    else
    {
        action.idAction = NOEXIST_REQ;
        sprintf(action.text,"%s","não implementado");
    }
    //Enviar resposta
    write(client,&action.idAction,sizeof(int));
    write(client,action.text,sizeof(action.text));
    close(client);
    return res;
}

void shutdownClients(pDatabase db)
{
    pClient auxClient;
    auxClient = db->clients;
    while (auxClient)
    {
        kill(auxClient->pid,SIGUSR1);
        auxClient = auxClient->next;
    }
}