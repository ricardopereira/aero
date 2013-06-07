#include <stdio.h>
#include <stdlib.h>
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
    int i;
    for (i=0; i<=strlen(Str); i++)
    {
        if( (Str[i] > 96 ) && (Str[i] < 123)) //Verifica se é minuscula
            newStr[i] = Str[i] - 'a' + 'A'; //Transformação
        else
            newStr[i] = Str[i]; //Igual
    }
}

int sameString(const char *a,const char *b)
{
    char *strA, *strB;
    int res;
    strA = malloc(sizeof(a));
    if (!strA) return -1;
    strB = malloc(sizeof(b));
    if (!strB) return -1;
    strcpy(strA,a);
    strcpy(strB,b);
    upperCase(strA,strA);
    upperCase(strB,strB);
    res = strcmp(strA,strB);
    free(strA);
    free(strB);
    return res;
}