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
        printf("\n======================================================\n");
        printf("                 YukiShell Help Menu                  \n");
        printf("======================================================\n");
        
        printf("[ Built-in Commands ]\n");
        printf("  help       : Displays this formatted help menu.\n");
        printf("  exit       : Safely terminates the shell.\n");
        printf("  cd <dir>   : Changes the current working directory.\n");
        printf("               (Example: cd Documents)\n\n");

        printf("[ Standard System Commands ]\n");
        printf("  YukiShell natively supports standard Linux commands\n");
        printf("  like: ls, pwd, whoami, cat, echo, mkdir, rm, etc.\n\n");

        printf("[ User Experience (UX) ]\n");
        printf("  Arrow Keys : Press UP or DOWN to cycle through your\n");
        printf("               command history.\n");
        printf("  TAB Key    : Press TAB to auto-complete file and\n");
        printf("               folder names as you type.\n\n");

        printf("[ Advanced Shell Features ]\n");
        printf("  Pipes (|)         : Passes the output of one command\n");
        printf("                      directly as input to the next.\n");
        printf("                      Example: ls -l | grep \"txt\"\n\n");
        
        printf("  Redirection (>)   : Saves the output of a command\n");
        printf("                      into a text file instead of the screen.\n");
        printf("                      Example: ls > directory_list.txt\n\n");
        
        printf("  Background (&)    : Runs a heavy or long task in the\n");
        printf("                      background so you can keep typing.\n");
        printf("                      Example: sleep 10 &\n");
        printf("======================================================\n\n");
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
