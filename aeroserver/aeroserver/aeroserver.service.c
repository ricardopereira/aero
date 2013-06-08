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

int doJob(pDatabase db, pRequest req, char *pipe);
//COMANDOS
int doInfo(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc);
int doAddVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doCancelVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doMudaData(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doGetData(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doLista(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc);
int doSeePast(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc);
int doAddUser(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doDelUser(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doAddCity(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doDelCity(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doMudaPass(char *username, pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doPesquisaVoos(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc);
int doMarcaViagem(pAction action, char *pipe, pDatabase db, char *argv[], int argc);
int doDesmarcaViagem(pAction action, char *pipe, pDatabase db, char *argv[], int argc);

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
        showVoosDisponiveis(db->voos,1);
        
        //Verificar histórico de voos
        checkVoos(db);
        
        //Voos após a passagem para o histórico
        printf("\nVoos disponíveis (%d):\n",db->totalVoos);
        showVoosDisponiveis(db->voos,0);
        printf("\n");
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
    int client, res = 0, commandDone;
    Action action;
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
    
    //Se o login falhou, então aborta a operação
    if (action.idAction == LOGIN_FAILED)
    {
        snprintf(action.message,MAXMESSAGE,"login failed to \"%s\"",req->username);
        action.hasText = 0;
        write(client,&action.idAction,sizeof(int));
        write(client,&action.hasText,sizeof(int));
        write(client,action.message,sizeof(action.message));
        close(client);
        return NOACCESS;
    }
    
    //Interpretar os argumentos
    getCommandArgs(req->command,commandArgv,&commandArgc);
    
    commandDone = 0;
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
            addClientWithUser(db,req->pid,req->username);
            if (!db->inBackground)
                printf("(%s)cliente adicionado: %s\n",pipe,req->username);
        }
        sprintf(action.message,"%s",req->username);
    }
    else if (strcmp("logout",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        removeClient(db,req->pid);
        action.idAction = SUCCESS_REQ;
        sprintf(action.message,"Logout com sucesso: %s",req->username);
        if (!db->inBackground)
            printf("(%s)cliente removido: %s\n",pipe,req->username);
    }
    else if (strcmp("shutdown",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        action.idAction = SUCCESS_REQ;
        sprintf(action.message,"%s","servidor a encerrar");
        res = SHUTDOWN;
    }
    else if (strcmp("info",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        commandDone = doInfo(&action,pipe,client,db,commandArgv,commandArgc);
    }
    else if (strcmp("addcity",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        commandDone = doAddCity(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("delcity",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        commandDone = doDelCity(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("addvoo",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 5)
    {
        commandDone = doAddVoo(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("cancel",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        commandDone = doCancelVoo(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("mudadata",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        commandDone = doMudaData(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("getdata",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        commandDone = doGetData(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("lista",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        commandDone = doLista(&action,pipe,client,db,commandArgv,commandArgc);
    }
    else if (strcmp("seepast",commandArgv[0]) == 0 && action.idAction == LOGIN_OK)
    {
        commandDone = doSeePast(&action,pipe,client,db,commandArgv,commandArgc);
    }
    else if (strcmp("adduser",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 3)
    {
        commandDone = doAddUser(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("deluser",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 2)
    {
        commandDone = doDelUser(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("mudapass",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 3)
    {
        commandDone = doMudaPass(req->username,&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("pesquisa",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 3)
    {
        commandDone = doPesquisaVoos(&action,pipe,client,db,commandArgv,commandArgc);
    }
    else if (strcmp("marca",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 3)
    {
        commandDone = doMarcaViagem(&action,pipe,db,commandArgv,commandArgc);
    }
    else if (strcmp("desmarca",commandArgv[0]) == 0 && action.idAction == LOGIN_OK && commandArgc == 3)
    {
        commandDone = doDesmarcaViagem(&action,pipe,db,commandArgv,commandArgc);
    }
    else
    {
        action.idAction = NOEXIST_REQ;
        snprintf(action.message,MAXMESSAGE,"não implementado");
        
        if (!db->inBackground)
            printf("\"%s\" não implementado\n",req->command);
    }
    
    if (!commandDone)
    {
        //////////////////
        //Enviar resposta
        //////////////////
        action.hasText = 0;
        write(client,&action.idAction,sizeof(int));
        write(client,&action.hasText,sizeof(int));
        write(client,action.message,sizeof(action.message));
        close(client);
    }
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

int doInfo(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc)
{
    pClient auxClient;
    pUser auxUser;
    char text[MAXMESSAGE];
    int totalLines;
    
    action->idAction = SUCCESS_REQ;
    snprintf(action->message,MAXMESSAGE,"\nINFORMAÇÃO DO SISTEMA:\n");
    
    //////////////////
    //Enviar resposta
    //////////////////
    action->hasText = 1;
    write(client,&action->idAction,sizeof(int));
    write(client,&action->hasText,sizeof(int));
    write(client,action->message,sizeof(action->message));
    
    totalLines = db->totalClients + db->totalUsers + 5; //5 - Linhas de informação
    //Número de Linhas
    write(client,&totalLines,sizeof(int));
    
    snprintf(text,sizeof(text),"Data actual: %d\n",db->data);
    write(client,text,sizeof(text));
    
    snprintf(text,sizeof(text),"Total de cidades: %d\n",db->totalCidades);
    write(client,text,sizeof(text));

    snprintf(text,sizeof(text),"Total de voos disponíveis: %d\n",db->totalVoos);
    write(client,text,sizeof(text));

    snprintf(text,sizeof(text),"Utilizadores: %d\n",db->totalUsers);
    write(client,text,sizeof(text));
    
    //Utilizadores
    auxUser = db->users;
    while (auxUser)
    {
        snprintf(text,sizeof(text)," %s\n",auxUser->username);
        write(client,text,sizeof(text));
        auxUser = auxUser->next;
    }
    
    snprintf(text,sizeof(text),"Clientes activos: %d\n",db->totalClients);
    write(client,text,sizeof(text));
    
    //Clientes activos
    auxClient = db->clients;
    while (auxClient)
    {
        snprintf(text,sizeof(text)," %d - %s\n",auxClient->pid,getClientUsername(auxClient));
        write(client,text,sizeof(text));
        auxClient = auxClient->next;
    }
    
    close(client);
    //Resposta enviada
    return 1;
}

int doAddVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    pCidade auxCidadeOrigem, auxCidadeDestino;
    int auxID, auxDia;
    
    action->idAction = FAILED_REQ;
    //ID
    auxID = atoi(argv[1]);
    if (!auxID)
    {
        sprintf(action->message,"ID do voo não pode ser nulo");
        return 0;
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
            return 0;
        }
        auxCidadeDestino = findCidade(db->cidades,argv[3]);
        if (!auxCidadeDestino)
        {
            sprintf(action->message,"Cidade destino \"%s\" não existe",argv[3]);
            return 0;
        }
        //Dia (Data)
        auxDia = atoi(argv[4]);
        if (!auxDia || auxDia < db->data)
        {
            sprintf(action->message,"É necessário especificar um dia superior a %d",db->data);
            return 0;
        }
        
        //Adiciona o voo
        addVooByID(db,auxID,auxCidadeOrigem,auxCidadeDestino,auxDia);
        //Com sucesso
        action->idAction = SUCCESS_REQ;
    }
    return 0;
}

int doCancelVoo(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    int auxID;
    
    action->idAction = FAILED_REQ;
    //ID
    auxID = atoi(argv[1]);
    if (!auxID)
    {
        sprintf(action->message,"ID do voo não pode ser nulo");
        return 0;
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
    return 0;
}

int doMudaData(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    int auxDia;
    //Dia (Data)
    auxDia = atoi(argv[1]);
    //Verificar superioridade
    if (auxDia && auxDia > db->data)
    {
        db->data = auxDia;
        saveData(SODATA,db);
        checkVoos(db);
        action->idAction = SUCCESS_REQ;
    }
    else
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Data atual do sistema: %d",db->data);
    }
    return 0;
}

int doGetData(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    action->idAction = SUCCESS_REQ;
    sprintf(action->message,"Data atual do sistema: %d",db->data);
    return 0;
}

int doLista(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc)
{
    pVoo auxVoo;
    char text[MAXMESSAGE];
    
    action->idAction = SUCCESS_REQ;
    //Resposta
    if (db->voos)
    {
        snprintf(action->message,MAXMESSAGE,"\nVoos disponíveis (%d):\n",db->totalVoos);
        
        //////////////////
        //Enviar resposta
        //////////////////
        action->hasText = 1;
        write(client,&action->idAction,sizeof(int));
        write(client,&action->hasText,sizeof(int));
        write(client,action->message,sizeof(action->message));
        //Número de Linhas
        write(client,&db->totalVoos,sizeof(int));
        
        auxVoo = db->voos;
        while (auxVoo)
        {
            snprintf(text,sizeof(text)," %d: DIA %d, ORIGEM %s, DESTINO %s, LUGARES VAGOS: %d\n",
                     auxVoo->ID,
                     auxVoo->dia,
                     auxVoo->cidadeOrigem->nome,
                     auxVoo->cidadeDestino->nome,
                     auxVoo->capacidade-auxVoo->ocupacao);
            
            //Escrever para o client
            write(client,text,sizeof(text));
            
            auxVoo = auxVoo->next;
        }

        close(client);
        //Resposta enviada
        return 1;
    }
    else
    {
        strcpy(action->message,"Sem voos disponíveis\n");
        return 0;
    }
}

int doSeePast(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc)
{
    int f, totalVoos = 0;
    char text[MAXMESSAGE];
    
    //Cria ou abre o ficheiro de histórico de voos
    f = open(SOHISTORICO,O_RDONLY);
    if (f == -1)
    {
        action->idAction = FAILED_REQ;
        strcpy(action->message,"Não foi possível obter o histórico de voos\n");
        return 0;
    }
    
    action->idAction = SUCCESS_REQ;
    
    //ToDo - Melhorar esta situação
    //Verificar o total de voos ultrapassados
    while (read(f,text,sizeof(text)))
        totalVoos++;
    
    if (!totalVoos)
    {
        action->idAction = SUCCESS_REQ;
        strcpy(action->message,"Sem voos\n");
        return 0;
    }
    
    //Voltar a posicionar no início do ficheiro
    lseek(f,0,SEEK_SET);
    
    snprintf(action->message,MAXMESSAGE,"\nHistórico de voos (%d):\n",totalVoos);
    
    //////////////////
    //Enviar resposta
    //////////////////
    action->hasText = 1;
    write(client,&action->idAction,sizeof(int));
    write(client,&action->hasText,sizeof(int));
    write(client,action->message,sizeof(action->message));
    //Número de Linhas
    write(client,&totalVoos,sizeof(int));
    
    //Ler voos ultrapassados
    while (read(f,text,sizeof(text)))
        //Escrever para o client
        write(client,text,sizeof(text));
    
    close(f);
    return 1;
}

int doAddUser(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    //Verificar se está a adicionar o administrador
    if (strcmp(ADMIN,argv[1]) == 0)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Não é possível adicionar um administrador");
    }
    //Verificar se utilizador já existe
    else if (findUser(db->users,argv[1]))
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Utilizador \"%s\" já existe",argv[1]);
    }
    else if (argv[2] == 0)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Password não pode ser nula");
    }
    else if (strlen(argv[2]) <= MINPASSWORD)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Password tem que ser superior a %d caracteres",MINPASSWORD);
    }
    else if (strlen(argv[1]) > MAXLOGIN)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Username tem que ser inferior a %d caracteres",MAXLOGIN);
    }
    else if (strlen(argv[2]) > MAXLOGIN)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Password tem que ser inferior a %d caracteres",MAXLOGIN);
    }
    else
    {
        addUser(db,argv[1],argv[2]);
        saveUsers(SOAGENTES,db);
        action->idAction = SUCCESS_REQ;
    }
    return 0;
}

int doDelUser(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    //Verificar se está a adicionar o administrador
    if (strcmp(ADMIN,argv[1]) == 0)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Não é possível remover um administrador");
    }
    //Verificar se utilizador existe
    else if (!findUser(db->users,argv[1]))
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Utilizador \"%s\" não existe",argv[1]);
    }
    else
    {
        removeUser(db,argv[1]);
        saveUsers(SOAGENTES,db);
        action->idAction = SUCCESS_REQ;
    }
    return 0;
}

int doAddCity(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    if (findCidade(db->cidades,argv[1]))
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Cidade \"%s\" já existe",argv[1]);
    }
    else if (db->totalCidades > MAXCITIES)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Foi atingido o limite de cidades (%d)",MAXCITIES);
    }
    else
    {
        addCidade(db,argv[1],0);
        action->idAction = SUCCESS_REQ;
    }
    return 0;
}

int doDelCity(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    //Verificar se cidade existe
    if (!findCidade(db->cidades,argv[1]))
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Cidade \"%s\" não existe",argv[1]);
    }
    //Verificar se a cidade tem voos associados
    else if (findVooByCidade(db,argv[1]))
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Cidade \"%s\" tem voos associados",argv[1]);
    }
    else
    {
        removeCidade(db,argv[1]);
        action->idAction = SUCCESS_REQ;
    }
    return 0;
}

int doMudaPass(char *username, pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    pUser auxUser = findUser(db->users,username);
    //Verificar se está a adicionar o administrador
    if (strcmp(ADMIN,username) == 0)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Não é possível mudar a password do administrador");
    }
    //Verificar se utilizador existe
    else if (!auxUser)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Utilizador \"%s\" não existe",username);
    }
    //Verificar se a password antiga coincide com a password atual
    else if (strcmp(auxUser->password,argv[1]) != 0)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Password antiga não coincide");
    }
    else if (argv[2] == 0)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Password nova não pode ser nula");
    }
    else if (strlen(argv[2]) <= MINPASSWORD)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Password nova tem que ser superior a %d caracteres",MINPASSWORD);
    }
    else if (strlen(argv[2]) > MAXLOGIN)
    {
        action->idAction = FAILED_REQ;
        sprintf(action->message,"Password nova tem que ser inferior a %d caracteres",MAXLOGIN);
    }
    else
    {
        snprintf(auxUser->password,MAXLOGIN,"%s",argv[2]);
        saveUsers(SOAGENTES,db);
        action->idAction = SUCCESS_REQ;
    }
    return 0;
}

int doPesquisaVoos(pAction action, char *pipe, int client, pDatabase db, char *argv[], int argc)
{
    pVoo auxVoo;
    char text[MAXMESSAGE];
    int totalLines = 0;
    
    //ToDo - melhorar esta situação
    auxVoo = db->voos;
    while (auxVoo)
    {
        if (sameString(auxVoo->cidadeOrigem->nome,argv[1]) == 0 && sameString(auxVoo->cidadeDestino->nome,argv[2]) == 0)
        {
            totalLines++;
        }
        auxVoo = auxVoo->next;
    }
    
    action->idAction = SUCCESS_REQ;
    //Resposta
    if (totalLines)
    {
        snprintf(action->message,MAXMESSAGE,"\nVoos disponíveis (%s -> %s): %d\n",argv[1],argv[2],totalLines);
        
        //////////////////
        //Enviar resposta
        //////////////////
        action->hasText = 1;
        write(client,&action->idAction,sizeof(int));
        write(client,&action->hasText,sizeof(int));
        write(client,action->message,sizeof(action->message));
        //Número de Linhas
        write(client,&totalLines,sizeof(int));
        
        auxVoo = db->voos;
        while (auxVoo)
        {
            if (sameString(auxVoo->cidadeOrigem->nome,argv[1]) == 0 && sameString(auxVoo->cidadeDestino->nome,argv[2]) == 0)
            {
                snprintf(text,sizeof(text)," %d: DIA %d, ORIGEM %s, DESTINO %s, LUGARES VAGOS: %d\n",
                         auxVoo->ID,
                         auxVoo->dia,
                         auxVoo->cidadeOrigem->nome,
                         auxVoo->cidadeDestino->nome,
                         auxVoo->capacidade-auxVoo->ocupacao);
                
                //Escrever para o client
                write(client,text,sizeof(text));
            }
            auxVoo = auxVoo->next;
        }
        
        close(client);
        //Resposta enviada
        return 1;
    }
    else
    {
        snprintf(action->message,MAXMESSAGE,"Sem voos disponíveis para %s -> %s\n",argv[1],argv[2]);
        return 0;
    }
}

int doMarcaViagem(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{
    int auxID, auxNumPassaporte, res;
    pVoo auxVoo = NULL;
    action->idAction = FAILED_REQ;
    
    //ID do Voo
    auxID = atoi(argv[1]);
    if (auxID == 0)
    {
        sprintf(action->message,"ID \"%s\" do voo não é válido",argv[1]);
        return 0;
    }
    //Número do Passaporte
    auxNumPassaporte = atoi(argv[2]);
    if (auxNumPassaporte == 0)
    {
        sprintf(action->message,"Passaporte \"%s\" não é válido",argv[2]);
        return 0;
    }
    
    auxVoo = findVoo(db->voos,auxID);
    if (!auxVoo)
    {
        sprintf(action->message,"Voo \"%d\" não existe",auxID);
    }
    else
    {
        res = addPassaporte(auxVoo,auxNumPassaporte);
        switch (res) {
            case 0: //0 - Adicionado com sucesso
                action->idAction = SUCCESS_REQ;
                break;
            case 1: //1 - Já existe o passaporte
                sprintf(action->message,"Passaporte \"%d\" já existe",auxNumPassaporte);
                break;
            case 2: //2 - Voo não tem capacidade
                sprintf(action->message,"Voo \"%d\" está sobrelotado",auxID);
                break;
            case 3: //3 - Número do passaporte não pode ser nulo
                sprintf(action->message,"Passaporte não pode ser nulo");
                break;
            default:
                break;
        }
    }
    return 0;
}

int doDesmarcaViagem(pAction action, char *pipe, pDatabase db, char *argv[], int argc)
{    
    int auxID, auxNumPassaporte, res;
    pVoo auxVoo = NULL;
    action->idAction = FAILED_REQ;
    
    //ID do Voo
    auxID = atoi(argv[1]);
    if (auxID == 0)
    {
        sprintf(action->message,"ID \"%s\" do voo não é válido",argv[1]);
        return 0;
    }
    //Número do Passaporte
    auxNumPassaporte = atoi(argv[2]);
    if (auxNumPassaporte == 0)
    {
        sprintf(action->message,"Passaporte \"%s\" não é válido",argv[2]);
        return 0;
    }
    
    auxVoo = findVoo(db->voos,auxID);
    if (!auxVoo)
    {
        sprintf(action->message,"Voo \"%d\" não existe",auxID);
    }
    else
    {
        res = removePassaporte(auxVoo,auxNumPassaporte);
        switch (res) {
            case 0: //0 - Não foi removido / não existir o passaporte
                sprintf(action->message,"Passaporte \"%d\" não existe",auxNumPassaporte);
                break;
            case 1: //1 - Removido com sucesso
                action->idAction = SUCCESS_REQ;
                break;
            case 2: //2 - Número do passaporte não pode ser nulo
                sprintf(action->message,"Passaporte não pode ser nulo");
                break;
            default:
                break;
        }
    }
    return 0;
}