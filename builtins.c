#include "yukishell.h"

// Returns 1 if a built-in command was found and executed, 0 if not.
int execute_builtin(char **args) {
    if (args[0] == NULL) {
        return 1; // Empty command, just return to prompt
    }

    if(strcmp(args[0], "exit") == 0) {
        printf("Closing YukiShell...\n");
        exit(0); // Actually exit the program here
    }

    if(strcmp(args[0], "help") == 0) {
        printf("Available commands:\n");
        printf("  help\n");
        printf("  exit\n");
        printf("  cd <dir>\n");
        printf("  ls\n");
        printf("  pwd\n");
        printf("  whoami\n");
        printf("  ls | grep <pattern>\n");
        printf("  ls > file.txt\n");
        return 1;
    }

    if(strcmp(args[0], "cd") == 0) {
        if(args[1] == NULL) {
            printf("cd: missing argument\n");
        } else {
            if(chdir(args[1]) != 0) {
                perror("cd failed");
            }
        }
        return 1;
    }

    return 0; // Not a built-in command
}
