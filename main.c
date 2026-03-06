#include "yukishell.h"

int main() {
    char command[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    char *command2[MAX_ARGS];
    char *filename = NULL;

    while(1) {
        printf("YukiShell > ");
        
        // Handle EOF (Ctrl+D) gracefully
        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("\nClosing YukiShell...\n");
            break;
        }
        
        command[strcspn(command, "\n")] = 0;  // Remove newline character

        if(strlen(command) == 0) 
            continue;

        // 1. Parse the raw input into arguments (Handled by parser.c)
        parse_command(command, args);

        // 2. Check if it's a built-in command like 'cd' or 'exit' (Handled by builtins.c)
        if (execute_builtin(args) == 1) {
            continue; // Built-in handled it successfully, loop back to prompt
        }

        // 3. Check for pipes '|' (Handled by parser.c & executor.c)
        if (check_for_pipes(args, command2) == 1) {
            execute_piped(args, command2);
            continue;
        }

        // 4. Check for redirection '>' (Handled by parser.c & executor.c)
        if (check_for_redirection(args, &filename) == 1) {
            if (args[0] != NULL) { // Ensure no syntax error occurred
                execute_redirected(args, filename);
            }
            continue;
        }

        // 5. If no pipes or redirection, run a normal external command
        execute_external(args);
    }

    return 0;
}
