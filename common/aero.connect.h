int checkServer();
void initRequest(pRequest p);
pRequest createClientPipe(int pid, int isAdmin);
void destroyClientPipe(pRequest req);
void destroyAdminPipe(pRequest req);

pRequest doLogin(char *username, char *password);
pRequest doLoginWithRetry(char *username, char *password, int *retryCount);
int doLogout(pRequest req);

void sendRequest(char *pipeClient, pRequest req, char *command, pAction resp);
void sendRequestWithStatus(char *pipeClient, pRequest req, char *command, pAction resp);
void sendRequestWithMessage(char *pipeClient, pRequest req, char *command, pAction resp);
void sendRequestWithFail(char *pipeClient, pRequest req, char *command, pAction resp);
void sendRequestWithExtendedText(char *pipeClient, pRequest req, char *command, pAction resp);