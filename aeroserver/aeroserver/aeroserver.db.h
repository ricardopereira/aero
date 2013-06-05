void freeDB(pDatabase p);
pDatabase loadDB(const char *path);
int saveDB(const char *path, pDatabase list);
//Utilizadores
pUser addUser(pDatabase db, char *username, char *password);
int loadAdmin(const char *path, pDatabase db);
int loadUsers(const char *path, pDatabase db);
int saveUsers(const char *path, pDatabase db);
void writeUserFile(int f,pDatabase p);