#include "logo.h"
#include <unistd.h>
#include <fcntl.h>
#include "../include/yukishell.h"
#include "../include/logo.h"
#include <readline/readline.h>
#include <readline/history.h>

// --- The Ghostty / True-Color Aesthetic ---
#define GHOST_LAVENDER "\001\033[38;2;180;190;254m\002" 
#define GHOST_PINK     "\001\033[38;2;245;194;231m\002" 
#define GHOST_BLUE     "\001\033[38;2;137;180;250m\002" 
#define RL_RESET       "\001\033[0m\002"
#define RL_BOLD        "\001\033[1m\002"

#define HISTORY_FILE ".yuki_history"
char global_history_path[1024]; 

#define MAX_ALIASES 50
typedef struct {
    char *name;
    char *value;
} Alias;

Alias alias_table[MAX_ALIASES];
int alias_count = 0;

void load_aliases() {
    char path[1024];
    char *home = getenv("HOME");
    if (!home) return;
    sprintf(path, "%s/.yukirc", home);

    FILE *file = fopen(path, "r");
    if (!file) return; 

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (strncmp(line, "alias ", 6) == 0) {
            char *name_start = line + 6;
            char *eq_sign = strchr(name_start, '=');
            if (!eq_sign) continue;

            *eq_sign = '\0'; 
            char *val_start = eq_sign + 1;

            if (val_start[0] == '"') val_start++;
            char *val_end = strrchr(val_start, '"');
            if (val_end) *val_end = '\0';
            else {
                val_end = strchr(val_start, '\n');
                if (val_end) *val_end = '\0';
            }

            if (alias_count < MAX_ALIASES) {
                alias_table[alias_count].name = strdup(name_start);
                alias_table[alias_count].value = strdup(val_start);
                alias_count++;
            }
        }
    }
    fclose(file);
}

void free_aliases() {
    for (int i = 0; i < alias_count; i++) {
        free(alias_table[i].name);
        free(alias_table[i].value);
    }
}

void get_history_path(char *path) {
    char *home = getenv("HOME");
    if (home) {
        sprintf(path, "%s/%s", home, HISTORY_FILE);
    } else {
        strcpy(path, HISTORY_FILE);
    }
}

void save_history_hook() {
    write_history(global_history_path);
}

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

// V15.0: Added "dash" to autocomplete
char *command_generator(const char *text, int state) {
    static int list_index, len;
    char *commands[] = {"help", "exit", "cd", "netscan", "serial", "neofetch", "ask", "dash", NULL};

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

extern void live_fastfetch_boot();

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

    get_history_path(global_history_path);
    read_history(global_history_path);
    load_aliases();
    
    atexit(save_history_hook);
    atexit(free_aliases); 

    if (argc > 1) {
        script_file = fopen(argv[1], "r");
        if (!script_file) {
            perror("Error opening script file");
            return 1;
        }
    }

    if (!script_file) {
        print_boot_screen();
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

            snprintf(prompt, sizeof(prompt), "%s%s╭─ yuki %s%s %s\n%s%s╰─❯%s ", 
                     RL_BOLD, GHOST_LAVENDER, GHOST_BLUE, cwd, RL_RESET, 
                     RL_BOLD, GHOST_PINK, RL_RESET);

            command = readline(geteuid() == 0 ? "\n\033[38;2;243;139;168m╭─ ⟁ [ SYSTEM OVERRIDE: ROOT ]\n╰─❯ \033[0m" : prompt);
            
            if (command == NULL) {
                printf("\n\033[38;2;243;139;168m[!] Closing YukiShell...\033[0m\n");
                break;
            }
            if(strlen(command) > 0) {
                add_history(command);
                
                if (strcmp(command, "clear") == 0 || strcmp(command, "cls") == 0) {
                    printf("\033[H\033[J");
                    free(command);
                    continue;
                }
            } else { 
                free(command); 
                continue; 
            }
        }

        char first_word[256] = {0};
        sscanf(command, "%255s", first_word);

        for (int i = 0; i < alias_count; i++) {
            if (strcmp(first_word, alias_table[i].name) == 0) {
                char new_cmd[2048];
                char *rest_of_cmd = command + strlen(first_word); 
                snprintf(new_cmd, sizeof(new_cmd), "%s%s", alias_table[i].value, rest_of_cmd);
                free(command);
                command = strdup(new_cmd);
                break; 
            }
        }

        parse_command(command, args, &background);

        if (args[0] != NULL) {
            if (execute_builtin(args) == 1) { free(command); continue; }
            
            if (check_for_pipes(args, command2) == 1) { execute_piped(args, command2); free(command); continue; }
            if (check_for_redirection(args, &filename) == 1) {
                if (args[0] != NULL) execute_redirected(args, filename);
                free(command); continue;
            }
            execute_external(args, background);
        }
        free(command); 
    }

    if (script_file) fclose(script_file);
    return 0;
}
