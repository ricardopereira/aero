#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aero.common.h"
#include "aeroserver.common.h"

void freeUsers(pUser p)
{
    pUser aux;
    while (p)
    {
        aux = p;
        p = p->next;
        free(aux);
    }
}

pUser createUser(pUser p)
{
    pUser new;
    new = malloc(sizeof(User));
    if (!new)
    {
        printf("(createUser)Erro: não foi possível alocar memória\n");
        return p;
    }
    if (p)
    {
        /*Próximo elemento*/
        p->next = new;
    }
    new->prev = NULL;
    new->next = NULL;
    return new;
}

pUser addUser(pDatabase db, char *username, char *password)
{
    pUser new, auxUser;
    int endOfList = 1;
    
    /*Novo elemento*/
    new = createUser(NULL);
    strcpy(new->username,username);
    strcpy(new->password,password);
    
    if (db)
    {
        auxUser = db->users;
        while (auxUser)
        {
            /*Verificar duplicação*/
            if (strcmp(auxUser->username,new->username) == 0)
            {
                /*Duplicado*/
                return auxUser;
            }
            
            /*Verificar se é primeiro elemento da lista*/
            if (!auxUser->prev && sameString(new->username,auxUser->username) < 0)
            {
                /*Inserir na primeira posição*/
                new->prev = NULL;
                new->next = auxUser;
                auxUser->prev = new;
                db->users = new;
                endOfList = 0;
                break;
            }
            
            /*Verificar elementos da lista:*/
            if (sameString(new->username,auxUser->username) < 0)
            {
                /*Inserir no meio da lista*/
                new->prev = auxUser->prev;
                auxUser->prev->next = new;
                auxUser->prev = new;
                new->next = auxUser;
                endOfList = 0;
                break;
            }
            
            auxUser = auxUser->next;
        }
        
        if (endOfList)
        {
            /*Inicio da lista*/
            if (!db->users)
                db->users = new;
            /*Guardar a última cidade adicionada*/
            if (db->lastUser)
            {
                db->lastUser->next = new;
                new->prev = db->lastUser;
            }
            db->lastUser = new;
        }
        db->totalUsers++;
    }
    return new;
}

pUser findUser(pUser p, char *username)
{
    pUser auxUser;
    auxUser = p;
    while (auxUser)
    {
        if (strcmp(auxUser->username,username) == 0)
            return (auxUser);
        auxUser = auxUser->next;
    }
    return NULL;
}

void removeUser(pDatabase db, char *username)
{
    pUser auxUser = findUser(db->users,username);
    if (auxUser)
    {
        /*Se for primeiro elemento*/
        if (!auxUser->prev)
        {
            if (auxUser->next)
            {
                db->users = auxUser->next;
                auxUser->next->prev = NULL;
            }
            else
            {
                db->users = NULL;
                db->lastUser = NULL;
            }
        }
        /*Ultimo elemento*/
        else if (auxUser->prev && !auxUser->next)
        {
            db->lastUser = auxUser->prev;
            auxUser->prev->next = NULL;
        }
        /*Se for elemento interior, sem ser dos extremos*/
        else
        {
            auxUser->prev->next = auxUser->next;
            auxUser->next->prev = auxUser->prev;
        }
        auxUser->next = NULL;
        auxUser->prev = NULL;
        db->totalUsers--;
        freeUsers(auxUser);
    }
}

int checkLogin(pDatabase db, char *username, char *password)
{
    pUser user = findUser(db->users,username);
    if (user)
    {
        if (strcmp(user->password,password) == 0)
            return LOGIN_OK;
    }
    return LOGIN_FAILED;
}