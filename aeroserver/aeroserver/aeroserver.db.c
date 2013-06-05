#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "aero.common.h"
#include "aeroserver.common.h"
#include "aeroserver.db.h"
#include "aeroserver.cidades.h"
#include "aeroserver.voos.h"
#include "aeroserver.clients.h"
#include "aeroserver.users.h"

void freeDB(pDatabase p)
{
    freeVoos(p->voos);
    //Em últmo por causa das dependências nos voos
    freeCidades(p->cidades);
    freeClients(p->clients);
    freeUsers(p->users);
    free(p);
}

pDatabase readFile(FILE *f)
{
    int totalCidades, totalVoos, aux, ID, idx;
    unsigned short sizeOf;
    char *str;
    pVoo auxVoo;
    pCidade cidadeOrigem, cidadeDestino;
    
    pDatabase db = malloc(sizeof(Database));
    //Init
    db->totalCidades = 0;
    db->totalVoos = 0;
    db->cidades = NULL;
    db->lastCidade = NULL;
    db->voos = NULL;
    db->lastVoo = NULL;
    db->clients = NULL;
    db->totalClients = 0;
    db->lastClient = NULL;
    db->users = NULL;
    db->totalUsers = 0;
    db->lastUser = NULL;
    db->data = 0;
    
    //Total de Cidades
    fread(&totalCidades,sizeof(int),1,f);
    //printf("%d\n",totalCidades);
    //Total de Voos
    fread(&totalVoos,sizeof(int),1,f);
    //printf("%d\n",totalVoos);
    
    //Percorrer o número de Cidades
    while (totalCidades--)
    {
        //Identificador
        fread(&aux,sizeof(int),1,f);
        //Ler o tamanho do nome da Cidade
        fread(&sizeOf,sizeof(unsigned short),1,f);
        //Allocar memória necessária para o nome
        str = malloc(sizeOf);
        //Ler o valor para a memória
        fread(str,sizeof(char),sizeOf,f);
        //Criar nova cidade na estrutura
        addCidade(db,str,aux);
        //ToDo: free(str);
    }
    //Percorrer o número de Cidades
    while (totalVoos--)
    {
        //Identificador
        fread(&ID,sizeof(int),1,f);
        //Buffer: dia
        fread(&aux,sizeof(int),1,f);
        //Buffer: cidadeOrigem
        fread(&idx,sizeof(int),1,f);
        cidadeOrigem = findCidadeByID(db->cidades,idx);
        //Buffer: cidadeDestino
        fread(&idx,sizeof(int),1,f);
        cidadeDestino = findCidadeByID(db->cidades,idx);
        
        //Adicionar voo
        auxVoo = addVooByID(db,ID,cidadeOrigem,cidadeDestino,aux);
        
        //Buffer: capacidade
        fread(&aux,sizeof(int),1,f);
        auxVoo->capacidade = aux;
        //Buffer: ocupacao
        fread(&aux,sizeof(int),1,f);
        auxVoo->ocupacao = aux;
        //Passaportes
        initPassaportes(auxVoo);
        idx = 0;
        while (aux--)
        {
            fread(&ID,sizeof(int),1,f);
            auxVoo->passaportes[idx] = ID;
            idx++;
        }
    }
    return db;
}

void writeFile(FILE *f, pDatabase db)
{
    pCidade auxCidade;
    pVoo auxVoo;
    unsigned short sizeOf;
    int i;
    //Gravar o número total de Cidades
    fwrite(&db->totalCidades,sizeof(int),1,f);
    //Gravar o número total de Voos
    fwrite(&db->totalVoos,sizeof(int),1,f);
    //Gravar Cidades
    auxCidade = db->cidades;
    while (auxCidade)
    {
        //Identificador
        fwrite(&auxCidade->ID,sizeof(int),1,f);
        //Gravar a string mais o /0
        sizeOf = strlen(auxCidade->nome) + 1;
        //Gravar o sem tamanho
        fwrite(&sizeOf,sizeof(unsigned short),1,f);
        //Gravar o conteúdo da string
        fwrite(auxCidade->nome,sizeof(char),sizeOf,f);
        //Próximo
        auxCidade = auxCidade->next;
    }
    //Gravar Voos
    auxVoo = db->voos;
    while (auxVoo)
    {
        //Identificador
        fwrite(&auxVoo->ID,sizeof(int),1,f);
        //Buffer
        fwrite(&auxVoo->dia,sizeof(int),1,f);
        fwrite(&auxVoo->cidadeOrigem->ID,sizeof(int),1,f);
        fwrite(&auxVoo->cidadeDestino->ID,sizeof(int),1,f);
        fwrite(&auxVoo->capacidade,sizeof(int),1,f);
        fwrite(&auxVoo->ocupacao,sizeof(int),1,f);
        //Passaportes
        //auxVoo->passaportes;
        for (i=0; i<auxVoo->ocupacao; i++)
            fwrite(&auxVoo->passaportes[i],sizeof(int),1,f);
        //Próximo
        auxVoo = auxVoo->next;
    }
}

pDatabase loadDB(const char *path)
{
    FILE *f = NULL;
    pDatabase startDB = NULL;
    
    f = fopen(path,"rb");
    if (!f)
    {
        printf("(loadDB)Erro: não foi possível abrir %s\n",path);
        return NULL;
    }
    //Carregar a estrutura do ficheiro
    startDB = readFile(f);
    
    fclose(f);
    return startDB;
}

int saveDB(const char *path, pDatabase db)
{
    FILE *f = NULL;
    
    f = fopen(path,"wb");
    if (!f)
    {
        printf("(saveDB)Erro: não foi possível criar %s\n",path);
        return 1;
    }
    //Gravar para o ficheiro
    writeFile(f,db);
    
    fclose(f);
    return 0;
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
        //Próximo elemento
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
    char Str1[MAXLOGIN];
    char Str2[MAXLOGIN];
    
    //Novo elemento
    new = createUser(NULL);
    strcpy(new->username,username);
    strcpy(new->password,password);
    
    if (db)
    {
        auxUser = db->users;
        while (auxUser)
        {
            //Verificar duplicação
            if (strcmp(auxUser->username,new->username) == 0)
            {
                //Duplicado
                return auxUser;
            }
        
            
            strcpy(Str1, new->username);
            strcpy(Str2, auxUser->username);
            upperCase(Str1, Str1);
            upperCase(Str2, Str2);
            //Verificar se é primeiro elemento da lista
            if (!auxUser->prev && strcmp(Str1,Str2) < 0)
            {
                //Inserir na primeira posição
                new->prev = NULL;
                new->next = auxUser;
                auxUser->prev = new;
                db->users = new;
                endOfList = 0;
                break;
            }
            
            strcpy(Str1, new->username);
            strcpy(Str2, auxUser->username);
            upperCase(Str1, Str1);
            upperCase(Str2, Str2);
            //Verificar elementos da lista:
            if (strcmp(Str1,Str2) < 0)
            {
                //Inserir no meio da lista
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
            //Inicio da lista
            if (!db->users)
                db->users = new;
            //Guardar a última cidade adicionada
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

int loadAdmin(const char *path, pDatabase db)
{
    int f;
    ssize_t len;
    char adminPassword[MAXLOGIN];
    if (!db) return 1;
    //Abrir para leitura
    f = open(path,O_RDONLY);
    if (f == -1)
    {
        printf("(loadAdmin)Erro: não foi possível abrir %s\n",path);
        return 1;
    }
    //Ler password
    len = read(f,adminPassword,sizeof(adminPassword)-1);
    //Fechar o ficheiro
    close(f);
    if (len >= 0)
    {
        //Colocar o terminador
        adminPassword[len] = 0;
        addUser(db,ADMIN,adminPassword);
        return 0;
    }
    return 1;
}

int loadUsers(const char *path, pDatabase db)
{
    int f;
    int TotalUsers=0;
    char UserName[MAXLOGIN];
    char UserPassword[MAXLOGIN];
    
    if (!db) return 1;
    
    //Abrir para leitura
    f = open(path, O_RDONLY,(S_IRUSR | S_IXOTH));
    if (f == -1)
    {
        printf("(loadUsers)Erro: não foi possível abrir %s\n",path);
        return 1;
    }
    
    read(f,&TotalUsers,sizeof(int));
    //Percorrer o número de Users
    while (TotalUsers)
    {
        //Ler o Nome Utilizador
        read(f,UserName,sizeof(UserName));
        //Ler password
        read(f,UserPassword,sizeof(UserPassword));
        //Criar nova User na estrutura
        addUser(db,UserName, UserPassword);
        TotalUsers--;
    }
    
    //Fechar o ficheiro
    close(f);
    return 0;
}

void writeUserFile(int f,pDatabase p)
{
    pUser auxUser=NULL;
    int totUsers;
    
    auxUser = p->users;
    if (auxUser)
    {
        if (f == -1)
        {
            printf("(writeUserFile)Erro: não foi possível criar\n");
        }
        else
        {
            totUsers = p->totalUsers -1;
            //Gravar para o ficheiro
            write(f,&totUsers,sizeof(int));
            
            while(auxUser)
            {
                if (strcmp(auxUser->username,ADMIN) < 0)
                {
                    write(f,auxUser->username,sizeof(auxUser->username));
                    write(f,auxUser->password,sizeof(auxUser->password));
                }
                auxUser = auxUser->next;
            }
        }
        close(f);
    }
    else
    {
        printf("(writeUserFile)Utilizadores não assignados!\n");
    }
}

int saveUsers(const char *path, pDatabase db)
{
    int f;
    if (!db) return 1;
    f = creat("SOAGENTES",(S_IRUSR | S_IWUSR) | (S_IRGRP | S_IXGRP) | (S_IWOTH));
    if (f == -1)
    {
        printf("(saveUsers)Erro: não foi possível criar %s\n",path);
        return 1;
    }
    //Gravar para o ficheiro
    writeUserFile(f,db);

    close(f);
    return 0;
}

int loadData(const char *path, pDatabase db)
{
    int f;
    if (!db) return 1;
    f = open(path,O_RDONLY);
    if (f == -1)
    {
        printf("(loadData)Erro: não foi possível abrir %s\n",path);
        return 1;
    }
    read(f,&db->data,sizeof(int));
    close(f);
    return 0;
}

int saveData(const char *path, pDatabase db)
{
    int f;
    if (!db) return 1;
    f = creat(path,(S_IRUSR | S_IWUSR) | (S_IRGRP | S_IXGRP) | (S_IWOTH));
    if (f == -1)
    {
        printf("(saveData)Erro: não foi possível criar %s\n",path);
        return 1;
    }
    write(f,&db->data,sizeof(int));
    close(f);
    return 0;
}