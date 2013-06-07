#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "aero.common.h"
#include "aero.shell.h"
#include "aeroserver.common.h"
#include "aeroserver.db.h"
#include "aeroserver.cidades.h"
#include "aeroserver.voos.h"
#include "aeroserver.users.h"
#include "aeroserver.clients.h"

pDatabase db;
int server;
int dbDefault;
char *dbName;

void shutdownClients(pDatabase db);

//COMANDOS
int doJob(pDatabase db, pRequest req, char *pipe);
void doAddVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
void doCancelVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
void doMudaData(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
void doGetData(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
void doLista(pAction action, char *pipe, pDatabase db, char *argv[], int argc);

void initRequest(pRequest r)
{
    r->pid = 0;
    r->isAdmin = 0;
    strcpy(r->command,"");
}

void initAction(pAction a)
{
    a->idAction = 0;
    strcpy(a->message,"");
    a->hasText = 0;
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
    //Gravar DB
    //saveDB(dbName,db);
    
    
    
    
    
    
    
    //Libertar memória
    if (dbDefault) free(dbName);
    freeDB(db);
    //Fechar a aplicação
    exit(0);
}

int startServer(int modeBG)
{
    Request req;
    char pipeClient[15];
    int shutDown = 0, res;
    
    //Verificar se o servidor já está a correr
    if (access(SERVER,W_OK) != -1)
    {
        printf("O servidor já está em execução\n");
        return 1;
    }
    //Verificar o caminho da base de dados
    dbName = getenv(ENVFICHEIRO);
    if (!dbName)
    {
        printf("Configuração em falta: %s\n",ENVFICHEIRO);
        //Por defeito
        dbName = malloc(sizeof(char)*6);
        strcpy(dbName,"db.bin");
        dbDefault = 1;
    }
    else
        dbDefault = 0;
    
    ////////////////////////////
    //Carregar dados do sistema
    ////////////////////////////
    db = loadDB(dbName);
    db->inBackground = modeBG;
    if (!db) return 1;
    
    //Obter Data Actual do sistema
    if (loadData(SODATA,db) != 0)
    {
        printf("Não foi possível obter a data do sistema: %s\n",SODATA);
        return 1;
    }
    else if (db->data == 0)
    {
        printf("Data do sistema inválida\n");
        return 1;
    }
    else
        printf("Data do Sistema: %d\n",db->data);
    
    //Obter password do Administrador
    if (loadAdmin(SOADMPASS,db) != 0)
    {
        printf("Não foi possível obter Administrador: %s\n",SOADMPASS);
        return 1;
    }
    //Obter lista de utilizadores
    if (loadUsers(SOAGENTES,db) != 0)
    {
        printf("Não foi possível obter Utilizadores: %s\n",SOAGENTES);
        return 1;
    }
    
    //Modo Foreground: informação
    if (!db->inBackground)
    {
        printf("\nUsers (%d):\n",db->totalUsers);
        showUtilizadores(db->users,1);
        printf("\nCidades (%d):\n",db->totalCidades);
        showCidades(db->cidades);
        printf("\nVoos (%d):\n",db->totalVoos);
        showVoosDisponiveis(db->voos,0);
        
        //Verificar histórico de voos
        printf("\nVoos ultrapassados:\n");
        checkVoos(db);
        //Voos após a passagem para o histórico
        printf("\nVoos disponíveis (%d):\n",db->totalVoos);
        showVoosDisponiveis(db->voos,0);
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
        
        ///////////////////////
        //Responder ao comando
        ///////////////////////
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
    char *commandArgv[MAXCOMMANDARGS];
    int commandArgc = 0;
    
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
    
    //Interpretar os argumentos
    getCommandArgs(req->command,commandArgv,&commandArgc);
    
    ///////////
    //COMANDOS
    ///////////
    /**/ if (commandArgc <= 0)
    {
        action.idAction = NOEXIST_REQ;
        sprintf(action.message,"%s","comando vazio");
    }
    else if (strcmp("login",commandArgv[0]) == 0)
    {
        if (action.idAction == LOGIN_OK)
        {
            if (!db->inBackground)
                printf("(%s)cliente adicionado: %s\n",pipe,req->username);
            addClientWithUser(db,req->pid,req->username);
        }
        sprintf(action.message,"%s",req->username);
    }
    else if (strcmp("logout",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        removeClient(db,req->pid);
    }
    else if (strcmp("shutdown",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        action.idAction = SUCCESS_REQ;
        sprintf(action.message,"%s","servidor a encerrar");
        res = SHUTDOWN;
    }
    else if (strcmp("info",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        action.idAction = SUCCESS_REQ;
        //Resposta
        if (db->clients)
        {
            strcpy(action.message,"\nINFORMAÇÃO DO SISTEMA:\n");
            snprintf(action.message,sizeof(action.message),"%sData actual: %d\n",action.message,db->data);
            snprintf(action.message,sizeof(action.message),"%sTotal de utilizadores: %d\n",action.message,db->totalUsers);
            snprintf(action.message,sizeof(action.message),"%sTotal de cidades: %d\n",action.message,db->totalCidades);
            snprintf(action.message,sizeof(action.message),"%sTotal de voos disponíveis: %d\n",action.message,db->totalVoos);

            snprintf(action.message,sizeof(action.message),"%sClientes activos (%d):\n",action.message,db->totalClients);
            ptr = db->clients;
            while (ptr)
            {
                snprintf(action.message,sizeof(action.message),"%s %d - %s\n",action.message,
                         ((pClient)ptr)->pid,getClientUsername((pClient)ptr));
                
                ptr = ((pClient)ptr)->next;
            }
        }
    }
    else if (strcmp("addcity",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        action.idAction = SUCCESS_REQ;
        
        //ToDo - Rever
        if (db->totalCidades < MAXCITIES)
        {
            addCidade(db,commandArgv[1],0);
        }
    }
    else if (strcmp("addvoo",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 5)
    {
        doAddVoo(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("cancel",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        doCancelVoo(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("mudadata",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        doMudaData(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("getdata",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        doGetData(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("lista",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        doLista(&action,pipe,db,commandArgv,commandArgc);
    }
    else
    {
        action.idAction = NOEXIST_REQ;
        sprintf(action.message,"%s","não implementado");
        
        if (!db->inBackground)
            printf("(%s)não implementado\n",pipe);
    }
    
    //////////////////
    //Enviar resposta
    //////////////////
    write(client,&action.idAction,sizeof(int));
    write(client,action.message,sizeof(action.message));
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

////////////
// Comandos
////////////

void doAddVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    pCidade auxCidadeOrigem, auxCidadeDestino;
    int auxID, auxDia;
    
    action->idAction = FAILED_REQ;
    //ID
    auxID = atoi(argv[1]);
    if (!auxID)
    {
        sprintf(action->message,"ID do voo não pode ser nulo");
        return;
    }
    //Verificar se já existe
    if (findVoo(db->voos,auxID))
    {
        //ID já existe
        sprintf(action->message,"ID %d do voo já existe",auxID);
    }
    else
    {
        //Cidades
        auxCidadeOrigem = findCidade(db->cidades,argv[2]);
        if (!auxCidadeOrigem)
        {
            sprintf(action->message,"Cidade origem \"%s\" não existe",argv[2]);
            return;
        }
        auxCidadeDestino = findCidade(db->cidades,argv[3]);
        if (!auxCidadeDestino)
        {
            sprintf(action->message,"Cidade destino \"%s\" não existe",argv[3]);
            return;
        }
        //Dia (Data)
        auxDia = atoi(argv[4]);
        if (!auxDia || auxDia < db->data)
        {
            sprintf(action->message,"É necessário especificar um dia superior a %d",db->data);
            return;
        }
        
        //Adiciona o voo
        addVooByID(db,auxID,auxCidadeOrigem,auxCidadeDestino,auxDia);
        //Com sucesso
        action->idAction = SUCCESS_REQ;
    }
}

void doCancelVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    int auxID;
    
    action->idAction = FAILED_REQ;
    //ID
    auxID = atoi(argv[1]);
    if (!auxID)
    {
        sprintf(action->message,"ID do voo não pode ser nulo");
        return;
    }
    //Verificar se já existe
    if (!findVoo(db->voos,auxID))
    {
        //ID já existe
        sprintf(action->message,"ID %d do voo não existe",auxID);
    }
    else
    {
        //ToDo - podia ser optimizado, como já tinhamos encontrado o voo
        removeVoo(db,auxID);
        action->idAction = SUCCESS_REQ;
    }
}

void doMudaData(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    int auxDia;
    //Dia (Data)
    auxDia = atoi(argv[1]);
    //Verificar superioridade
    if (auxDia && auxDia > db->data)
    {
        db->data = auxDia;
        saveData(SODATA,db);
        action->idAction = SUCCESS_REQ;
    }
    else
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Data atual do sistema: %d",db->data);
    }
}

void doGetData(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    action->idAction = SUCCESS_REQ;
    sprintf(action->message,"Data atual do sistema: %d",db->data);
}

void doLista(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    pVoo auxVoo;
    action->idAction = SUCCESS_REQ;
    //Resposta
    if (db->voos)
    {
        strcpy(action->message,"\nVoos disponíveis");
        snprintf(action->message,sizeof(action->message),"%s (%d):\n",action->message,db->totalVoos);
        auxVoo = db->voos;
        while (auxVoo)
        {
            snprintf(action->message,sizeof(action->message),"%s %d: ORIGEM %s, DESTINO %s, DIA %d, LUGARES VAGOS: %d\n",action->message,
                     auxVoo->ID,
                     auxVoo->cidadeOrigem->nome,
                     auxVoo->cidadeDestino->nome,
                     auxVoo->dia,
                     auxVoo->capacidade-auxVoo->ocupacao);
            
            auxVoo = auxVoo->next;
        }
    }
    else
        strcpy(action->message,"Sem voos disponíveis\n");
}