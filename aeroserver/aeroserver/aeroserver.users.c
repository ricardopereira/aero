#include <stdio.h>
#include <string.h>

#include "aero.common.h"
#include "aeroserver.common.h"

void freeUsers(pUser p)
{
    pUser aux;
    while (p)
    {
        aux = p;
        p = p->next;
        free(aux);
    }
}

pUser findUser(pUser p, char *username)
{
    pUser auxUser;
    auxUser = p;
    while (auxUser)
    {
        if (strcmp(auxUser->username,username) == 0)
            return (auxUser);
        auxUser = auxUser->next;
    }
    return NULL;
}

int checkLogin(pDatabase db, char *username, char *password)
{
    pUser user = findUser(db->users,username);
    if (user)
    {
        if (strcmp(user->password,password) == 0)
            return LOGIN_OK;
    }
    return LOGIN_FAILED;
}