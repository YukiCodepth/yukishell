#include "../include/yukishell.h"

int execute_builtin(char **args) {
    if (args[0] == NULL) return 1; 

    if(strcmp(args[0], "exit") == 0) {
        printf("Closing YukiShell...\n");
        exit(0); // Parent shell exiting politely. This stays normal!
    }

    if(strcmp(args[0], "serial") == 0) {
        if (args[1] == NULL) {
            printf("\x1b[31mUsage: serial <device_path> [baud_rate]\x1b[0m\n");
            printf("Example: serial /dev/ttyUSB0 115200\n");
            return 1;
        }

        char *portname = args[1];
        int target_baud = 115200; 
        if (args[2] != NULL) target_baud = atoi(args[2]);

        int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
            perror("\x1b[31mError opening serial port (Try running with sudo?)\x1b[0m");
            return 1;
        }

        speed_t speed;
        switch(target_baud) {
            case 9600:   speed = B9600;   break;
            case 19200:  speed = B19200;  break;
            case 38400:  speed = B38400;  break;
            case 57600:  speed = B57600;  break;
            case 115200: speed = B115200; break;
            default: speed = B115200;
        }

        struct termios tty;
        if (tcgetattr(fd, &tty) != 0) { perror("Error from tcgetattr"); close(fd); return 1; }
        
        cfsetospeed(&tty, speed); cfsetispeed(&tty, speed);
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        tty.c_iflag &= ~IGNBRK; tty.c_lflag = 0; tty.c_oflag = 0;        
        tty.c_cc[VMIN] = 1; tty.c_cc[VTIME] = 1;    
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); 
        tty.c_cflag |= (CLOCAL | CREAD); 
        tcsetattr(fd, TCSANOW, &tty);

        printf("\n\x1b[36m\x1b[1m[ YukiShell Serial Monitor ]\x1b[0m\n");
        printf("Listening on \x1b[32m%s\x1b[0m at \x1b[32m%d\x1b[0m baud...\n", portname, target_baud);
        printf("Press \x1b[33mCtrl+C\x1b[0m to exit.\n\n");

        pid_t pid = fork();
        if (pid == 0) {
            char buf[256]; int n;
            while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
                buf[n] = '\0'; printf("%s", buf); fflush(stdout);
            }
            _exit(0); // THE FIX: Ruthless termination for the serial child
        } else {
            waitpid(pid, NULL, 0); close(fd);
            printf("\n\x1b[31m[Serial Connection Closed]\x1b[0m\n");
        }
        return 1;
    }

    if(strcmp(args[0], "netscan") == 0) {
        printf("\n\x1b[36m\x1b[1m[ YukiShell Network Scanner ]\x1b[0m\n");
        FILE *fp = fopen("/proc/net/arp", "r");
        if (fp == NULL) { perror("\x1b[31mError accessing network data\x1b[0m"); return 1; }

        char line[256], ip[32], hw_type[32], flags[32], mac[32], mask[32], dev[32];
        fgets(line, sizeof(line), fp); 
        printf("\x1b[32m%-20s %-20s %-10s\x1b[0m\n", "IP Address", "MAC Address", "Interface");
        printf("------------------------------------------------------\n");

        int count = 0;
        while (fgets(line, sizeof(line), fp)) {
            sscanf(line, "%s %s %s %s %s %s", ip, hw_type, flags, mac, mask, dev);
            if (strcmp(mac, "00:00:00:00:00:00") != 0) {
                printf("%-20s %-20s %-10s\n", ip, mac, dev); count++;
            }
        }
        fclose(fp);
        printf("\n\x1b[32m[+] Found %d device(s) on the local network.\x1b[0m\n\n", count);
        return 1;
    }

    if(strcmp(args[0], "help") == 0) {
        printf("\n\x1b[36m\x1b[1m======================================================\x1b[0m\n");
        printf("\x1b[1m                 YukiShell Help Menu                  \x1b[0m\n");
        printf("\x1b[36m\x1b[1m======================================================\x1b[0m\n");
        printf("\x1b[32m[ Core Commands ]\x1b[0m\n");
        printf("  help       : Displays this formatted menu.\n");
        printf("  exit       : Safely terminates the shell process.\n");
        printf("  cd <dir>   : Changes the current working directory.\n\n");
        printf("\x1b[32m[ IoT & Hardware Toolkit ]\x1b[0m\n");
        printf("  netscan    : Scans Wi-Fi/LAN for connected devices.\n");
        printf("  serial     : Natively reads USB logs.\n");
        printf("\x1b[36m\x1b[1m======================================================\x1b[0m\n\n");
        return 1;
    }

    if(strcmp(args[0], "cd") == 0) {
        if(args[1] != NULL && chdir(args[1]) != 0) perror("cd failed");
        return 1;
    }
    return 0; 
}
