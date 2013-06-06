pCidade addCidade(pDatabase db, char *nome, int ID);
pCidade newCidade(pDatabase db, char *nome);
pCidade findCidade(pCidade p, char *nome);
pCidade findCidadeByID(pCidade p, int ID);
void removeCidade(pDatabase db, char *nome);

void freeCidades(pCidade p);