#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aeroserver.common.h"
#include "aeroserver.users.h"

void freeClients(pClient p)
{
    pClient aux;
    while (p)
    {
        aux = p;
        p = p->next;
        free(aux);
    }
}

pClient createClient(pClient p)
{
    pClient new;
    new = malloc(sizeof(Client));
    if (!new)
    {
        printf("(createClient)Erro: não foi possível alocar memória\n");
        return p;
    }
    if (p)
    {
        //Próximo elemento
        p->next = new;
    }
    new->prev = NULL;
    new->next = NULL;
    new->user = NULL;
    return new;
}

pClient addClient(pDatabase db, int pid)
{
    pClient new, auxClient;
    int endOfList = 1;
    
    //Novo elemento
    new = createClient(NULL);
    new->pid = pid;
    
    if (db)
    {
        auxClient = db->clients;
        while (auxClient)
        {
            //Verificar duplicação
            if (auxClient->pid == new->pid)
            {
                //Duplicado
                return auxClient;
            }
            
            //Verificar se é primeiro elemento da lista
            if (!auxClient->prev && (new->pid < auxClient->pid))
            {
                //Inserir na primeira posição
                new->prev = NULL;
                new->next = auxClient;
                auxClient->prev = new;
                db->clients = new;
                endOfList = 0;
                break;
            }
            
            //Verificar elementos da lista:
            if (new->pid < auxClient->pid)
            {
                //Inserir no meio da lista
                new->prev = auxClient->prev;
                auxClient->prev->next = new;
                auxClient->prev = new;
                new->next = auxClient;
                endOfList = 0;
                break;
            }
            
            auxClient = auxClient->next;
        }
        
        if (endOfList)
        {
            //Inicio da lista
            if (!db->clients)
                db->clients = new;
            //Guardar a última cidade adicionada
            if (db->lastClient)
            {
                db->lastClient->next = new;
                new->prev = db->lastClient;
            }
            db->lastClient = new;
        }
        db->totalClients++;
    }
    return new;
}

pClient addClientWithUser(pDatabase db, int pid, char *username)
{
    pClient new = addClient(db,pid);
    new->user = findUser(db->users,username);
    return new;
}

pClient findClient(pClient p, int pid)
{
    pClient auxClient = p;
    while (auxClient)
    {
        if (pid == auxClient->pid)
            return auxClient;
        auxClient = auxClient->next;
    }
    return NULL;
}

void removeClient(pDatabase db, int pid)
{
    pClient auxClient = findClient(db->clients,pid);
    if (auxClient)
    {
        //Se for primeiro elemento
        if (!auxClient->prev)
        {
            if (auxClient->next)
            {
                db->clients = auxClient->next;
                auxClient->next->prev = NULL;
            }
            else
            {
                db->clients = NULL;
                db->lastClient = NULL;
            }
        }
        //Ultimo elemento
        else if (auxClient->prev && !auxClient->next)
        {
            db->lastClient = auxClient->prev;
            auxClient->prev->next = NULL;
        }
        //Se for elemento interior, sem ser dos extremos
        else
        {
            auxClient->prev->next = auxClient->next;
            auxClient->next->prev = auxClient->prev;
        }
        auxClient->next = NULL;
        auxClient->prev = NULL;
        db->totalClients--;
        freeClients(auxClient);
    }
}

char *getClientUsername(pClient p)
{
    if (p && p->user)
        return p->user->username;
    else
        return NULL;
}