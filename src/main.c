#include "../include/yukishell.h"

#define RL_CYAN    "\001\x1b[36m\002"
#define RL_GREEN   "\001\x1b[32m\002"
#define RL_BLUE    "\001\x1b[34m\002"
#define RL_RESET   "\001\x1b[0m\002"
#define RL_BOLD    "\001\x1b[1m\002"

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void format_directory(char *cwd) {
    char *home = getenv("HOME");
    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
        char temp[1024];
        snprintf(temp, sizeof(temp), "~%s", cwd + strlen(home));
        strncpy(cwd, temp, 1024);
    }
}

char *command_generator(const char *text, int state) {
    static int list_index, len;
    char *commands[] = {"help", "exit", "cd", "netscan", "serial", NULL};

    if (!state) { list_index = 0; len = strlen(text); }

    while (commands[list_index]) {
        char *name = commands[list_index];
        list_index++;
        if (strncmp(name, text, len) == 0) return strdup(name);
    }
    return NULL;
}

char **yuki_autocomplete(const char *text, int start, int end) {
    if (start == 0) return rl_completion_matches(text, command_generator);
    return NULL;
}

int main(int argc, char **argv) {
    char *command; 
    char *args[MAX_ARGS];
    char *command2[MAX_ARGS];
    char *filename = NULL;
    int background = 0; 
    
    char cwd[1024];    
    char prompt[2048]; 
    FILE *script_file = NULL;

    signal(SIGCHLD, sigchld_handler);
    rl_attempted_completion_function = yuki_autocomplete;

    if (argc > 1) {
        script_file = fopen(argv[1], "r");
        if (!script_file) {
            perror("Error opening script file");
            return 1;
        }
    }

    while(1) {
        if (script_file) {
            char line[1024];
            if (fgets(line, sizeof(line), script_file) == NULL) break; 
            
            line[strcspn(line, "\n")] = 0; 
            if (strlen(line) == 0 || line[0] == '#') continue; 
            
            command = strdup(line);
        } else {
            if (getcwd(cwd, sizeof(cwd)) != NULL) format_directory(cwd); 
            else strcpy(cwd, "unknown");

            snprintf(prompt, sizeof(prompt), "%s%sYuki%s%sShell %s%s[%s]%s ❯ ", 
                     RL_BOLD, RL_CYAN, RL_GREEN, RL_BOLD, RL_BLUE, RL_BOLD, cwd, RL_RESET);

            command = readline(prompt);
            
            if (command == NULL) {
                printf("\nClosing YukiShell...\n");
                break;
            }
            if(strlen(command) > 0) add_history(command);
            else { free(command); continue; }
        }

        parse_command(command, args, &background);

        if (execute_builtin(args) == 1) { free(command); continue; }
        if (check_for_pipes(args, command2) == 1) { execute_piped(args, command2); free(command); continue; }
        if (check_for_redirection(args, &filename) == 1) {
            if (args[0] != NULL) execute_redirected(args, filename);
            free(command); continue;
        }

        execute_external(args, background);
        free(command); 
    }

    if (script_file) fclose(script_file);
    return 0;
}
