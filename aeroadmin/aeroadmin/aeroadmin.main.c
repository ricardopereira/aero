#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aero.common.h"
#include "aero.shell.h"
#include "aero.connect.h"
#include "aeroadmin.jobs.h"

pRequest req;

#define TOTALCOMMANDS 10

void stopClient(int sinal)
{
    fflush(stdout);
    //Destruir dependências
    destroyAdminPipe(req);
    //Fechar a aplicação
    exit(0);
}

int main(int argc, const char * argv[])
{
    char command[MAXCOMMAND];
    char *commandArgv[MAXCOMMANDARGS];
    int commandArgc = 0;
    int idx, loggedIn = 0;
    //Ao alterar a lista de comandos, é necessário alterar a constante TOTALCOMMANDS
    char *listCommands[] = {"exit","login","show","help","shutdown","info","addcity","mudadata","addvoo","lista"};
    char *listCommandsArgs[] = {"","[password]","","","","","[nome]","[dia]","[id] [origem] [destino] [dia]",""};
    int listCommandsArgc[] = {0,1,0,0,0,0,1,1,4,0};
    //Request
    req = NULL;
    
    //Verificar se o admin já está a correr
    if (access(ADMIN,W_OK) != -1)
    {
        printf("Administrador já está em execução\n");
        return 1;
    }
    
    //Sinal para parar
    signal(SIGUSR1,stopClient);
    
    while (1)
    {
        promptShell("admin");
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
        else if (strcmp(commandArgv[0],"show") == 0)
        {
            //Mostrar leitura de argumentos para verificação
            printf("Comando: %s (%d)\n",command,commandArgc);
            idx = 0;
            while (idx < commandArgc)
                printf("Arg%d: %s\n",idx,commandArgv[idx++]);
            fflush(stdout);
        }
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
                //Verificar se o admin já está a correr
                if (access(ADMIN,W_OK) != -1)
                {
                    printf("Administrador já está em execução\n");
                    return 1;
                }
                //ToDo: colocar no checkCommand a validação de argumentos
                if (commandArgc != 2)
                {
                    printf("login:\n Argumentos possíveis: [password]\n");
                }
                else
                {
                    //Efectuar login do Administrador
                    req = doLogin(ADMIN,commandArgv[1]);
                    loggedIn = req != NULL;
                }
            }
            else
                printf("Iniciar sessão pelo comando \"login\"\n");
        }
        else
            doJob(command,commandArgv,&commandArgc,req);
    }
    destroyAdminPipe(req);
    return 0;
}