#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aero.common.h"
#include "aero.shell.h"
#include "aero.connect.h"
#include "aeroagent.jobs.h"

pRequest req;

#define TOTALCOMMANDS 3

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
    char *listCommands[] = {"exit","login","help"};
    char *listCommandsArgs[] = {"","[password]",""};
    int listCommandsArgc[] = {0,1,0};
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
            //ToDo
            printf("Comandos disponíveis:\n");
            printf(" close - fechar a aplicação\n");
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
        else
            doJob(command,commandArgv,&commandArgc,req);
    }
    destroyClientPipe(req);
    return 0;
}