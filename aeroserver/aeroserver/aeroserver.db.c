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
    int f, i=0;
    char aux, data[MAXDATASTR];
    if (!db) return 1;
    //Abrir ficheiro
    f = open(path,O_RDONLY);
    if (f == -1)
    {
        if (!db->inBackground)
        {
            printf("(loadData)Erro: não foi possível abrir %s\n",path);
            printf("(loadData)criar %s com data por defeito\n",path);
        }
        //Cria ficheiro com data por defeito
        db->data = DEFAULTDATA;
        saveData(path,db);
        f = open(path,O_RDONLY);
        if (f == -1) return 1;
    }
    //Ler o valor (data é um inteiro mas gravado em caracteres)
    while (read(f,&aux,sizeof(char)))
    {
        if (i == MAXDATASTR-1) break;
        data[i++] = aux;
    }
    //Converter para inteiro
    db->data = atoi(data);
    close(f);
    return 0;
}

int saveData(const char *path, pDatabase db)
{
    int f, i=0;
    char data[MAXDATASTR];
    if (!db) return 1;
    //Cria ou substituir ficheiro
    
    f = creat(path,(S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | S_IROTH);
    if (f == -1)
    {
        printf("(saveData)Erro: não foi possível criar %s\n",path);
        return 1;
    }
    //Converter para string
    sprintf(data,"%d",db->data);
    //Gravar cada caracter
    while (*(data+i))
        write(f,data+i++,sizeof(char));
    //Gravar terminador
    write(f,data+i,sizeof(char));
    close(f);
    return 0;
}