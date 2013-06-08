void freeDB(pDatabase p);
pDatabase loadDB(const char *path);
int saveDB(const char *path, pDatabase list);
//Utilizadores
int loadAdmin(const char *path, pDatabase db);
int loadUsers(const char *path, pDatabase db);
int saveUsers(const char *path, pDatabase db);
void writeUserFile(int f,pDatabase p);//Data
int loadData(const char *path, pDatabase db);
int saveData(const char *path, pDatabase db);