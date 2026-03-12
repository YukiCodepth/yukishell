#include "../include/yukishell.h"

int execute_builtin(char **args) {
    if (args[0] == NULL) {
        return 1; 
    }

    if(strcmp(args[0], "exit") == 0) {
        printf("Closing YukiShell...\n");
        exit(0); 
    }

    if(strcmp(args[0], "help") == 0) {
        printf("\n\x1b[36m\x1b[1m======================================================\x1b[0m\n");
        printf("\x1b[1m                 YukiShell Help Menu                  \x1b[0m\n");
        printf("\x1b[36m\x1b[1m======================================================\x1b[0m\n");
        
        printf("\x1b[32m[ Core Commands ]\x1b[0m\n");
        printf("  help       : Displays this formatted help menu.\n");
        printf("  exit       : Safely terminates the shell process.\n");
        printf("  cd <dir>   : Changes the current working directory.\n\n");

        printf("\x1b[32m[ Advanced Data Routing ]\x1b[0m\n");
        printf("  Pipes (|)         : Chains commands (e.g., ls -l | grep txt)\n");
        printf("  Redirection (>)   : Saves output to file (e.g., echo hi > a.txt)\n\n");
        
        printf("\x1b[32m[ Process Management ]\x1b[0m\n");
        printf("  Background (&)    : Runs task silently (e.g., sleep 10 &)\n\n");

        printf("\x1b[32m[ Syntax & Parsing ]\x1b[0m\n");
        printf("  Quotes (\" \")      : Ignores special characters (&, |, >) inside text.\n");
        printf("                      (Example: echo \"hello & world > test\")\n\n");

        printf("\x1b[32m[ User Experience ]\x1b[0m\n");
        printf("  Arrow Keys : Press UP/DOWN to cycle command history.\n");
        printf("  TAB Key    : Press TAB to auto-complete files/folders.\n");
        printf("  UI Prompt  : Dynamically tracks your current working dir.\n");
        
        printf("\x1b[36m\x1b[1m======================================================\x1b[0m\n\n");
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

    return 0; 
}
