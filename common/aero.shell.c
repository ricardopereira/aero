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
    /*Repartir os vários espaços*/
    tempArgv = strtok(tempCommand," ");
    while (tempArgv)
    {
        argv[(*argc)++] = tempArgv;
        tempArgv = strtok(NULL," ");
    }
}

void initCommand(char *command, char *argv[], int *argc)
{
    /*Inicializar a vazio*/
    strcpy(command,"");
    while (*argc != 0)
        argv[(*argc)--] = NULL;
}

int readCommand(char *command, char *argv[], int *argc)
{
    char userInput;
    int idx;
    /*Inicializar*/
    initCommand(command,argv,argc);
    /*Obter comando do utilizador*/
    userInput = getchar();
    if (userInput != '\n')
    {
        idx = 0;
        /*Ler caracter a caracter os dados introduzidos pelo utilizador,*/
        /*limitando pelo máximo permitido*/
        while ((userInput != '\n') && (idx < MAXCOMMAND))
        {
            command[idx++] = userInput;
            /*Próximo caracter*/
            userInput = getchar();
        }
        /*Indicação do terminador da string*/
        command[idx] = 0;
        /*Interpretar argumentos do comando*/
        getCommandArgs(command,argv,argc);
        return 0;
    }
    else
        return 1;
}

int checkCommand(char *validCommands[], char *validArgs[], int *validArgc, int maxValidCommands, char *command, int argc)
{
    int i = 0;
    char *bf;
    bf = validCommands[i];

    while (bf && i<maxValidCommands)
    {
        /*Verificar se comando existe*/
        if (strcmp(bf,command) == 0)
        {
            /*Verificar argumentos*/
            if (validArgc[i] != argc-1)
            {
                if (strcmp(validArgs[i],"") != 0)
                    /*Argumento inválido*/
                    printf("Argumentos possíveis para \"%s\": %s\n",command,validArgs[i]);
                else
                    /*Sem argumentos*/
                    printf("Não é necessário argumentos para \"%s\"\n",command);
                return 0;
            }
            else
                return 1;
        }
        bf = validCommands[++i];
    }
    printf("Comando \"%s\" não existe\n",command);
    return 0;
}

void showCommandList(char *validCommands[], char *validArgs[], int *validArgc, int maxValidCommands)
{
    int i = 0;
    char *bf;
    bf = validCommands[i];
    while (bf && i<maxValidCommands)
    {
        if (validArgc[i])
            printf(" %s : %s (%d)\n",bf,validArgs[i],validArgc[i]);
        else
            printf(" %s\n",bf);
        bf = validCommands[++i];
    }
}