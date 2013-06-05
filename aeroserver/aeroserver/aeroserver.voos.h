pVoo addVoo(pDatabase db, pCidade origem, pCidade destino, int dia);
pVoo addVooByID(pDatabase db, int ID, pCidade origem, pCidade destino, int dia);
pVoo newVoo(pDatabase db, char *origem, char *destino, int dia);

void initPassaportes(pVoo voo);
void addPassaporte(pVoo voo, int passaporte);
void removePassaporte(pVoo voo, int passaporte);
pVoo findVoobyCity(pDatabase DB, char *Cityname);

void freeVoos(pVoo p);