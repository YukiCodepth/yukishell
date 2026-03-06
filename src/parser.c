#include "../include/yukishell.h"

// Chops the raw string into an array of words and checks for '&'
void parse_command(char *command, char **args, int *background) {
    *background = 0; // Default to foreground
    int i = 0;
    
    args[i] = strtok(command, " ");
    while(args[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args[i] = strtok(NULL, " ");
    }
    args[i] = NULL; 

    // Check if the very last argument is '&'
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        *background = 1;      // Set the flag to true
        args[i-1] = NULL;     // Remove the '&' from the command so execvp ignores it
    }
}

// Scans for the pipe symbol and splits into two command arrays
int check_for_pipes(char **args, char **command2) {
    int pipeIndex = -1;
    int i = 0;
    
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            pipeIndex = i;
            break;
        }
        i++;
    }

    if (pipeIndex != -1) {
        args[pipeIndex] = NULL; // Terminate the first command
        
        int k = 0;
        for (int j = pipeIndex + 1; args[j] != NULL; j++) {
            command2[k] = args[j];
            k++;
        }
        command2[k] = NULL; // Terminate the second command
        return 1; 
    }
    return 0; 
}

// Scans for the redirection symbol and isolates the filename
int check_for_redirection(char **args, char **filename) {
    int redirIndex = -1;
    int i = 0;

    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            redirIndex = i;
            break;
        }
        i++;
    }

    if (redirIndex != -1) {
        if (args[redirIndex + 1] == NULL) {
            printf("Syntax error: expected filename after '>'\n");
            args[0] = NULL; 
            return 1; 
        }

        *filename = args[redirIndex + 1]; 
        args[redirIndex] = NULL; 
        return 1; 
    }
    return 0; 
}
