#define _GNU_SOURCE
#include <sched.h>
#include <dirent.h>
#include <sys/stat.h>


#include <sys/sysinfo.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int kbhit_boot() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

void live_fastfetch_boot() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("\033[2J\033[?25l"); // Clear screen, hide cursor
    
    while(!kbhit_boot()) {
        struct sysinfo info;
        sysinfo(&info);
        long total_ram = (info.totalram * info.mem_unit) / (1024 * 1024);
        long free_ram = (info.freeram * info.mem_unit) / (1024 * 1024);
        long used_ram = total_ram - free_ram;
        long h = info.uptime / 3600;
        long m = (info.uptime % 3600) / 60;
        long s = info.uptime % 60;

        printf("\033[H\n\n"); // Move to top left, add padding
        
        // --- FASTFETCH LAYOUT ---
        printf("\033[38;2;180;190;254m"); // Lavender
        printf("                       \033[1;37myuki\033[0m@\033[1;37maegis-edge\033[0m\n");
        printf("      ▰▰▰▰▰▰▰          \033[38;2;180;190;254m-------------------\033[0m\n");
        printf("    ▰▰▰▰▰▰▰▰▰▰▰        \033[1;36mOS\033[0m      Aegis-Edge Core v18.0\n");
        printf("   ▰▰▰       ▰▰▰       \033[1;36mKERNEL\033[0m  Yuki Dynamic\n");
        printf("   ▰▰▰       ▰▰▰       \033[1;36mUPTIME\033[0m  %02ldh %02ldm %02lds\n", h, m, s);
        
        printf("    ▰▰▰▰▰▰▰▰▰▰▰        \033[1;36mMEM\033[0m     [");
        int filled = 0;
        if (total_ram > 0) filled = (used_ram * 15) / total_ram;
        for(int i=0; i<15; i++) { 
            if(i < filled) printf("\033[38;2;166;227;161m■\033[0m"); // Green blocks
            else printf("\033[90m-\033[0m"); // Dim dashes
        }
        printf("] %ldMB / %ldMB\n", used_ram, total_ram);
        
        printf("      ▰▰▰▰▰▰▰          \033[1;36mSHELL\033[0m   YukiShell (C-Native)\n");
        printf("\033[0m\n");
        printf("   \033[5m\033[90m[ Press ANY KEY to initialize command prompt ]\033[0m\n");

        fflush(stdout);
        usleep(500000); // Wait 500ms before drawing the next frame
    }

    getchar(); // Consume the keypress
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\033[?25h\033[2J\033[H"); // Show cursor, wipe screen, start shell
}

#include "../include/yukishell.h"
#include "../include/logo.h"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int execute_builtin(char **args) {
    if (strcmp(args[0], "net") == 0) {
        if (args[1] != NULL && strcmp(args[1], "scan") == 0) {
            system("python3 yuki_net.py");
        } else {
            printf("\033[33mUsage: net scan\033[0m\n");
        }
        return 1;
    }

    if (strcmp(args[0], "i2c") == 0) {
        if (args[1] != NULL && strcmp(args[1], "scan") == 0) {
            int status = system("./i2c_scan");
            if (status == -1) {
                printf("\033[31m[!] YukiShell Error: i2c_scan binary not found. Did you compile it?\033[0m\n");
            }
        } else {
            printf("\033[33mUsage: i2c scan\033[0m\n");
        }
        return 1;
    }

    if (args[0] == NULL) return 1; 

    if(strcmp(args[0], "exit") == 0) {
        printf("\033[38;2;243;139;168m[!] Shutting down YukiShell systems...\033[0m\n");
        exit(0); 
    }

    if (strcmp(args[0], "neofetch") == 0) {
        live_fastfetch_boot();
        return 1;
    }

    // --- AI Router ---
    if(strcmp(args[0], "ask") == 0) {
        if (args[1] == NULL) {
            printf("\x1b[31mUsage: ask [--gemini|--search|--exec|--auto|--live] \"your prompt\"\x1b[0m\n");
            return 1;
        }

        char model_flag[32] = "gemini"; 
        int prompt_idx = 1;

        if (strncmp(args[1], "--", 2) == 0) {
            strncpy(model_flag, args[1] + 2, sizeof(model_flag) - 1); 
            prompt_idx = 2; 
            
            // V16: Allow --live to run without a text prompt
            if (args[2] == NULL && strcmp(model_flag, "live") != 0) {
                printf("\x1b[31mError: Please provide a prompt after the flag.\x1b[0m\n");
                return 1;
            }
        }

        char py_cmd[2048] = "./venv/bin/python yuki_ai.py ";
        strncat(py_cmd, model_flag, sizeof(py_cmd) - strlen(py_cmd) - 1);
        strncat(py_cmd, " \"", sizeof(py_cmd) - strlen(py_cmd) - 1);

        // Safely append arguments if they exist
        if (args[prompt_idx] != NULL) {
            for (int i = prompt_idx; args[i] != NULL; i++) {
                strncat(py_cmd, args[i], sizeof(py_cmd) - strlen(py_cmd) - 1);
                if (args[i+1] != NULL) {
                    strncat(py_cmd, " ", sizeof(py_cmd) - strlen(py_cmd) - 1);
                }
            }
        } else {
            strncat(py_cmd, "LiveStreamInit", sizeof(py_cmd) - strlen(py_cmd) - 1);
        }
        strncat(py_cmd, "\"", sizeof(py_cmd) - strlen(py_cmd) - 1);

        if (strcmp(model_flag, "auto") == 0) {
            printf("\n\x1b[1m\x1b[31m[ ⚠️ SYSTEM OVERRIDE: GIVING AI RAW TERMINAL ACCESS ]\x1b[0m\n");
            printf("\x1b[33mPress Ctrl+C at ANY time to instantly revoke access and kill the agent.\x1b[0m\n\n");
            system(py_cmd);
            return 1;
        }

        // V16: Route for Yuki Live
        if (strcmp(model_flag, "live") == 0) {
            system(py_cmd);
            return 1;
        }

        if (strcmp(model_flag, "exec") == 0) {
            printf("\033[38;2;203;166;247m[ Yuki AI ]\033[0m Generating command...\n");
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

    // --- Serial Monitor ---
    if(strcmp(args[0], "serial") == 0) {
        if (args[1] == NULL) {
            printf("\x1b[31mUsage: serial <device_path> [baud_rate]\x1b[0m\n");
            return 1;
        }
        char *portname = args[1];
        int target_baud = 115200; 
        if (args[2] != NULL) target_baud = atoi(args[2]);

        int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) { perror("\x1b[31m[Error] Hardware port not found\x1b[0m"); return 1; }

        speed_t speed = (target_baud == 9600) ? B9600 : B115200;
        struct termios tty;
        if (tcgetattr(fd, &tty) != 0) { close(fd); return 1; }
        cfsetospeed(&tty, speed); cfsetispeed(&tty, speed);
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_iflag &= ~IGNBRK; tty.c_lflag = 0; tty.c_oflag = 0;        
        tty.c_cc[VMIN] = 1; tty.c_cc[VTIME] = 1;    
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); 
        tty.c_cflag |= (CLOCAL | CREAD); 
        tcsetattr(fd, TCSANOW, &tty);

        printf("\n\033[38;2;137;180;250m\x1b[1m⚡ YUKI HARDWARE MONITOR\x1b[0m\n");
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
            printf("\n\033[38;2;243;139;168m[!] Connection Terminated\033[0m\n");
        }
        return 1;
    }

    // --- Network Scanner ---
    if(strcmp(args[0], "netscan") == 0) {
        printf("\n\033[38;2;166;227;161m\x1b[1m🌐 YUKI NETWORK SCANNER\x1b[0m\n");
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

    // --- V15.0: Native Hardware Dashboard ---
    if(strcmp(args[0], "dash") == 0) {
        pid_t pid = fork();
        if (pid == 0) {
            while(1) {
                printf("\033[H\033[J"); 

                printf("\033[38;2;180;190;254m\x1b[1m╭━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╮\x1b[0m\n");
                printf("\033[38;2;180;190;254m\x1b[1m┃\x1b[0m                 \033[38;2;245;194;231mYUKI LIVE TELEMETRY\033[0m                        \033[38;2;180;190;254m\x1b[1m┃\x1b[0m\n");
                printf("\033[38;2;180;190;254m\x1b[1m┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\x1b[0m\n");

                FILE *load_fp = fopen("/proc/loadavg", "r");
                if (load_fp) {
                    double load1, load5, load15;
                    fscanf(load_fp, "%lf %lf %lf", &load1, &load5, &load15);
                    printf("\033[38;2;180;190;254m\x1b[1m┃\x1b[0m \033[38;2;137;180;250mCPU Load (1m, 5m, 15m):\033[0m %.2f, %.2f, %.2f\n", load1, load5, load15);
                    fclose(load_fp);
                }

                FILE *mem_fp = fopen("/proc/meminfo", "r");
                if (mem_fp) {
                    char label[32];
                    long total_mem = 0, free_mem = 0;
                    while (fscanf(mem_fp, "%31s %ld kB", label, &total_mem) != EOF) {
                        if (strcmp(label, "MemTotal:") == 0) break;
                    }
                    rewind(mem_fp);
                    while (fscanf(mem_fp, "%31s %ld kB", label, &free_mem) != EOF) {
                        if (strcmp(label, "MemAvailable:") == 0) break;
                    }
                    fclose(mem_fp);

                    long used_mem = total_mem - free_mem;
                    double used_gb = (double)used_mem / (1024.0 * 1024.0);
                    double total_gb = (double)total_mem / (1024.0 * 1024.0);
                    int mem_percent = (total_mem > 0) ? (int)((used_mem * 100.0) / total_mem) : 0;

                    printf("\033[38;2;180;190;254m\x1b[1m┃\x1b[0m \033[38;2;166;227;161mMemory Usage:\033[0m %.2f GB / %.2f GB [%d%%]\n", used_gb, total_gb, mem_percent);
                    
                    printf("\033[38;2;180;190;254m\x1b[1m┃\x1b[0m \033[38;2;243;139;168mRAM:\033[0m [");
                    int bar_width = 40;
                    int filled = (mem_percent * bar_width) / 100;
                    for (int i = 0; i < bar_width; i++) {
                        if (i < filled) printf("\033[38;2;166;227;161m■\033[0m");
                        else printf("\033[38;2;88;91;112m-\033[0m");
                    }
                    printf("]\n");
                }

                printf("\033[38;2;180;190;254m\x1b[1m╰━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╯\x1b[0m\n");
                printf("\n\033[38;2;88;91;112mPress [Ctrl+C] to return to shell\033[0m\n");
                
                fflush(stdout);
                sleep(1); 
            }
            _exit(0); 
        } else {
            waitpid(pid, NULL, 0); 
            printf("\n"); 
        }
        return 1;
    }

    // --- Help Menu ---
    if(strcmp(args[0], "help") == 0) {
        printf("\n");
        printf("\033[38;2;137;180;250m\x1b[1m┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\x1b[0m\n");
        printf("\033[38;2;137;180;250m\x1b[1m┃                   YUKI-SHELL V16.0 CORE                    ┃\x1b[0m\n");
        printf("\033[38;2;137;180;250m\x1b[1m┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\x1b[0m\n");

        printf("\n\x1b[1m  \033[38;2;166;227;161m[SYSTEM & CONFIGURATION]\x1b[0m\n");
        printf("   \x1b[1mhelp\x1b[0m        Open this high-performance UI menu\n");
        printf("   \x1b[1mneofetch\x1b[0m    Display OS logo and live hardware specs\n");
        printf("   \x1b[1mdash\x1b[0m        Launch the real-time hardware telemetry dashboard\n");
        printf("   \x1b[1mHistory\x1b[0m     Use \x1b[33m[Up/Down Arrows]\x1b[0m to navigate ~/.yuki_history\n");
        printf("   \x1b[1mAliases\x1b[0m     Dynamically loaded from \x1b[33m~/.yukirc\x1b[0m\n");

        printf("\n\x1b[1m  \033[38;2;166;227;161m[IOT & HARDWARE]\x1b[0m\n");
        printf("   \x1b[1mnetscan\x1b[0m     Live mapping of local network devices\n");
        printf("   \x1b[1mserial\x1b[0m      Universal monitor (STM32, ESP32, Arduino)\n");

        printf("\n\x1b[1m  \033[38;2;166;227;161m[ENGINEERING & AI AGENTS]\x1b[0m\n");
        printf("   \x1b[1mask\x1b[0m         Query the Multi-Model AI Engine\n");
        printf("               \x1b[2mUsage: ask [flag] \"your prompt\"\x1b[0m\n");
        printf("               \x1b[33m--gemini, --search, --exec\x1b[0m\n");
        printf("               \033[38;2;137;180;250m--auto\x1b[0m   (Autonomous Agent + Viewfinder)\n");
        printf("               \033[38;2;243;139;168m--live\x1b[0m   (Real-Time Continuous Video Stream)\n\n");
        
        printf("   \x1b[1mChaining\x1b[0m    Chain operators together (e.g., \x1b[33mls | grep .c > out\x1b[0m)\n");

        printf("\n\033[38;2;137;180;250m\x1b[1m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\x1b[0m\n");
        printf("\x1b[2m Developed by Aman Kumar | ECE Core | SRM Kattankulathur\x1b[0m\n\n");
        return 1;
    }


    // --- V21.0: NATIVE PROCESS MANAGER ---
    if(strcmp(args[0], "taskmgr") == 0) {
        pid_t pid = fork();
        if (pid == 0) {
            printf("\033[2J\033[?25l"); // Clear screen & hide cursor
            system("stty raw -echo");   // Set terminal to raw non-blocking mode
            int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

            while(1) {
                printf("\033[H"); // Home cursor (Prevents flickering)
                printf("\r\033[38;2;180;190;254m\x1b[1m╭━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╮\x1b[0m\n");
                printf("\r\033[38;2;180;190;254m\x1b[1m┃\x1b[0m                 \033[38;2;245;194;231mYUKI TASK MANAGER\033[0m                          \033[38;2;180;190;254m\x1b[1m┃\x1b[0m\n");
                printf("\r\033[38;2;180;190;254m\x1b[1m┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\x1b[0m\n");

                FILE *fp = popen("ps -eo pid,user,%mem,comm --sort=-%mem | head -n 12 | tail -n +2", "r");
                if (fp) {
                    char line[256];
                    printf("\r\033[38;2;180;190;254m\x1b[1m┃\x1b[0m \033[38;2;137;180;250m%-8s %-12s %-8s %-24s\033[0m \033[38;2;180;190;254m\x1b[1m┃\x1b[0m\n", "PID", "USER", "MEM%", "COMMAND");
                    while(fgets(line, sizeof(line), fp)) {
                        line[strcspn(line, "\n")] = 0;
                        printf("\r\033[38;2;180;190;254m\x1b[1m┃\x1b[0m \033[38;2;166;227;161m%-55s\033[0m \033[38;2;180;190;254m\x1b[1m┃\x1b[0m\n", line);
                    }
                    pclose(fp);
                }
                printf("\r\033[38;2;180;190;254m\x1b[1m╰━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╯\x1b[0m\n");
                printf("\r\n\033[38;2;243;139;168m [q] Quit \033[0m | \033[38;2;249;226;175m [k] Kill Process \033[0m\033[K\n");

                char c;
                if (read(STDIN_FILENO, &c, 1) > 0) {
                    if (c == 'q' || c == 'Q') break;
                    if (c == 'k' || c == 'K') {
                        system("stty cooked echo"); // Restore normal input
                        fcntl(STDIN_FILENO, F_SETFL, flags);
                        printf("\r\n\033[33m ❯ Enter PID to terminate: \033[0m");
                        char pid_str[16];
                        if (fgets(pid_str, 16, stdin)) {
                            int target = atoi(pid_str);
                            if(target > 0) {
                                kill(target, SIGKILL);
                                printf("\r\033[31m [!] SIGKILL sent to %d\033[0m\n", target);
                                usleep(800000);
                            }
                        }
                        // Reset back to raw mode for loop
                        system("stty raw -echo");
                        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
                        printf("\033[2J"); 
                    }
                }
                usleep(500000); // 500ms real-time refresh
            }
            system("stty cooked echo");
            fcntl(STDIN_FILENO, F_SETFL, flags);
            printf("\033[2J\033[H\033[?25h"); // Restore terminal state
            _exit(0);
        } else {
            waitpid(pid, NULL, 0); 
        }
        return 1;
    }

    // --- V21.0: CRYPTOGRAPHIC VAULT ---
    if(strcmp(args[0], "vault") == 0) {
        if (args[1] == NULL || args[2] == NULL) {
            printf("\033[31mUsage: vault [encrypt|decrypt] <filename>\033[0m\n");
            return 1;
        }
        char *action = args[1];
        char *file = args[2];
        
        if (access(file, F_OK) != 0) {
            printf("\033[31m[-] Target file '%s' does not exist.\033[0m\n", file);
            return 1;
        }

        char *pass = getpass("\033[38;2;203;166;247m[ Vault ] Enter Master Password: \033[0m");
        if (!pass) return 1;

        char cmd[1024];
        if(strcmp(action, "encrypt") == 0) {
            snprintf(cmd, sizeof(cmd), "openssl enc -aes-256-cbc -salt -pbkdf2 -in %s -out %s.vault -pass pass:%s 2>/dev/null", file, file, pass);
            if(system(cmd) == 0) {
                remove(file); // Securely destroy original
                printf("\033[32m[+] File encrypted to %s.vault\033[0m\n", file);
                printf("\033[31m[!] Original unencrypted file shredded.\033[0m\n");
            } else printf("\033[31m[-] Encryption failed.\033[0m\n");
        } 
        else if (strcmp(action, "decrypt") == 0) {
            char out_file[256];
            strcpy(out_file, file);
            char *ext = strstr(out_file, ".vault");
            if(ext) *ext = '\0'; else strcat(out_file, ".dec");

            snprintf(cmd, sizeof(cmd), "openssl enc -d -aes-256-cbc -pbkdf2 -in %s -out %s -pass pass:%s 2>/dev/null", file, out_file, pass);
            if(system(cmd) == 0) {
                printf("\033[32m[+] File securely decrypted to %s\033[0m\n", out_file);
            } else {
                printf("\033[31m[-] Decryption failed. Incorrect password or corrupted data.\033[0m\n");
                remove(out_file);
            }
        } else {
            printf("\033[31mUsage: vault [encrypt|decrypt] <filename>\033[0m\n");
        }
        return 1;
    }


    // --- V22.0: NATIVE HEX VIEWER ---
    if(strcmp(args[0], "hexview") == 0) {
        if(args[1] == NULL) { printf("\033[31mUsage: hexview <file>\033[0m\n"); return 1; }
        FILE *fp = fopen(args[1], "rb");
        if(!fp) { perror("\033[31m[-] Failed to open file\033[0m"); return 1; }
        
        unsigned char buffer[16];
        size_t bytes_read;
        size_t offset = 0;
        
        printf("\n\033[38;2;137;180;250m\x1b[1m[ YUKI HEX SENTINEL ]\033[0m Target: %s\n\n", args[1]);
        
        while((bytes_read = fread(buffer, 1, 16, fp)) > 0) {
            printf("\033[38;2;180;190;254m%08zx\033[0m  ", offset); 
            
            for(size_t i = 0; i < 16; i++) {
                if(i < bytes_read) {
                    if(buffer[i] == 0x00) printf("\033[90m%02x \033[0m", buffer[i]); 
                    else printf("\033[38;2;249;226;175m%02x \033[0m", buffer[i]); 
                } else {
                    printf("   ");
                }
                if(i == 7) printf(" "); 
            }
            
            printf(" |");
            for(size_t i = 0; i < bytes_read; i++) {
                if(buffer[i] >= 32 && buffer[i] <= 126) 
                    printf("\033[38;2;166;227;161m%c\033[0m", buffer[i]); 
                else 
                    printf("\033[90m.\033[0m"); 
            }
            printf("|\n");
            offset += 16;
        }
        fclose(fp);
        printf("\n");
        return 1;
    }

    // --- V22.0: HARDWARE WATCHDOG ---
    if(strcmp(args[0], "lsdev") == 0) {
        DIR *d = opendir("/sys/bus/usb/devices");
        if (d) {
            struct dirent *dir;
            printf("\n\033[38;2;166;227;161m\x1b[1m[ YUKI HARDWARE WATCHDOG ]\033[0m Scanning System Buses...\n\n");
            printf("\033[1;36m%-15s %-12s %-40s\033[0m\n", "SYS-PORT", "VID:PID", "MANUFACTURER / PRODUCT");
            printf("------------------------------------------------------------------------\n");
            
            while ((dir = readdir(d)) != NULL) {
                if(dir->d_name[0] == '.' || strchr(dir->d_name, ':')) continue; 
                
                char path[512], vendor[64] = "Unknown", product[128] = "Unknown Device";
                char vid[16] = "", pid[16] = "";
                FILE *f;

                snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", dir->d_name);
                if((f = fopen(path, "r"))) { fgets(vid, 16, f); vid[strcspn(vid, "\n")] = 0; fclose(f); }

                snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", dir->d_name);
                if((f = fopen(path, "r"))) { fgets(pid, 16, f); pid[strcspn(pid, "\n")] = 0; fclose(f); }

                snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/manufacturer", dir->d_name);
                if((f = fopen(path, "r"))) { fgets(vendor, 64, f); vendor[strcspn(vendor, "\n")] = 0; fclose(f); }

                snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/product", dir->d_name);
                if((f = fopen(path, "r"))) { fgets(product, 128, f); product[strcspn(product, "\n")] = 0; fclose(f); }

                if(strlen(vid) > 0) {
                    printf("\033[38;2;180;190;254m%-15s\033[0m \033[38;2;245;194;231m%s:%s\033[0m  \033[32m%s %s\033[0m\n", 
                           dir->d_name, vid, pid, vendor, product);
                }
            }
            closedir(d);
            printf("\n");
        } else {
            printf("\033[31m[-] Sysfs not accessible. Cannot map hardware.\033[0m\n");
        }
        return 1;
    }


    // --- V23.0: GHOST MODE (Ephemeral Sandboxing) ---
    if(strcmp(args[0], "ghostmode") == 0) {
        printf("\n\033[38;2;203;166;247m  ✧ [ GHOST VORTEX ] Initiating Ephemeral Namespace...\033[0m\n");
        printf("\033[90mRipping namespace parameters...\033[0m\n");
        
        // Attempt to unshare the mount and UTS namespaces
        if(unshare(CLONE_NEWNS | CLONE_NEWUTS) != 0) {
            printf("\033[31m[-] Kernel rejected unshare.\033[0m\n");
            printf("\033[33m[!] Ghost Mode requires elevated capabilities. Try: sudo ./yukishell\033[0m\n\n");
            return 1;
        }
        
        // Mount a completely ephemeral RAM disk over /tmp
        system("mount -t tmpfs -o size=512m tmpfs /tmp");
        chdir("/tmp");
        
        printf("\033[32m[+] Reality isolated.\033[0m You are now in an ephemeral container.\n");
        printf("\033[32m[+] All files written to /tmp exist only in RAM and will vanish on exit.\033[0m\n\n");
        
        // Spawn a nested, isolated bash shell as the "Ghost"
        system("env PROMPT_COMMAND=\'PS1=\"\\[\\e[38;2;203;166;247m\\]╭─ ✧ [ GHOST VORTEX ] ✧ \\w\\n\\[\\e[38;2;203;166;247m\\]╰─❯ \\[\\e[0m\\] \"\' bash --norc");
        
        // Clean up the timeline when they type exit
        system("umount -l /tmp");
        printf("\n\033[1;35m[ 👻 GHOST MODE TERMINATED ]\033[0m Timeline restored. Traces erased.\n\n");
        return 1;
    }

    // --- V23.0: YUKI-QL (Object-Oriented Pipes) ---
    if(strcmp(args[0], "yql") == 0) {
        if(args[1] == NULL || args[2] == NULL || args[3] == NULL) {
            printf("\033[31mUsage: <cmd> | yql <COLUMN_NUMBER> <OPERATOR> <VALUE>\033[0m\n");
            printf("\033[90mExample: ps aux | yql 4 > 5.0  (Show processes using > 5%% RAM)\033[0m\n");
            printf("\033[90mExample: ls -l  | yql 5 > 1000 (Show files larger than 1000 bytes)\033[0m\n\n");
            return 1;
        }
        
        int target_col = atoi(args[1]);
        char *op = args[2];
        double threshold = atof(args[3]);
        
        char line[1024];
        int is_header = 1;
        
        // Intercept standard input from the pipe
        while(fgets(line, sizeof(line), stdin)) {
            // Always print the header row for context
            if(is_header) {
                printf("\033[1;36m%s\033[0m", line);
                is_header = 0;
                continue;
            }
            
            // Duplicate the line because strtok destroys the original string
            char line_copy[1024];
            strcpy(line_copy, line);
            
            char *token = strtok(line_copy, " \t");
            int current_col = 1;
            double cell_val = 0.0;
            int match = 0;
            
            // Walk the columns to find the target struct
            while(token != NULL) {
                if(current_col == target_col) {
                    cell_val = atof(token);
                    if(strcmp(op, ">") == 0 && cell_val > threshold) match = 1;
                    if(strcmp(op, "<") == 0 && cell_val < threshold) match = 1;
                    if(strcmp(op, "==") == 0 && cell_val == threshold) match = 1;
                    break;
                }
                token = strtok(NULL, " \t");
                current_col++;
            }
            
            // If the math checks out, print the row in green
            if(match) {
                printf("\033[32m%s\033[0m", line);
            }
        }
        return 1;
    }


    // --- V24.0: YUKI READ (xcat) ---
    if(strcmp(args[0], "xcat") == 0) {
        if(args[1] == NULL) { printf("\033[31mUsage: xcat <file>\033[0m\n"); return 1; }
        FILE *fp = fopen(args[1], "r");
        if(!fp) { perror("\033[31m[-] File access denied\033[0m"); return 1; }
        char line[1024]; int count = 1;
        printf("\n\033[38;2;137;180;250m╭── [ READING: %s ]\033[0m\n", args[1]);
        while(fgets(line, sizeof(line), fp)) printf("\033[90m│ %4d │\033[0m \033[38;2;205;214;244m%s\033[0m", count++, line);
        printf("\033[38;2;137;180;250m╰────────────────────────\033[0m\n\n");
        fclose(fp); return 1;
    }

    // --- V24.0: YUKI LIST (xls) ---
    if(strcmp(args[0], "xls") == 0) {
        DIR *d; struct dirent *dir; struct stat file_stat;
        char *target_dir = args[1] ? args[1] : ".";
        d = opendir(target_dir);
        if(d) {
            printf("\n\033[1;36m%-10s %-15s %s\033[0m\n", "SIZE", "TYPE", "NAME");
            printf("\033[90m──────────────────────────────────────────────\033[0m\n");
            while((dir = readdir(d)) != NULL) {
                if(dir->d_name[0] == '.') continue; 
                char path[1024]; snprintf(path, sizeof(path), "%s/%s", target_dir, dir->d_name);
                stat(path, &file_stat);
                double size = file_stat.st_size; char* unit = "B";
                if(size > 1024) { size /= 1024; unit = "KB"; }
                if(size > 1024) { size /= 1024; unit = "MB"; }
                
                if(S_ISDIR(file_stat.st_mode)) printf("\033[38;2;180;190;254m%-7.1f %-2s\033[0m \033[38;2;245;194;231m%-15s\033[0m \033[1;34m%s/\033[0m\n", size, unit, "<DIR>", dir->d_name);
                else if(file_stat.st_mode & S_IXUSR) printf("\033[38;2;166;227;161m%-7.1f %-2s\033[0m \033[38;2;137;180;250m%-15s\033[0m \033[1;32m%s*\033[0m\n", size, unit, "<EXEC>", dir->d_name);
                else printf("\033[38;2;205;214;244m%-7.1f %-2s\033[0m \033[90m%-15s\033[0m %s\n", size, unit, "FILE", dir->d_name);
            }
            closedir(d); printf("\n");
        } else printf("\033[31m[-] Cannot open directory\033[0m\n");
        return 1;
    }

    // --- V24.0: YUKI NET-WATCH (xping) ---
    if(strcmp(args[0], "xping") == 0) {
        if(args[1] == NULL) { printf("\033[31mUsage: xping <host>\033[0m\n"); return 1; }
        printf("\n\033[38;2;166;227;161m  [ YUKI SONAR ] Pinging %s \033[0m\n\n", args[1]);
        char cmd[256]; snprintf(cmd, sizeof(cmd), "ping -c 1 -W 1 %s | grep time=", args[1]);
        for(int i=0; i<15; i++) { 
            FILE *fp = popen(cmd, "r"); char line[128];
            if(fp && fgets(line, sizeof(line), fp)) {
                char *time_ptr = strstr(line, "time=");
                if(time_ptr) {
                    double ms = atof(time_ptr + 5);
                    printf("\033[38;2;137;180;250m%-8.2f ms\033[0m │ ", ms);
                    int bars = (int)(ms / 5.0); if(bars > 40) bars = 40;
                    for(int b=0; b<bars; b++) printf("\033[38;2;166;227;161m█\033[0m");
                    printf("\n");
                }
            } else printf("\033[31mTIMEOUT\033[0m    │ \033[31mX\033[0m\n");
            if(fp) { pclose(fp); }
            usleep(500000); 
        }
        printf("\n"); return 1;
    }


    // --- V26.0: SHADOW MONITOR (jobs) ---
    if(strcmp(args[0], "jobs") == 0) {
        printf("\n\033[38;2;203;166;247m  ✧ [ YUKI SHADOW REALM ]\033[0m\n");
        printf("\033[90m------------------------------------------------\033[0m\n");
        // Grep the process tree for children of this shell
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "ps -o pid,stat,comm --ppid %d | tail -n +2", getpid());
        FILE *fp = popen(cmd, "r");
        if (fp) {
            char line[256];
            int count = 0;
            while(fgets(line, sizeof(line), fp)) {
                printf("\033[38;2;166;227;161m  [⚙] \033[0m%s", line);
                count++;
            }
            if (count == 0) printf("  \033[90mNo active shadow processes.\033[0m\n");
            pclose(fp);
        }
        printf("\n");
        return 1;
    }

    if(strcmp(args[0], "cd") == 0) {
        if(args[1] != NULL && chdir(args[1]) != 0) perror("cd failed");
        return 1;
    }
    return 0; 
}
