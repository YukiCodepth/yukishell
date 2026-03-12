#include "../include/yukishell.h"

void parse_command(char *command, char **args, int *background) {
    *background = 0; 
    int i = 0;
    int in_quotes = 0;
    char *ptr = command;

    while (*ptr == ' ') ptr++;

    if (*ptr == '\0') {
        args[0] = NULL;
        return;
    }

    args[i++] = ptr;

    while (*ptr != '\0') {
        if (*ptr == '"') {
            in_quotes = !in_quotes; 
        } 
        else if (*ptr == ' ' && !in_quotes) {
            *ptr = '\0'; 
            ptr++;
            
            while (*ptr == ' ') ptr++; 
            
            if (*ptr != '\0') {
                args[i++] = ptr; 
                if (i >= MAX_ARGS - 1) break; 
            }
            continue; 
        }
        ptr++;
    }
    args[i] = NULL; 

    for (int j = 0; j < i; j++) {
        int len = strlen(args[j]);
        if (len >= 2 && args[j][0] == '"' && args[j][len-1] == '"') {
            args[j][len-1] = '\0'; 
            args[j]++;             
        }
    }

    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        *background = 1;      
        args[i-1] = NULL;     
    }
}

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
        args[pipeIndex] = NULL; 
        int k = 0;
        for (int j = pipeIndex + 1; args[j] != NULL; j++) {
            command2[k] = args[j];
            k++;
        }
        command2[k] = NULL; 
        return 1; 
    }
    return 0; 
}

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
