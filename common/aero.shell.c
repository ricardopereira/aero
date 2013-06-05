#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aero.common.h"

void promptShell(char *name)
{
    printf("%s:>",name);
}

void getCommandArgs(char *command, char *argv[], int *argc)
{
    char *tempArgv, *tempCommand;
    tempCommand = strdup(command);
    //Repartir os vários espaços
    tempArgv = strtok(tempCommand," ");
    while (tempArgv)
    {
        argv[(*argc)++] = tempArgv;
        tempArgv = strtok(NULL," ");
    }
}

void initCommand(char *command, char *argv[], int *argc)
{
    //Inicializar a vazio
    strcpy(command,"");
    while (*argc != 0)
        argv[(*argc)--] = NULL;
}

int readCommand(char *command, char *argv[], int *argc)
{
    char userInput;
    int idx;
    //Inicializar
    initCommand(command,argv,argc);
    //Obter comando do utilizador
    userInput = getchar();
    if (userInput != '\n')
    {
        idx = 0;
        //Ler caracter a caracter os dados introduzidos pelo utilizador,
        //limitando pelo máximo permitido
        while ((userInput != '\n') && (idx < MAXCOMMAND))
        {
            command[idx++] = userInput;
            //Próximo caracter
            userInput = getchar();
        }
        //Indicação do terminador da string
        command[idx] = 0;
        //Interpretar argumentos do comando
        getCommandArgs(command,argv,argc);
        return 0;
    }
    else
        return 1;
}

int checkCommand(char *validCommands[], int *validArgc, int maxValidCommands, char *command, int argc)
{
    int i = 0;
    char *bf;
    bf = validCommands[i];
    //ToDo: Validar argumentos?
    while (bf && i<maxValidCommands)
    {
        if (strcmp(bf,command) == 0 && validArgc[i] == argc-1)
            return 1;
        bf = validCommands[++i];
    }
    return 0;
}