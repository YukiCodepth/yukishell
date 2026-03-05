#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    char command[100];

    while(1) {

        printf("YukiShell > ");

        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;

        if(strcmp(command, "exit") == 0) {
            printf("Closing YukiShell...\n");
            break;
        }

        pid_t pid = fork();

        if(pid == 0) {

            char *args[] = {command, NULL};

            execvp(args[0], args);

            printf("Command failed\n");

        }

        else {

            wait(NULL);

        }

    }

    return 0;
}
