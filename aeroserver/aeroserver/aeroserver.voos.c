#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "aero.common.h"
#include "aeroserver.common.h"
#include "aeroserver.cidades.h"

void freeVoos(pVoo p)
{
    pVoo aux;
    while (p)
    {
        if (p->passaportes)
            free(p->passaportes);
        aux = p;
        p = p->next;
        free(aux);
    }
}

pVoo createVoo(pVoo p)
{
    pVoo new;
    new = malloc(sizeof(Voo));
    if (!new)
    {
        printf("(createVoo)Erro: não foi possível alocar memória\n");
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

void initPassaportes(pVoo voo)
{
    if (voo->passaportes)
        //ToDo: realloc deveria ser inicializado a 0
        voo->passaportes = realloc(voo->passaportes,sizeof(int) * voo->capacidade);
    else
        voo->passaportes = calloc(voo->capacidade,sizeof(int));
}

pVoo addVooByID(pDatabase db, int ID, pCidade origem, pCidade destino, int dia)
{
    pVoo aux;
    
    if (!origem || !destino || dia <= 0)
        return NULL;
    
    //Novo elemento
    aux = malloc(sizeof(Voo));
    if (!aux)
    {
        printf("(addVoo)Erro: Não foi possível alocar memória\n");
        return NULL;
    }
    
    //ToDo: Será necessário ficar ordenado por dia?!
    //ToDo: Será necessário verificar duplicações?!
    
    aux->prev = NULL;
    if (db)
    {
        if (!db->voos)
            db->voos = aux;
        
        if (db->lastVoo)
        {
            db->lastVoo->next = aux;
            if (!ID)
            {
                //ID baseado pelo último para garantir ser único
                aux->ID = db->lastVoo->ID;
                aux->ID++;
            }
            //O anterior será o último
            aux->prev = db->lastVoo;
        }
        else
        {
            //Para o primeiro e novo elemento
            if (!ID)
                aux->ID = 1;
        }
        //Actualiza o último e incrementa o número de voos
        db->lastVoo = aux;
        if (ID)
            aux->ID = ID;
        db->totalVoos++;
    }
    //Init do Buffer
    aux->cidadeOrigem = origem;
    aux->cidadeDestino = destino;
    aux->dia = dia;
    aux->capacidade = 5; //Default
    aux->ocupacao = 0;
    initPassaportes(aux);
    aux->next = NULL;
    return aux;
}

pVoo addVoo(pDatabase db, pCidade origem, pCidade destino, int dia)
{
    return addVooByID(db,0,origem,destino,dia);
}

pVoo newVoo(pDatabase db, char *origem, char *destino, int dia)
{
    pCidade cidadeOrigem, cidadeDestino;
    
    //Verificar se existe a Origem
    cidadeOrigem = findCidade(db->cidades,origem);
    if (!cidadeOrigem)
    {
        printf("(newVoo)Erro: cidade %s não existe",origem);
        return NULL;
    }
    //Verificar se existe o Destino
    cidadeDestino = findCidade(db->cidades,destino);
    if (!cidadeDestino)
    {
        printf("(newVoo)Erro: cidade %s não existe",destino);
        return NULL;
    }
    
    return addVoo(db,cidadeOrigem,cidadeDestino,dia);
}

int addPassaporte(pVoo voo, int passaporte)
{
    int i;
    
    //Resultados:
    //0 - Adicionado com sucesso
    //1 - Já existe o passaporte
    //2 - Voo não tem capacidade
    //3 - Número do passaporte não pode ser nulo
    
    if (!voo) return 3;
    if (!voo->passaportes) return 3;
    if (passaporte <= 0) return 3;
    
    //Verificar se já existe
    for (i=0; i<voo->capacidade; i++)
        if (voo->passaportes[i] == passaporte)
            return 1;
    //Verificar se tem capacidade para o passageiro
    if (voo->ocupacao + 1 > voo->capacidade)
        return 2;
    //Adicionar
    voo->passaportes[voo->ocupacao++] = passaporte;
    return 0;
}

int removePassaporte(pVoo voo, int passaporte)
{
    int i, removed = 0;
    
    //Resultados:
    //0 - Não foi removido / não existir o passaporte
    //1 - Removido com sucesso
    //2 - Número do passaporte não pode ser nulo
    
    if (!voo) return 2;
    if (!voo->passaportes) return 2;
    if (passaporte <= 0) return 2;
    
    //Verificar se já existe
    for (i=0; i<voo->capacidade; i++)
    {
        //Último elemento fica a zero
        if (removed && i == voo->capacidade-1)
        {
            voo->passaportes[i-1] = voo->passaportes[i];
            voo->passaportes[i] = 0;
            break;
        }
        
        //Se encontrou o elemento para eliminar então os restantes vão uma casa para trás
        if (removed)
        {
            voo->passaportes[i-1] = voo->passaportes[i];
        }
        
        //Dá sinal para remover
        if (voo->passaportes[i] == passaporte)
        {
            voo->passaportes[i] = 0;
            voo->ocupacao--;
            removed = 1;
        }
    }
    return removed;
}

pVoo findVoo(pVoo p, int ID)
{
    pVoo auxVoo = p;
    while (auxVoo)
    {
        if (ID == auxVoo->ID)
            return auxVoo;
        auxVoo = auxVoo->next;
    }
    return NULL;
}

pVoo findVooByCidade(pDatabase db, char *nomeCidade)
{
    pCidade auxCity;
    pVoo auxVoo;
    
    auxCity = findCidade(db->cidades,nomeCidade);
    if (auxCity)
    {
        auxVoo = db->voos;
        while (auxVoo)
        {
            if (auxVoo->cidadeOrigem == auxCity || auxVoo->cidadeDestino == auxCity)
            {
                return auxVoo;
            }
            auxVoo = auxVoo->next;
        }
    }
    return NULL;
}

void removeVoo(pDatabase db, int ID)
{
    pVoo auxVoo = findVoo(db->voos,ID);
    if (auxVoo)
    {
        //Se for primeiro elemento
        if (!auxVoo->prev)
        {
            if (auxVoo->next)
            {
                db->voos = auxVoo->next;
                auxVoo->next->prev = NULL;
            }
            else
            {
                db->voos = NULL;
                db->lastVoo = NULL;
            }
        }
        //Ultimo elemento
        else if (auxVoo->prev && !auxVoo->next)
        {
            db->lastVoo = auxVoo->prev;
            auxVoo->prev->next = NULL;
        }
        //Se for elemento interior, sem ser dos extremos
        else
        {
            auxVoo->prev->next = auxVoo->next;
            auxVoo->next->prev = auxVoo->prev;
        }
        auxVoo->next = NULL;
        auxVoo->prev = NULL;
        db->totalVoos--;
        freeVoos(auxVoo);
    }
}

void checkVoos(pDatabase db)
{
    pVoo auxVoo;
    int f, auxID;
    char text[MAXMESSAGE];
    
    //Cria ou abre o ficheiro de histórico de voos
    f = open(SOHISTORICO,O_CREAT | O_APPEND | O_RDWR, (S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | S_IROTH);
    if (f == -1)
    {
        printf("(checkVoos)Erro: não foi possível criar %s\n",SOHISTORICO);
        return;
    }
    
    auxVoo = db->voos;
    while (auxVoo)
    {
        //Verificar se o voo é passado
        if (auxVoo->dia < db->data)
        {
            auxID = auxVoo->ID;
            
            snprintf(text,MAXMESSAGE,"Voo %d, dia %d de origem %s e destino %s com %d passageiros (%d)\n",auxVoo->ID,
                     auxVoo->dia,
                     auxVoo->cidadeOrigem->nome,
                     auxVoo->cidadeDestino->nome,
                     auxVoo->ocupacao,
                     auxVoo->capacidade);
            
            write(f,text,sizeof(text));
            
            auxVoo = auxVoo->next;
            //Remover voo passado
            removeVoo(db,auxID);
        }
        else
            auxVoo = auxVoo->next;
    }
    //Modo Foreground: informação
    if (!db->inBackground)
    {
        //Posicionar no início do ficheiro
        lseek(f,0,SEEK_SET);
        printf("\nVoos ultrapassados:\n");
        while (read(f,text,sizeof(text)))
            printf("%s",text);
    }
    
    close(f);
}