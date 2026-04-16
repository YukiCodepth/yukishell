#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if(argc < 4) {
        printf("\033[31mUsage: <cmd> | yql <COL_NUM> <gt|lt|eq> <VALUE>\033[0m\n");
        return 1;
    }
    int target_col = atoi(argv[1]);
    char *op = argv[2];
    double threshold = atof(argv[3]);
    char line[1024];
    int is_header = 1;
    
    while(fgets(line, sizeof(line), stdin)) {
        if(is_header) { printf("\033[1;36m%s\033[0m", line); is_header = 0; continue; }
        char line_copy[1024]; strcpy(line_copy, line);
        char *token = strtok(line_copy, " \t");
        int current_col = 1; double cell_val = 0.0; int match = 0;
        
        while(token != NULL) {
            if(current_col == target_col) {
                cell_val = atof(token);
                if(strcmp(op, "gt") == 0 && cell_val > threshold) match = 1;
                if(strcmp(op, "lt") == 0 && cell_val < threshold) match = 1;
                if(strcmp(op, "eq") == 0 && cell_val == threshold) match = 1;
                break;
            }
            token = strtok(NULL, " \t"); current_col++;
        }
        if(match) printf("\033[32m%s\033[0m", line);
    }
    return 0;
}
