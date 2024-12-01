#include "config.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// Function to load configuration settings from a specified file
int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening config file");
        return -1;
    }

    // Initialize all configuration values to default or invalid states
    // config->MIN_TIME = -1;
    config->MAX_TIME = 1.5 ;
    config->MIN_COLUMN = -1;
    config->MAX_COLUMN = -1;
    config->NUM_GENERATORS = 5;
    config->MIN_ROW = -1;
    config->MAX_ROW = -1;
    config->MIN_VALUE = -1;
    config->MAX_VALUE = -1;
    config->MISS_PERCENTAGE = .2f;
    config->NUM_CALCULATORS = 5;
    config->NUM_MOVERS = 5;
    config->NUM_INSPECTOR1 = 5;
    config->INSPECTOR1_THRESHOLD = 2;
    config->NUM_INSPECTOR2 = 5;
    config->INSPECTOR2_THRESHOLD = 5 ;
    config->NUM_INSPECTOR3 = 5 ;
    config->INSPECTOR3_THRESHOLD = 5 ;
    config->PROCESSED_THRESHOLD = 300;
    config->UNPROCESSED_THRESHOLD = 100;
    config->BACKUP_THRESHOLD = 100;
    config->DELETED_THRESHOLD = 50;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Ignore comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;

        char key[50];
        char value_str[50];
        if (sscanf(line, "%49[^=]=%49s", key, value_str) == 2) {

            // Check for int value first
            if (atof(value_str) == round(atof(value_str))) { // Check if value is an integer
                int value = atoi(value_str); // Parse as int
                // if (strcmp(key, "MIN_TIME") == 0) config->MIN_TIME = value;
               if (strcmp(key, "MAX_TIME") == 0) config->MAX_TIME = value;
                else if (strcmp(key, "MIN_COLUMN") == 0) config->MIN_COLUMN = value;
                else if (strcmp(key, "MAX_COLUMN") == 0) config->MAX_COLUMN = value;
                else if (strcmp(key, "NUM_GENERATORS") == 0) config->NUM_GENERATORS = value;
                else if (strcmp(key, "MIN_ROW") == 0) config->MIN_ROW = value;
                else if (strcmp(key, "MAX_ROW") == 0) config->MAX_ROW = value;
                else if (strcmp(key, "MIN_VALUE") == 0) config->MIN_VALUE = value;
                else if (strcmp(key, "MAX_VALUE") == 0) config->MAX_VALUE = value;
                else if (strcmp(key, "NUM_CALCULATORS") == 0) config->NUM_CALCULATORS = value;
                else if (strcmp(key, "NUM_MOVERS") == 0) config->NUM_MOVERS = value;
                else if (strcmp(key, "NUM_INSPECTOR1") == 0) config->NUM_INSPECTOR1 = value;
                else if (strcmp(key, "INSPECTOR1_THRESHOLD") == 0) config->INSPECTOR1_THRESHOLD = value;
                else if (strcmp(key, "NUM_INSPECTOR2") == 0) config->NUM_INSPECTOR2 = value;
                else if (strcmp(key, "INSPECTOR2_THRESHOLD") == 0) config->INSPECTOR2_THRESHOLD = value;
                else if (strcmp(key, "NUM_INSPECTOR3") == 0) config->NUM_INSPECTOR3 = value;
                else if (strcmp(key, "INSPECTOR3_THRESHOLD") == 0) config->INSPECTOR3_THRESHOLD = value;
                else if (strcmp(key, "PROCESSED_THRESHOLD") == 0) config->PROCESSED_THRESHOLD = value;
                else if (strcmp(key, "UNPROCESSED_THRESHOLD") == 0) config->UNPROCESSED_THRESHOLD = value;
                else if (strcmp(key, "BACKUP_THRESHOLD") == 0) config->BACKUP_THRESHOLD = value;
                else if (strcmp(key, "DELETED_THRESHOLD") == 0) config->DELETED_THRESHOLD = value;
                else {
                    fprintf(stderr, "Unknown key: %s\n", key);
                }
            } 
            
            // Check for float value
            else {
                float value = atof(value_str); // Parse as float
                if (strcmp(key, "MISS_PERCENTAGE") == 0) config->MISS_PERCENTAGE = value;
                else if (strcmp(key, "MAX_TIME") == 0) config->MAX_TIME = value;
                else {
                    fprintf(stderr, "Unexpected float key: %s\n", key);
                }
            }
        }
    }

    fclose(file);
    return 0;
}
