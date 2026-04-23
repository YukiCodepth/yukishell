#ifndef LOGO_H
#define LOGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

static inline void fetch_sys(const char *cmd, char *out, size_t max_len) {
    FILE *fp = popen(cmd, "r");
    if (fp) {
        if (fgets(out, max_len, fp) != NULL) {
            out[strcspn(out, "\n")] = 0; 
            char *p = out;
            while(*p == ' ' || *p == '\t') p++;
            if (p != out) memmove(out, p, strlen(p) + 1);
        } else {
            strcpy(out, "Unknown");
        }
        pclose(fp);
    } else {
        strcpy(out, "Unknown");
    }
}

static inline void print_boot_screen() {
    char os[128], host[128], kernel[128], uptime[128], pkgs[128], shell[128];
    char res[128], de[128], wm[128], theme[128], term[128];
    char gpu[128], mem[128];

    fetch_sys("grep '^PRETTY_NAME=' /etc/os-release | cut -d '=' -f 2 | tr -d '\"'", os, 128);
    fetch_sys("cat /sys/devices/virtual/dmi/id/product_name 2>/dev/null || echo 'Unknown Host'", host, 128);

    struct utsname sys_info;
    uname(&sys_info);
    strncpy(kernel, sys_info.release, 128);

    struct sysinfo mem_info;
    sysinfo(&mem_info);
    long up_h = mem_info.uptime / 3600;
    long up_m = (mem_info.uptime % 3600) / 60;
    snprintf(uptime, 128, "%ld hours, %ld mins", up_h, up_m);

    fetch_sys("dpkg-query -f '.\\n' -W 2>/dev/null | wc -l | tr -d ' '", pkgs, 128);
    if (strcmp(pkgs, "0") == 0 || strcmp(pkgs, "Unknown") == 0) {
        fetch_sys("pacman -Qq 2>/dev/null | wc -l | tr -d ' '", pkgs, 128);
    }

    strcpy(shell, "bash 5.1.16 (YukiShell)");

    fetch_sys("xrandr 2>/dev/null | grep '\\*' | awk '{print $1}' | head -n 1", res, 128);
    if(strlen(res) < 3) strcpy(res, "1920x1080 (Headless)");

    char *de_env = getenv("XDG_CURRENT_DESKTOP");
    strncpy(de, de_env ? de_env : "GNOME 44.3", 128);

    char *wm_env = getenv("XDG_SESSION_DESKTOP");
    strncpy(wm, wm_env ? wm_env : "Mutter", 128);

    char *term_env = getenv("TERM");
    strncpy(term, term_env ? term_env : "xterm-256color", 128);

    strcpy(theme, "Adwaita [GTK2/3]");

    fetch_sys("lspci 2>/dev/null | grep -i 'vga\\|3d\\|display' | cut -d ':' -f 3 | head -n 1", gpu, 128);
    if(strlen(gpu) < 3) strcpy(gpu, "Integrated Graphics / Unknown");

    long t_ram = (mem_info.totalram * mem_info.mem_unit) / (1024 * 1024);
    long f_ram = (mem_info.freeram * mem_info.mem_unit) / (1024 * 1024);
    snprintf(mem, 128, "%ldMiB / %ldMiB", (t_ram - f_ram), t_ram);

    char *user = getenv("USER");
    if (!user) user = "yuki";
    char hostname[128];
    gethostname(hostname, 128);

    // Live Telemetry Calcs
    double load = mem_info.loads[0] / 65536.0;
    int mem_percent = (t_ram > 0) ? (int)(((t_ram - f_ram) * 100) / t_ram) : 0;
    uid_t current_uid = geteuid();
    char *mode_str = (current_uid == 0) ? "\033[31m☢ ROOT\033[0m" : "\033[32m✧ USER\033[0m";

    // Build RAM Bar String (Requires 512 buffer to hold all the ANSI colors)
    char ram_bar[512] = "[";
    for(int i=0; i<10; i++) {
        if(i < mem_percent/10) strcat(ram_bar, "\033[38;2;166;227;161m■\033[0m");
        else strcat(ram_bar, "\033[90m■\033[0m");
    }
    char suffix[32];
    snprintf(suffix, 32, "] %d%%", mem_percent);
    strcat(ram_bar, suffix);

    // Flawlessly Aligned 43-Character Wide Logo
    const char *logo[18] = {
        "\033[38;2;116;199;236m                    ..                     \033[0m",
        "\033[38;2;116;199;236m                  .8888.                   \033[0m",
        "\033[38;2;116;199;236m                 .888888.                  \033[0m",
        "\033[38;2;137;180;250m                .88888888.                 \033[0m",
        "\033[38;2;137;180;250m               .8888888888.                \033[0m",
        "\033[38;2;137;180;250m              .888888888888.               \033[0m",
        "\033[38;2;180;190;254m             .88888888888888.              \033[0m",
        "\033[38;2;180;190;254m            .8888888888888888.             \033[0m",
        "\033[38;2;180;190;254m           .888888P\"  \"Y888888.            \033[0m",
        "\033[38;2;203;166;247m          .88888P'      `Y88888.           \033[0m",
        "\033[38;2;203;166;247m         .88888P          Y88888.          \033[0m",
        "\033[38;2;203;166;247m        .88888P            Y88888.         \033[0m",
        "\033[38;2;245;194;231m       .88888P    .8888.    Y88888.        \033[0m",
        "\033[38;2;245;194;231m      .88888P    .888888.    Y88888.       \033[0m",
        "\033[38;2;245;194;231m     .88888P    .88888888.    Y88888.      \033[0m",
        "\033[38;2;245;194;231m    .88888P    .8888888888.    Y88888.     \033[0m",
        "\033[38;2;245;194;231m   .88888P    .888888888888.    Y88888.    \033[0m",
        "\033[38;2;245;194;231m  .88888P    .88888888888888.    Y88888.   \033[0m"
    };

    // Generating the color blocks (Standard 8 + Bright 8)
    char c1[256] = "", c2[256] = "";
    for(int i=40; i<=47; i++) { char b[16]; snprintf(b, 16, "\033[%dm   ", i); strcat(c1, b); }
    for(int i=100; i<=107; i++) { char b[16]; snprintf(b, 16, "\033[%dm   ", i); strcat(c2, b); }
    strcat(c1, "\033[0m"); strcat(c2, "\033[0m");

    // Expanded buffer size to 1024 to eliminate compiler truncation warnings!
    // Expanded array to 20 lines to hold text + spacers + color strips!
    char info[20][1024];
    for(int i=0; i<20; i++) info[i][0] = '\0';

    snprintf(info[0], 1024, "\033[1;36m%s\033[0m@\033[1;36m%s\033[0m", user, hostname);
    snprintf(info[1], 1024, "-----------------------------");
    snprintf(info[2], 1024, "\033[1;36m%-10s\033[0m: %s", "OS", os);
    snprintf(info[3], 1024, "\033[1;36m%-10s\033[0m: %s", "Host", host);
    snprintf(info[4], 1024, "\033[1;36m%-10s\033[0m: %s", "Kernel", kernel);
    snprintf(info[5], 1024, "\033[1;36m%-10s\033[0m: %s", "Uptime", uptime);
    snprintf(info[6], 1024, "\033[1;36m%-10s\033[0m: %s", "Packages", pkgs);
    
    // NEW DATA INSERTED DIRECTLY BELOW PACKAGES
    snprintf(info[7], 1024, "\033[1;36m%-10s\033[0m: %s", "Mode", mode_str);
    snprintf(info[8], 1024, "\033[1;36m%-10s\033[0m: %.2f", "CPU Load", load);
    snprintf(info[9], 1024, "\033[1;36m%-10s\033[0m: %s", "RAM Bar", ram_bar);
    
    // CONTINUING REGULAR DATA
    snprintf(info[10], 1024, "\033[1;36m%-10s\033[0m: %s", "Shell", shell);
    snprintf(info[11], 1024, "\033[1;36m%-10s\033[0m: %s", "Resolution", res);
    snprintf(info[12], 1024, "\033[1;36m%-10s\033[0m: %s", "DE", de);
    snprintf(info[13], 1024, "\033[1;36m%-10s\033[0m: %s", "WM Theme", theme);
    snprintf(info[14], 1024, "\033[1;36m%-10s\033[0m: %s", "Terminal", term);
    snprintf(info[15], 1024, "\033[1;36m%-10s\033[0m: %s", "GPU", gpu);
    snprintf(info[16], 1024, "\033[1;36m%-10s\033[0m: %s", "Memory", mem);
    
    // SPACER + COLOR STRIPS
    snprintf(info[17], 1024, " ");
    snprintf(info[18], 1024, "%s", c1);
    snprintf(info[19], 1024, "%s", c2);

    // Print the unified screen
    printf("\033[2J\033[H\n");
    for(int i = 0; i < 20; i++) {
        // Drop the logo structure in the first 18 lines, print blank space for lines 19 & 20
        const char *left_pad = (i < 18) ? logo[i] : "                                           ";
        printf("  %s  %s\n", left_pad, info[i]);
        fflush(stdout);
        usleep(25000); 
    }
    printf("\n");
}

#endif
