#include <stdio.h>
#include <string.h>

int main() {

    char command[100];

    while(1) {

        printf("YukiShell > ");

        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;

        if(strcmp(command, "hello") == 0) {
            printf("Hello Yuki!\n");
        }

        else if(strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("hello\n");
            printf("help\n");
            printf("exit\n");
        }

        else if(strcmp(command, "exit") == 0) {
            printf("Closing YukiShell...\n");
            break;
        }

        else {
            printf("Command not found\n");
        }
    }

    return 0;
}
