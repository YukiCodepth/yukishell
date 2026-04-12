#ifndef LOGO_H
#define LOGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

static inline void print_boot_screen() {
    struct utsname sys_info;
    uname(&sys_info); 

    // Extract CPU Model from Linux Kernel
    char cpu_model[128] = "Unknown CPU";
    FILE *cpu_file = fopen("/proc/cpuinfo", "r");
    if (cpu_file) {
        char line[256];
        while (fgets(line, sizeof(line), cpu_file)) {
            if (strncmp(line, "model name", 10) == 0) {
                char *colon = strchr(line, ':');
                if (colon) {
                    strncpy(cpu_model, colon + 2, sizeof(cpu_model) - 1);
                    cpu_model[strcspn(cpu_model, "\n")] = 0; 
                    break;
                }
            }
        }
        fclose(cpu_file);
    }

    // Extract Memory Data (RAM)
    long total_mem = 0, free_mem = 0;
    FILE *mem_file = fopen("/proc/meminfo", "r");
    if (mem_file) {
        char line[256];
        while (fgets(line, sizeof(line), mem_file)) {
            if (sscanf(line, "MemTotal: %ld kB", &total_mem) == 1) continue;
            if (sscanf(line, "MemAvailable: %ld kB", &free_mem) == 1) continue;
        }
        fclose(mem_file);
    }
    long used_mem = total_mem - free_mem;

    // Extract System Uptime
    long uptime_sec = 0;
    FILE *uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file) {
        fscanf(uptime_file, "%ld", &uptime_sec);
        fclose(uptime_file);
    }
    int up_hours = uptime_sec / 3600;
    int up_mins = (uptime_sec % 3600) / 60;

    // Get User and Hostname
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    char *user = getenv("USER");
    if (!user) user = "yuki_user";

    printf("\033[2J\033[1;1H"); 
    
    printf("\n\x1b[36m\x1b[1m");
    printf("       __  __   __  __   __  __   __ \n");
    printf("       \\ \\/ /  /\\ \\/\\ \\ /\\ \\/ /  /\\ \\\n");
    printf("        \\  /   \\ \\ \\_\\ \\\\ \\  /   \\ \\ \\\n");
    printf("        / /     \\ \\____/ \\ \\  \\   \\ \\_\\\n");
    printf("       /_/       \\/___/   \\/_/\\_\\  \\/_/   \x1b[32mOS v11.0\n");
    printf("\x1b[0m\n");
    
    printf("  \x1b[36m%s\x1b[0m@\x1b[36m%s\x1b[0m\n", user, hostname);
    printf("  --------------------------------------\n");
    printf("  \x1b[1m\x1b[32mOS:\x1b[0m       %s %s %s\n", sys_info.sysname, sys_info.release, sys_info.machine);
    printf("  \x1b[1m\x1b[32mKernel:\x1b[0m   %s\n", sys_info.version);
    printf("  \x1b[1m\x1b[32mUptime:\x1b[0m   %d hours, %d mins\n", up_hours, up_mins);
    printf("  \x1b[1m\x1b[32mShell:\x1b[0m    YukiShell v11\n");
    printf("  \x1b[1m\x1b[32mCPU:\x1b[0m      %s\n", cpu_model);
    if (total_mem > 0) {
        printf("  \x1b[1m\x1b[32mMemory:\x1b[0m   %ld MB / %ld MB\n", used_mem / 1024, total_mem / 1024);
    }
    printf("\n\x1b[36m  =========================================\x1b[0m\n");
    printf("\x1b[2m   Type 'help' to see available commands.\x1b[0m\n\n");
}

#endif
