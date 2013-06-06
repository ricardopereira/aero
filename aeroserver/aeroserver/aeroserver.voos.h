pVoo addVoo(pDatabase db, pCidade origem, pCidade destino, int dia);
pVoo addVooByID(pDatabase db, int ID, pCidade origem, pCidade destino, int dia);
pVoo newVoo(pDatabase db, char *origem, char *destino, int dia);
pVoo findVoo(pVoo p, int ID);
pVoo findVooByCidade(pDatabase db, char *nomeCidade);
void removeVoo(pDatabase db, int ID);

void checkVoos(pDatabase db);

void initPassaportes(pVoo voo);
void addPassaporte(pVoo voo, int passaporte);
void removePassaporte(pVoo voo, int passaporte);

void freeVoos(pVoo p);