#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aeroserver.common.h"
#include "aeroserver.voos.h"

pDatabase testSave()
{
    pDatabase db = malloc(sizeof(Database));

    db->totalCidades = 0;
    db->totalVoos = 0;
    db->cidades = NULL;
    db->lastCidade = NULL;
    db->voos = NULL;
    db->lastVoo = NULL;
    
    newCidade(db,"Coimbra");
    newCidade(db,"Amaral");
    newCidade(db,"Coimbra");
    newCidade(db,"Lisboa");
    newCidade(db,"Febres");
    newCidade(db,"Balsas");
    
    newVoo(db,"Coimbra","Febres",13);
    newVoo(db,"Febres","Balsas",25)->capacidade = 15;
    
    saveDB("db.bin",db);
    return db;
}