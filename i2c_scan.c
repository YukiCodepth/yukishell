#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int main(int argc, char **argv) {
    int file;
    int port = 1; // Default to hardware bus 1 (/dev/i2c-1)
    char filename[20];
    
    // Allow user to specify a different bus (e.g., ./i2c_scan 0)
    if (argc == 2) {
        port = atoi(argv[1]);
    }
    
    snprintf(filename, 19, "/dev/i2c-%d", port);
    printf("\n\033[1;36mYuki Hardware Probe ❯\033[0m Scanning %s...\n\n", filename);
    
    file = open(filename, O_RDWR);
    if (file < 0) {
        printf("\033[31m[!] Fatal: Could not open %s.\033[0m\n", filename);
        printf("\033[90m(Note: I2C buses only exist natively on SBCs like Raspberry Pi/Jetson, or via USB-to-I2C bridges.)\033[0m\n\n");
        return 1;
    }
    
    // Print the Grid Header
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    
    // Loop through the hex grid
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            int addr = i + j;
            
            // Valid I2C addresses are only between 0x03 and 0x77
            if (addr < 0x03 || addr > 0x77) {
                printf("   ");
                continue;
            }
            
            // Set the slave address
            if (ioctl(file, I2C_SLAVE, addr) >= 0) {
                // Attempt to read 1 byte. If successful, a device is there!
                unsigned char buf[1];
                if (read(file, buf, 1) >= 0) {
                    printf("\033[1;32m%02x\033[0m ", addr); // Print found address in Bold Green
                } else {
                    printf("\033[90m--\033[0m "); // Print empty slots in Dim Gray
                }
            } else {
                printf("\033[90m--\033[0m ");
            }
        }
        printf("\n");
    }
    
    close(file);
    printf("\n");
    return 0;
}
