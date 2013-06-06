void promptShell(char *name);
void initCommand(char *command, char *argv[], int *argc);
int readCommand(char *command, char *argv[], int *argc);
void getCommandArgs(char *command, char *argv[], int *argc);
int checkCommand(char *validCommands[], char *validArgs[], int *validArgc, int maxValidCommands, char *command, int argc);