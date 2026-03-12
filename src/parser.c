#include "../include/yukishell.h"

// The completely rewritten, Quote-Aware Parser State Machine
void parse_command(char *command, char **args, int *background) {
    *background = 0; 
    int i = 0;
    int in_quotes = 0;
    char *ptr = command;

    // 1. Skip any accidental spaces at the very beginning
    while (*ptr == ' ') ptr++;

    // If they just hit enter, bail out
    if (*ptr == '\0') {
        args[0] = NULL;
        return;
    }

    // Set the first argument to the start of our string
    args[i++] = ptr;

    // 2. The State Machine Loop
    while (*ptr != '\0') {
        if (*ptr == '"') {
            // Flip the switch! If it was 0, it becomes 1. If 1, it becomes 0.
            in_quotes = !in_quotes; 
        } 
        // 3. If we see a space AND we are NOT safely inside quotes
        else if (*ptr == ' ' && !in_quotes) {
            *ptr = '\0'; // Cut the string here using a null terminator
            ptr++;
            
            // Skip any extra spaces between words
            while (*ptr == ' ') ptr++; 
            
            if (*ptr != '\0') {
                args[i++] = ptr; // Mark the start of the next word
                if (i >= MAX_ARGS - 1) break; // Prevent memory overflow
            }
            continue; // Skip the ptr++ at the bottom since we already moved it
        }
        ptr++;
    }
    args[i] = NULL; 

    // 4. Clean up the actual quote characters so the user doesn't see them
    for (int j = 0; j < i; j++) {
        int len = strlen(args[j]);
        if (len >= 2 && args[j][0] == '"' && args[j][len-1] == '"') {
            args[j][len-1] = '\0'; // Erase the back quote
            args[j]++;             // Move pointer forward to hide the front quote
        }
    }

    // Look for the '&' operator to run tasks in the background
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        *background = 1;      
        args[i-1] = NULL;     
    }
}

// --- The pipe and redirection checkers remain exactly the same ---

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
