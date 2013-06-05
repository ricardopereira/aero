int checkServer();
void initRequest(pRequest p);
pRequest createClientPipe(int pid, int isAdmin);
void destroyClientPipe(pRequest req);
void destroyAdminPipe(pRequest req);
pRequest doLogin(char *username, char *password);
void sendRequest(char *pipeClient, pRequest req, char *command, char *argv[], int *argc, pAction resp);