#include "config.h"

// Function to load configuration settings from a specified file
int load_config(const char *filename, Config *config) {
    // Attempt to open the config file in read mode
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening config file"); // Print an error message if file opening fails
        return -1; // Return error code
    }

    // Initialize all configuration values to default or invalid values to indicate uninitialized state
   
    config->MIN_TIME = -1;
    config->MAX_TIME = -1;
    config->MIN_COLUMN = -1;
    config->MAX_COLUMN = -1;
    config->NUM_GENERATORS = -1;
    config->MIN_ROW = -1;
    config->MAX_ROW = -1;
    config->MIN_VALUE = -1;
    config->MAX_VALUE = -1;
    config->MISS_PERCENTAGE = -1;
    config->NUM_CALCULATORS = -1;
    config->NUM_MOVERS = -1;
    config->NUM_INSPECTOR1 = -1;
    config->INSPECTOR1_THRESHOLD = -1;
    config->NUM_INSPECTOR2 = -1;
    config->INSPECTOR2_THRESHOLD = -1;
    config->NUM_INSPECTOR3 = -1;
    config->INSPECTOR3_THRESHOLD = -1;
    config->PROCESSED_THRESHOLD = -1;
    config->UNPROCESSED_THRESHOLD = -1;
    config->BACKUP_THRESHOLD = -1;
    config->DELETED_THRESHOLD = -1;


    // Buffer to hold each line from the configuration file
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Ignore comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;

        // Parse each line as a key-value pair
        char key[50];
        int value;
        if (sscanf(line, "%40[^=]=%d", key, &value) == 2) {
            // Set corresponding config fields based on the key

            if (strcmp(key,"MIN_TIME") == 0) config->MIN_TIME = value;
            else if (strcmp(key, "MAX_TIME") == 0) config->MAX_TIME = value;
            else if (strcmp(key, "MIN_COLUMN") == 0) config->MIN_COLUMN = value;
            else if (strcmp(key, "MAX_COLUMN") == 0) config->MAX_COLUMN = value;
            else if (strcmp(key, "NUM_GENERATORS") == 0) config->NUM_GENERATORS = value;
            else if (strcmp(key, "MIN_RAW") == 0) config->MIN_ROW = value;
            else if (strcmp(key, "MAX_RAW") == 0) config->MAX_ROW = value;
            else if (strcmp(key, "MIN_VALUE") == 0) config->MIN_VALUE = value;
            else if (strcmp(key, "MAX_VALUE") == 0) config->MAX_VALUE = value;
            else if (strcmp(key, "MISS_PERCENTAGE") == 0) config->MISS_PERCENTAGE = value;
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
    }

    fclose(file); // Close the config file



    return 0; // Success
}
