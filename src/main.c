#include "../include/yukishell.h"

// --- Readline-Safe ANSI Color Codes ---
// \001 and \002 tell readline to ignore these invisible characters when calculating line length
#define RL_CYAN    "\001\x1b[36m\002"
#define RL_GREEN   "\001\x1b[32m\002"
#define RL_BLUE    "\001\x1b[34m\002"
#define RL_RESET   "\001\x1b[0m\002"
#define RL_BOLD    "\001\x1b[1m\002"

// Kills Zombie Processes running in the background
void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// Replaces "/home/username" with "~" for a cleaner prompt
void format_directory(char *cwd) {
    char *home = getenv("HOME");
    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
        char temp[1024];
        snprintf(temp, sizeof(temp), "~%s", cwd + strlen(home));
        strncpy(cwd, temp, 1024);
    }
}

int main() {
    char *command; 
    char *args[MAX_ARGS];
    char *command2[MAX_ARGS];
    char *filename = NULL;
    int background = 0; 
    
    char cwd[1024];    // Holds current directory path
    char prompt[2048]; // UPDATED: 2048 bytes to safely hold all color codes without warnings!

    signal(SIGCHLD, sigchld_handler);

    while(1) {
        // 1. Get current directory and format it
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            format_directory(cwd); 
        } else {
            strcpy(cwd, "unknown");
        }

        // 2. Build the ultimate dynamic prompt
        snprintf(prompt, sizeof(prompt), "%s%sYuki%s%sShell %s%s[%s]%s ❯ ", 
                 RL_BOLD, RL_CYAN, 
                 RL_GREEN, RL_BOLD, 
                 RL_BLUE, RL_BOLD, cwd, 
                 RL_RESET);

        // 3. Print the prompt and wait for user input
        command = readline(prompt);
        
        if (command == NULL) {
            printf("\nClosing YukiShell...\n");
            break;
        }
        
        // 4. Add command to Up/Down arrow history
        if(strlen(command) > 0) {
            add_history(command);
        } else {
            free(command); 
            continue;
        }

        // 5. Route the command through our architecture
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
        
        free(command); 
    }

    return 0;
}
