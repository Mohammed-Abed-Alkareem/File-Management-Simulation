#include "inspectors.h"

Config config;

int main (int argc , char * argv[]){

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }

    if(!dirExists(unprocessedDir)){
        
        // Create directories for the files
        if (createDirectory(unprocessedDir) == -1) {
            perror("Error creating files directory");
            return 1;
        }
        
    }

    sleep(10);

    return 0;
}