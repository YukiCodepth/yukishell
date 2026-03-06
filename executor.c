#include "yukishell.h"

// 1. Normal Command Execution
int execute_external(char **args) {
    pid_t pid = fork();
    if(pid == 0) {
        execvp(args[0], args);  
        perror("Command failed");
        exit(1); // Child must exit if exec fails
    } else if (pid < 0) {
        perror("Fork failed");
    } else {
        wait(NULL);  // Parent waits for the child to finish
    }
    return 1;
}

// 2. Execution with Pipes (|)
int execute_piped(char **args, char **command2) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Pipe failed");
        return 0;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // First child process (before the pipe)
        close(pipefd[0]);  // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe
        close(pipefd[1]);  // Close write end after duping
        
        execvp(args[0], args);  
        perror("Command 1 failed");
        exit(1);
    } 
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Second child process (after the pipe)
        close(pipefd[1]);  // Close unused write end
        dup2(pipefd[0], STDIN_FILENO);  // Redirect stdin from pipe
        close(pipefd[0]);  // Close read end after duping
        
        execvp(command2[0], command2);  
        perror("Command 2 failed");
        exit(1);
    }
    
    // Parent process
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);  // Wait for child 1
    waitpid(pid2, NULL, 0);  // Wait for child 2
    
    return 1;
}

// 3. Execution with Redirection (>)
int execute_redirected(char **args, char *filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
    if (fd == -1) {
        perror("Failed to open file");
        return 0;
    }

    pid_t pid = fork();
    if(pid == 0) {
        dup2(fd, STDOUT_FILENO);  // Redirect stdout to file
        close(fd);                // Close original fd
        
        execvp(args[0], args);  
        perror("Command failed");
        exit(1);
    } else if (pid < 0) {
        perror("Fork failed");
    } else {
        wait(NULL);
        close(fd);  // Parent needs to close the file descriptor too
    }
    return 1;
}