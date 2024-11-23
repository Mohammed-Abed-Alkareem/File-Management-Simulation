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
   /*
*
*
*
*
*
*/

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


           // if (strcmp(key, "MIN_WEIGHT") == 0) config->MIN_WEIGHT = value;
           // else if (strcmp(key, "MAX_WEIGHT") == 0) config->MAX_WEIGHT = value;


         
        }
    }

    fclose(file); // Close the config file



    return 0; // Success
}
