#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "aero.common.h"
#include "aeroserver.common.h"
#include "aeroserver.db.h"
#include "aeroserver.service.h"

int main(int argc, const char * argv[], char * envp[])
{
    int i,pid;
    
    if (argc == 2)
    {
        //Mostrar Variáveis de Ambiente
        if (strcmp(argv[1],"env") == 0)
        {
            for (i=0; envp[i]!=(char *)0; i++)
                printf("envp[%d] = <%s>\n", i, envp[i]);
        }
        //Modo "Foreground"
        else if (strcmp(argv[1],"fg") == 0)
        {
            printf("Servidor em foreground\n");
            fflush(stdout);
            startServer(0);
            return 0;
        }
    }
    //Colocar em Background
    pid = fork();
    if (pid < 0)
        return 1; //Erro
    else if (pid != 0)
        exit(0); //Parent
    else
        startServer(1);
    return 0;
}