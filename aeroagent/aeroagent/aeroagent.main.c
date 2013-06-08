#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "aero.common.h"
#include "aero.shell.h"
#include "aero.connect.h"
#include "aeroagent.jobs.h"

pRequest req;

#define TOTALCOMMANDS 9

void stopClient(int sinal)
{
    fflush(stdout);
    //Destruir dependências
    destroyClientPipe(req);
    //Fechar a aplicação
    exit(0);
}

int main(int argc, const char * argv[])
{
    char command[MAXCOMMAND];
    char *commandArgv[MAXCOMMANDARGS];
    int commandArgc = 0;
    int loggedIn = 0;
    
    //Ao alterar a lista de comandos, é necessário alterar a constante TOTALCOMMANDS
    char *listCommands[] = {"exit","help","login","logout","lista","mudapass","pesquisa","marca","desmarca"};
    
    //Lista de argumentos de cada comando
    char *listCommandsArgs[] = {"","","[username] [password]","","","[passwordantiga] [passwordnova]","[origem] [destino]",
                                "[id] [passaporte]","[id] [passaporte]"};
    
    //Total de argumentos de cada comando
    int listCommandsArgc[] = {0,0,2,0,0,2,2,2,2};
    
    //Request
    req = NULL;
    
    //Sinal para parar
    signal(SIGUSR1,stopClient);
    
    while (1)
    {
        promptShell("agente");
        //Ler comando e interpretar os argumentos
        if (readCommand(command,commandArgv,&commandArgc) == 1)
            continue;
        //Verificar se o comando existe
        if (checkCommand(listCommands,listCommandsArgs,listCommandsArgc,TOTALCOMMANDS,commandArgv[0],commandArgc) == 0)
        {
            continue;
        }
        
        //Executar comando
        if (strcmp(commandArgv[0],"exit") == 0)
            break;
        else if (strcmp(commandArgv[0],"help") == 0)
        {
            printf("Comandos disponíveis:\n");
            showCommandList(listCommands,listCommandsArgs,listCommandsArgc,TOTALCOMMANDS);
        }
        else if (!loggedIn)
        {
            //Verificar login
            if (strcmp(commandArgv[0],"login") == 0)
            {
                //ToDo: colocar no checkCommand a validação de argumentos
                if (commandArgc != 3)
                {
                    printf("login:\n Argumentos possíveis: [username] [password]\n");
                }
                else
                {
                    //Efectuar login do Administrador
                    req = doLogin(commandArgv[1],commandArgv[2]);
                    loggedIn = req != NULL;
                }
            }
            else
                printf("Iniciar sessão pelo comando \"login\"\n");
        }
        else if (strcmp(commandArgv[0],"login") == 0)
        {
            printf("Sessão já foi iniciada: %s\n",req->username);
            printf("Utilize o comando \"logout\" para terminar a sessão actual\n");
        }
        else if (strcmp(commandArgv[0],"logout") == 0)
        {
            if (doLogout(req))
            {
                destroyClientPipe(req);
                req = NULL;
                loggedIn = 0;
            }
        }
        else
        {
            switch (doJob(command,commandArgv,&commandArgc,req)) {
                case PASSCHANGED:
                    if (doLogout(req))
                    {
                        printf("Efectue o início de sessão com a nova password\n");
                        destroyClientPipe(req);
                        req = NULL;
                        loggedIn = 0;
                    }
                    break;
            }
        }
    }
    //Logout
    doLogout(req);
    //Destruir dependências
    destroyClientPipe(req);
    return 0;
}