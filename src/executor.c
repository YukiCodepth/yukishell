#include "../include/yukishell.h"

int execute_external(char **args, int background) {
    pid_t pid = fork();
    if(pid == 0) {
        execvp(args[0], args);  
        perror("Command failed");
        exit(1); 
    } else if (pid < 0) {
        perror("Fork failed");
    } else {
        if (background == 0) {
            waitpid(pid, NULL, 0); 
        } else {
            printf("[Background Process Started] PID: %d\n", pid);
        }
    }
    return 1;
}

int execute_piped(char **args, char **command2) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Pipe failed");
        return 0;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipefd[0]);  
        dup2(pipefd[1], STDOUT_FILENO);  
        close(pipefd[1]);  
        
        execvp(args[0], args);  
        perror("Command 1 failed");
        exit(1);
    } 
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipefd[1]);  
        dup2(pipefd[0], STDIN_FILENO);  
        close(pipefd[0]);  
        
        execvp(command2[0], command2);  
        perror("Command 2 failed");
        exit(1);
    }
    
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);  
    waitpid(pid2, NULL, 0);  
    
    return 1;
}

int execute_redirected(char **args, char *filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
    if (fd == -1) {
        perror("Failed to open file");
        return 0;
    }

    pid_t pid = fork();
    if(pid == 0) {
        dup2(fd, STDOUT_FILENO);  
        close(fd);                
        
        execvp(args[0], args);  
        perror("Command failed");
        exit(1);
    } else if (pid < 0) {
        perror("Fork failed");
    } else {
        wait(NULL);
        close(fd);  
    }
    return 1;
}
