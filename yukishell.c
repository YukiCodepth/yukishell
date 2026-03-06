#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>  // For file control options

#define MAX_ARGS 10

int main() {
    char command[100];
    char *args[MAX_ARGS];
    char *command2[MAX_ARGS];

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

        // Parsing command and arguments
        int i = 0;
        args[i] = strtok(command, " ");
        while(args[i] != NULL && i < MAX_ARGS-1) {
            i++;
            args[i] = strtok(NULL, " ");
        }
        // Ensure args is always NULL terminated for normal execution
        args[i] = NULL; 

        // Handle built-in commands
        if(strcmp(args[0], "exit") == 0) {
            printf("Closing YukiShell...\n");
            break;
        }

        if(strcmp(args[0], "help") == 0) {
            printf("Available commands:\n");
            printf("  help\n");
            printf("  exit\n");
            printf("  cd <dir>\n");
            printf("  ls\n");
            printf("  pwd\n");
            printf("  whoami\n");
            printf("  ls | grep <pattern>\n");
            printf("  ls > file.txt\n");
            continue;
        }

        if(strcmp(args[0], "cd") == 0) {
            if(args[1] == NULL) {
                printf("cd: missing argument\n");
            } else {
                if(chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        }

        /* --- 1. HANDLE PIPES (|) --- */
        int pipeIndex = -1;
        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], "|") == 0) {
                pipeIndex = j;
                break;
            }
        }

        if (pipeIndex != -1) {
            // Split the command before and after the pipe
            args[pipeIndex] = NULL; // Terminate the first command
            
            int k = 0;
            for (int j = pipeIndex + 1; j < i; j++) {
                command2[k] = args[j];
                k++;
            }
            command2[k] = NULL; // <-- THE FIX: Terminate the second command!

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("Pipe failed");
                continue;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                // First child process (before the pipe)
                close(pipefd[0]);  // Close unused read end
                dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe
                close(pipefd[1]);  // Close write end after duping
                
                execvp(args[0], args);  // Execute first command
                perror("Command 1 failed");
                return 1;
            } 
            
            pid_t pid2 = fork();
            if (pid2 == 0) {
                // Second child process (after the pipe)
                close(pipefd[1]);  // Close unused write end
                dup2(pipefd[0], STDIN_FILENO);  // Redirect stdin from pipe
                close(pipefd[0]);  // Close read end after duping
                
                execvp(command2[0], command2);  // Execute second command
                perror("Command 2 failed");
                return 1;
            }
            
            // Parent process
            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0);  // Wait specifically for child 1
            waitpid(pid2, NULL, 0);  // Wait specifically for child 2
            
            continue; // Go back to the prompt
        }

        /* --- 2. HANDLE REDIRECTION (>) --- */
        // robust check: look through all arguments for '>'
        int redirIndex = -1;
        for (int j = 0; j < i; j++) {
            if (strcmp(args[j], ">") == 0) {
                redirIndex = j;
                break;
            }
        }

        if (redirIndex != -1) {
            if (args[redirIndex + 1] == NULL) {
                printf("Syntax error: expected filename after '>'\n");
                continue;
            }

            char *filename = args[redirIndex + 1];
            args[redirIndex] = NULL;  // Nullify the '>' so execvp stops reading here

            int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
            if (fd == -1) {
                perror("Failed to open file");
                continue;
            }

            pid_t pid = fork();
            if(pid == 0) {
                dup2(fd, STDOUT_FILENO);  // Redirect stdout to file
                close(fd);                // Close original fd since it's now attached to stdout
                
                execvp(args[0], args);  // Execute the command
                perror("Command failed");
                return 1;
            } else {
                wait(NULL);
                close(fd);  // Parent needs to close the file descriptor too
            }
            continue;
        }

        /* --- 3. NORMAL COMMAND EXECUTION --- */
        pid_t pid = fork();
        if(pid == 0) {
            execvp(args[0], args);  
            perror("Command failed");
            return 1;
        } else {
            wait(NULL);  // Parent waits for the child to finish
        }
    }

    return 0;
}
