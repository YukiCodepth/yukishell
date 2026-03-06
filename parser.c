#include "yukishell.h"

// Chops the raw string into an array of words
void parse_command(char *command, char **args) {
    int i = 0;
    args[i] = strtok(command, " ");
    while(args[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args[i] = strtok(NULL, " ");
    }
    args[i] = NULL; // Ensure null termination for execvp
}

// Scans for the pipe symbol and splits into two command arrays
int check_for_pipes(char **args, char **command2) {
    int pipeIndex = -1;
    int i = 0;
    
    // Find the pipe symbol
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
        return 1; // True: Pipe was found
    }
    return 0; // False: No pipe
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
            args[0] = NULL; // Prevent execution on syntax error
            return 1; 
        }

        *filename = args[redirIndex + 1]; // Save the filename
        args[redirIndex] = NULL; // Cut off the args array at the '>'
        return 1; // True: Redirection found
    }
    return 0; // False: No redirection
}
