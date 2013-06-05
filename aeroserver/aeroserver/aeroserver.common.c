#include <stdio.h>
#include <string.h>

#include "aeroserver.common.h"

void showCidades(pCidade p)
{
    pCidade auxCidade = p;
    while (auxCidade)
    {
        printf("%d - %s",auxCidade->ID,auxCidade->nome);
        //write(1,auxCidade->nome,strlen(auxCidade->nome));
        printf("\n");
        auxCidade = auxCidade->next;
    }
}

void showVoosDisponiveis(pVoo p, int showPassaportes)
{
    pVoo auxVoo;
    int i;
    auxVoo = p;
    while (auxVoo)
    {
        printf("Voo %d, dia %d de origem %s e destino %s com %d passageiros (%d)",auxVoo->ID,auxVoo->dia,
               auxVoo->cidadeOrigem->nome,
               auxVoo->cidadeDestino->nome,
               auxVoo->ocupacao,
               auxVoo->capacidade);
        printf("\n");
        //Passaportes
        if (showPassaportes && auxVoo->ocupacao > 0)
            for (i=0; i<auxVoo->ocupacao; i++)
                printf("P%d: %d\n",i+1,auxVoo->passaportes[i]);
        auxVoo = auxVoo->next;
    }
}

void showUtilizadores(pUser p, int showPassword)
{
    pUser auxUser = p;
    while (auxUser)
    {
        printf("%s",auxUser->username);
        if (showPassword)
            printf(": %s",auxUser->password);
        printf("\n");
        auxUser = auxUser->next;
    }
}

void showClientesLigados(pClient p)
{
    pClient auxClient = p;
    while (auxClient)
    {
        if (auxClient->user)
            printf("%d: %s\n",auxClient->pid,auxClient->user->username);
        else
            printf("%d: %s\n",auxClient->pid,"Administrador");
        auxClient = auxClient->next;
    }
}

void upperCase(char *Str, char *newStr)
{
    for(int i=0; i<=strlen(Str);i++)
    {
        if( (Str[i] > 96 ) && (Str[i] < 123) ) // verifica se é minuscula
            newStr[i] = Str[i] - 'a' + 'A';   //transformação
        else
            newStr[i] = Str[i]; //Igual
    }
}