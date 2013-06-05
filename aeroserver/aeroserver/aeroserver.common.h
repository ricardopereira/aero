#define MAXLOGIN 20
#define MaxCitys 30

typedef struct recDatabase Database, *pDatabase;
typedef struct recCidade Cidade, *pCidade;
typedef struct recPassaporte Passaporte, *pPassaporte;
typedef struct recVoo Voo, *pVoo;
typedef struct recClient Client, *pClient;
typedef struct recUser User, *pUser;

struct recDatabase {
    int inBackground;
    //Cidades
    pCidade cidades;
    pCidade lastCidade;
    int totalCidades;
    int lastIDCidade;
    //Voos
    pVoo voos;
    pVoo lastVoo;
    int totalVoos;
    pVoo voosHistorico;
    //Terminais e Admin connectados
    pClient clients;
    pClient lastClient;
    int totalClients;
    //Utilizadores + Administrador
    pUser users;
    pUser lastUser;
    int totalUsers;
};

struct recCidade {
    int ID;
    char *nome;
    pCidade next;
    pCidade prev;
};

struct recVoo {
    int ID;
    pCidade cidadeOrigem;
    pCidade cidadeDestino;
    int dia;
    int capacidade;
    int ocupacao;
    int *passaportes;
    pVoo next;
    pVoo prev;
};

struct recClient {
    int pid;
    int isAdmin;
    pUser user;
    pClient next;
    pClient prev;
};

struct recUser {
    char username[MAXLOGIN];
    char password[MAXLOGIN];
    pUser next;
    pUser prev;
};

void showCidades(pCidade p);
void showVoosDisponiveis(pVoo p, int showPassaportes);
void showUtilizadores(pUser p, int showPassword);
void showClientesLigados(pClient p);
void upperCase(char *Str, char *newStr);