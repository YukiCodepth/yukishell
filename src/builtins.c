#include "../include/yukishell.h"
#include "../include/logo.h"
#include <termios.h>
#include <fcntl.h>

int execute_builtin(char **args) {
    if (args[0] == NULL) return 1; 

    // --- System Exit ---
    if(strcmp(args[0], "exit") == 0) {
        printf("\x1b[33m[!] Shutting down YukiShell systems...\x1b[0m\n");
        exit(0); 
    }

    // --- System Neofetch ---
    if(strcmp(args[0], "neofetch") == 0) {
        print_boot_screen();
        return 1;
    }

    // --- The Multi-Model AI Router & Agents ---
    if(strcmp(args[0], "ask") == 0) {
        if (args[1] == NULL) {
            printf("\x1b[31mUsage: ask [--gemini|--search|--exec|--auto] \"your question\"\x1b[0m\n");
            return 1;
        }

        char model_flag[32] = "gemini"; 
        int prompt_idx = 1;

        if (strncmp(args[1], "--", 2) == 0) {
            strncpy(model_flag, args[1] + 2, sizeof(model_flag) - 1); 
            prompt_idx = 2; 
            if (args[2] == NULL) {
                printf("\x1b[31mError: Please provide a prompt after the flag.\x1b[0m\n");
                return 1;
            }
        }

        char py_cmd[2048] = "./venv/bin/python yuki_ai.py ";
        strncat(py_cmd, model_flag, sizeof(py_cmd) - strlen(py_cmd) - 1);
        strncat(py_cmd, " \"", sizeof(py_cmd) - strlen(py_cmd) - 1);

        for (int i = prompt_idx; args[i] != NULL; i++) {
            strncat(py_cmd, args[i], sizeof(py_cmd) - strlen(py_cmd) - 1);
            if (args[i+1] != NULL) {
                strncat(py_cmd, " ", sizeof(py_cmd) - strlen(py_cmd) - 1);
            }
        }
        strncat(py_cmd, "\"", sizeof(py_cmd) - strlen(py_cmd) - 1);

        if (strcmp(model_flag, "auto") == 0) {
            printf("\n\x1b[1m\x1b[31m[ ⚠️ SYSTEM OVERRIDE: GIVING AI RAW TERMINAL ACCESS ]\x1b[0m\n");
            printf("\x1b[33mPress Ctrl+C at ANY time to instantly revoke access and kill the agent.\x1b[0m\n\n");
            system(py_cmd);
            return 1;
        }

        if (strcmp(model_flag, "exec") == 0) {
            printf("\x1b[35m[ Yuki AI ]\x1b[0m Generating command...\n");
            FILE *fp = popen(py_cmd, "r");
            if (fp == NULL) { printf("Failed to run AI engine.\n"); return 1; }

            char ai_command[1024] = {0};
            fgets(ai_command, sizeof(ai_command) - 1, fp);
            pclose(fp);

            ai_command[strcspn(ai_command, "\n")] = 0; 

            printf("\n\x1b[33mProposed Command:\x1b[0m \x1b[1m%s\x1b[0m\n", ai_command);
            printf("\x1b[32mExecute this command? [Y/n]: \x1b[0m");
            
            char confirm = getchar();
            int c; 
            while ((c = getchar()) != '\n' && c != EOF); 

            if (confirm == 'Y' || confirm == 'y' || confirm == '\n') {
                printf("\n"); system(ai_command); 
            } else {
                printf("\x1b[31mAborted.\x1b[0m\n");
            }
            return 1;
        }

        system(py_cmd);
        return 1;
    }

    // --- The Universal Hardware Serial Monitor ---
    if(strcmp(args[0], "serial") == 0) {
        if (args[1] == NULL) {
            printf("\x1b[31mUsage: serial <device_path> [baud_rate]\x1b[0m\n");
            return 1;
        }

        char *portname = args[1];
        int target_baud = 115200; 
        if (args[2] != NULL) target_baud = atoi(args[2]);

        int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
            perror("\x1b[31m[Error] Hardware port not found\x1b[0m");
            return 1;
        }

        speed_t speed;
        switch(target_baud) {
            case 9600: speed = B9600; break;
            case 115200: speed = B115200; break;
            default: speed = B115200;
        }

        struct termios tty;
        if (tcgetattr(fd, &tty) != 0) { close(fd); return 1; }
        cfsetospeed(&tty, speed); cfsetispeed(&tty, speed);
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_iflag &= ~IGNBRK; tty.c_lflag = 0; tty.c_oflag = 0;        
        tty.c_cc[VMIN] = 1; tty.c_cc[VTIME] = 1;    
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); 
        tty.c_cflag |= (CLOCAL | CREAD); 
        tcsetattr(fd, TCSANOW, &tty);

        printf("\n\x1b[36m\x1b[1m⚡ YUKI HARDWARE MONITOR\x1b[0m\n");
        printf("Connected to \x1b[32m%s\x1b[0m at \x1b[32m%d\x1b[0m baud\n", portname, target_baud);
        printf("\x1b[33m[Press Ctrl+C to disconnect]\x1b[0m\n\n");

        pid_t pid = fork();
        if (pid == 0) {
            char buf[256]; int n;
            while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
                buf[n] = '\0'; printf("%s", buf); fflush(stdout);
            }
            _exit(0); 
        } else {
            waitpid(pid, NULL, 0); close(fd);
            printf("\n\x1b[31m[!] Connection Terminated\x1b[0m\n");
        }
        return 1;
    }

    // --- The Native Network Scanner ---
    if(strcmp(args[0], "netscan") == 0) {
        printf("\n\x1b[36m\x1b[1m🌐 YUKI NETWORK SCANNER\x1b[0m\n");
        FILE *fp = fopen("/proc/net/arp", "r");
        if (fp == NULL) { return 1; }
        char line[256], ip[32], hw_type[32], flags[32], mac[32], mask[32], dev[32];
        fgets(line, sizeof(line), fp); 
        printf("\x1b[32m%-20s %-20s %-10s\x1b[0m\n", "IP ADDRESS", "MAC ADDRESS", "INTERFACE");
        printf("------------------------------------------------------\n");
        int count = 0;
        while (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%s %s %s %s %s %s", ip, hw_type, flags, mac, mask, dev);
            if (strcmp(mac, "00:00:00:00:00:00") != 0) {
                printf("%-20s %-20s %-10s\n", ip, mac, dev); count++;
            }
        }
        fclose(fp);
        printf("\n\x1b[32m[+] Devices Detected: %d\x1b[0m\n\n", count);
        return 1;
    }

    // --- [V12] The Updated Help Menu ---
    if(strcmp(args[0], "help") == 0) {
        printf("\n");
        printf("\x1b[36m\x1b[1m┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\x1b[0m\n");
        printf("\x1b[36m\x1b[1m┃                   YUKI-SHELL V12.0 CORE                    ┃\x1b[0m\n");
        printf("\x1b[36m\x1b[1m┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\x1b[0m\n");

        printf("\n\x1b[1m  \x1b[32m[SYSTEM & HISTORY]\x1b[0m\n");
        printf("   \x1b[1mhelp\x1b[0m        Open this high-performance UI menu\n");
        printf("   \x1b[1mneofetch\x1b[0m    Display OS logo and live hardware specs\n");
        printf("   \x1b[1mclear/cls\x1b[0m   Wipe the terminal screen cleanly\n");
        printf("   \x1b[1mHistory\x1b[0m     Use \x1b[33m[Up/Down Arrows]\x1b[0m to navigate ~/.yuki_history\n");
        printf("   \x1b[1mexit\x1b[0m        Kill the shell process safely\n");

        printf("\n\x1b[1m  \x1b[32m[CUSTOM ALIASES]\x1b[0m\n");
        printf("   \x1b[1mll\x1b[0m          Alias for 'ls -la' (Detailed list)\n");
        printf("   \x1b[1mupdate\x1b[0m      Alias for 'sudo dnf update -y'\n");
        printf("   \x1b[1mports\x1b[0m       Alias for 'netscan'\n");

        printf("\n\x1b[1m  \x1b[32m[IOT & HARDWARE]\x1b[0m\n");
        printf("   \x1b[1mnetscan\x1b[0m     Live mapping of local network devices\n");
        printf("   \x1b[1mserial\x1b[0m      Universal monitor (STM32, ESP32, Arduino)\n");

        printf("\n\x1b[1m  \x1b[32m[ENGINEERING & AI AGENTS]\x1b[0m\n");
        printf("   \x1b[1mask\x1b[0m         Query the Multi-Model AI Engine\n");
        printf("               \x1b[2mUsage: ask [flag] \"your prompt\"\x1b[0m\n");
        printf("               \x1b[33m--gemini, --search, --exec\x1b[0m\n");
        printf("               \x1b[31m--auto\x1b[0m   (God Mode: Autonomous Terminal Agent)\n\n");
        
        printf("   \x1b[1mChaining\x1b[0m    Chain operators together (e.g., \x1b[33mls | grep .c > out\x1b[0m)\n");
        printf("   \x1b[1mScripting\x1b[0m   Run \x1b[33m.yuki\x1b[0m files (Automation Engine)\n");

        printf("\n\x1b[36m\x1b[1m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\x1b[0m\n");
        printf("\x1b[2m Developed by Aman Kumar | ECE Core | SRM Kattankulathur\x1b[0m\n\n");
        return 1;
    }

    if(strcmp(args[0], "cd") == 0) {
        if(args[1] != NULL && chdir(args[1]) != 0) perror("cd failed");
        return 1;
    }
    return 0; 
}
