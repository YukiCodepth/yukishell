#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 10

int main() {

    char command[100];
    char *args[MAX_ARGS];

    while(1) {

        printf("YukiShell > ");
        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;  // Remove newline character

        if(strlen(command) == 0)  // Skip empty commands
            continue;

        int i = 0;
        args[i] = strtok(command, " ");

        while(args[i] != NULL && i < MAX_ARGS-1) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        /* Built-In Commands */
        if(strcmp(args[0], "exit") == 0) {
            printf("Closing YukiShell...\n");
            break;
        }

        if(strcmp(args[0], "help") == 0) {
            printf("Available commands:\n");
            printf("help\n");
            printf("exit\n");
            printf("cd <dir>\n");
            printf("ls\n");
            printf("pwd\n");
            printf("whoami\n");
            continue;
        }

        if(strcmp(args[0], "cd") == 0) {
            // Built-in command: cd (change directory)
            if(args[1] == NULL) {
                printf("cd: missing argument\n");
            } else {
                if(chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        }

        /* External Commands */
        pid_t pid = fork();

        if(pid == 0) {
            execvp(args[0], args);
            perror("Command failed");
        } else {
            wait(NULL);  // Parent waits for the child to finish
        }
    }

    return 0;
}
