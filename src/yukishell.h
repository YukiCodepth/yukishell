#ifndef YUKISHELL_H
#define YUKISHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 10
#define MAX_CMD_LEN 100

// --- Function Prototypes ---
void parse_command(char *command, char **args, int *background);
int check_for_pipes(char **args, char **command2);
int check_for_redirection(char **args, char **filename);
int execute_builtin(char **args);
int execute_external(char **args, int background);
int execute_piped(char **args, char **command2);
int execute_redirected(char **args, char *filename);

#endif
