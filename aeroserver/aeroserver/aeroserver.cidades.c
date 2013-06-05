#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aeroserver.common.h"
#include "aeroserver.voos.h"

void freeCidades(pCidade p)
{
    pCidade aux;
    while (p)
    {
        aux = p;
        p = p->next;
        free(aux);
    }
}

pCidade createCidade(pCidade p)
{
    pCidade new;
    new = malloc(sizeof(Cidade));
    if (!new)
    {
        printf("(createCidade)Erro: não foi possível alocar memória\n");
        return p;
    }
    if (p)
    {
        //Próximo elemento
        p->next = new;
    }
    new->next = NULL;
    new->prev = NULL;
    return new;
}

pCidade addCidade(pDatabase db, char *nome, int ID)
{
    pCidade new, auxCidade;
    int endOfList = 1;
    
    //Novo elemento
    new = createCidade(NULL);
    new->nome = nome;
    
    if (db)
    {
        auxCidade = db->cidades;
        while (auxCidade)
        {
            //Verificar duplicação
            if (strcmp(auxCidade->nome,new->nome) == 0)
            {
                //Duplicado
                return auxCidade;
            }
            
            //Verificar se é primeiro elemento da lista
            if (!auxCidade->prev && strcmp(new->nome,auxCidade->nome) < 0)
            {
                //Inserir na primeira posição
                new->prev = NULL;
                new->next = auxCidade;
                auxCidade->prev = new;
                db->cidades = new;
                endOfList = 0;
                break;
            }
            
            //Verificar elementos da lista:
            if (strcmp(new->nome,auxCidade->nome) < 0)
            {
                //Inserir no meio da lista
                new->prev = auxCidade->prev;
                auxCidade->prev->next = new;
                auxCidade->prev = new;
                new->next = auxCidade;
                endOfList = 0;
                break;
            }
            
            auxCidade = auxCidade->next;
        }
        
        if (endOfList)
        {
            //Inicio da lista
            if (!db->cidades)
                db->cidades = new;
            //Guardar a última cidade adicionada
            if (db->lastCidade)
            {
                db->lastCidade->next = new;
                new->prev = db->lastCidade;
            }
            db->lastCidade = new;
        }
        
        if (ID == 0)
        {
            //Sequencial
            new->ID = ++db->lastIDCidade;
        }
        else
        {
            new->ID = ID;
            if (ID > db->lastIDCidade)
                db->lastIDCidade = ID;
        }
        db->totalCidades++;
    }
    return new;
}

pCidade newCidade(pDatabase db, char *nome)
{
    //Nova cidade com geração automática do ID
    return addCidade(db,nome,0);
}

pCidade findCidade(pCidade p, char *nome)
{
    pCidade auxCidade;
    auxCidade = p;
    while (auxCidade)
    {
        if (strcmp(nome,auxCidade->nome) == 0)
            return auxCidade;
        auxCidade = auxCidade->next;
    }
    return NULL;
}

pCidade findCidadeByID(pCidade p, int ID)
{
    pCidade auxCidade;
    auxCidade = p;
    while (auxCidade)
    {
        if (ID == auxCidade->ID)
            return auxCidade;
        auxCidade = auxCidade->next;
    }
    return NULL;
}

void RemoveCity(pDatabase DB,char *nome)
{
    pCidade auxCidade=NULL;
    pVoo auxVoo=NULL;
    
    if (DB)
    {
        auxCidade = findCidade(DB->cidades, nome);
    }
    
    if (auxCidade)
    {
        auxVoo = findVoobyCity(DB, auxCidade->nome);
        if (!auxVoo)
        {
        
            //Se for primeiro elemento
            if (!auxCidade->prev)
            {
                if (auxCidade->next)
                {
                    DB->cidades = auxCidade->next;
                    auxCidade->next->prev = NULL;
                }
                else
                {
                    DB->cidades = NULL;
                    DB->lastCidade = NULL;
                }
            }
            //Ultimo elemento
            else if (auxCidade->prev && !auxCidade->next)
            {
                auxCidade->prev->next = NULL;
            }
            //Se for elemento interior, sem ser dos extremos
            else
            {
                auxCidade->prev->next = auxCidade->next;
                auxCidade->next->prev = auxCidade->prev;
            }
            auxCidade->next = NULL;
            auxCidade->prev = NULL;
            DB->totalCidades--;
            freeCidades(auxCidade);
        }
        else
        {
            printf("A cidade %s que deseja remover tem voos associados", auxCidade->nome);
        }
    }
    
}