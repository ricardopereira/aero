pClient addClient(pDatabase db, int pid);
void removeClient(pDatabase db, int pid);
pClient findClient(pClient p, int pid);
char *getClientUsername(pClient p);

void freeClients(pClient p);