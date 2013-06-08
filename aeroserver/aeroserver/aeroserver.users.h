pUser addUser(pDatabase db, char *username, char *password);
void removeUser(pDatabase db, char *username);
pUser findUser(pUser p, char *username);
int checkLogin(pDatabase db, char *username, char *password);

void freeUsers(pUser p);