#include "yukishell.h"

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char *command; // Changed from array to pointer for readline
    char *args[MAX_ARGS];
    char *command2[MAX_ARGS];
    char *filename = NULL;
    int background = 0; 

    signal(SIGCHLD, sigchld_handler);

    while(1) {
        // readline automatically prints the prompt and handles arrows/tabs!
        command = readline("YukiShell > ");
        
        // Handle EOF (Ctrl+D) gracefully
        if (command == NULL) {
            printf("\nClosing YukiShell...\n");
            break;
        }
        
        // If the user typed something, add it to the Up/Down arrow history
        if(strlen(command) > 0) {
            add_history(command);
        } else {
            free(command); // Clean up memory if they just hit Enter
            continue;
        }

        parse_command(command, args, &background);

        if (execute_builtin(args) == 1) {
            free(command); 
            continue; 
        }

        if (check_for_pipes(args, command2) == 1) {
            execute_piped(args, command2);
            free(command);
            continue;
        }

        if (check_for_redirection(args, &filename) == 1) {
            if (args[0] != NULL) { 
                execute_redirected(args, filename);
            }
            free(command);
            continue;
        }

        execute_external(args, background);
        
        free(command); // Always free the memory readline gave us at the end of the loop
    }

    return 0;
}
