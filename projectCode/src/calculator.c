#include "calculator.h"

   Config config;
int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);

    }

    // Load the configuration file
 
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }
    



     // Access the message queue
    char *key_str = getenv("MSG_QUEUE_GC_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }
    
    int key = atoi(key_str);
    int msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }

    // Message structure
    struct msgbuf message;

      while (1) {
        // Receive a message from the queue
        if (msgrcv(msgid, &message, sizeof(message.file_number), 1, 0) == -1) {
            perror("Error receiving message from queue");
            return 1;
        }

        int file_number = message.file_number;
        printf("Calculator %d received file number: %d\n",getpid(), file_number);
      }


    return 0;
}

float calculateAvgCSV(char *filename) {
    int rows = getNumRowsCSV(filename); // Get the number of rows
    int cols = getNumColsCSV(filename); // Get the number of columns

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        return -1;
    }

    float *sum = (float *)malloc(cols * sizeof(float)); // Array to hold the sums for each column
    int *count = (int *)malloc(cols * sizeof(int)); // Array to hold the count of valid numbers for each column

    memset(sum, 0, cols * sizeof(float)); // Initialize sums to 0
    memset(count, 0, cols * sizeof(int)); // Initialize counts to 0

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Tokenize the line using ',' as the delimiter
        char *token = strtok(line, ",");
        int col_index = 0;

        while (token != NULL) {
            // If the token is not empty, process it
            if (strlen(token) > 0) {
                sum[col_index] += atof(token); // Add the value to the column sum
                count[col_index]++; // Increment the count for the column
            }
            token = strtok(NULL, ","); // Get the next token (next column)
            col_index++;
        }
    }

    fclose(file);

    // Calculate and print the average for each column
    for (int i = 0; i < cols; i++) {
      
            printf("Column %d average: %.6f\n", i + 1, sum[i] / count[i]);
            printf("Column %d has %d values\n", i + 1, count[i]);
         
    }

    free(sum); // Free the allocated memory for sums
    free(count); // Free the allocated memory for counts

    return 0;
}

int getNumRowsCSV(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        return -1;
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        count++; // Count each line as a row
    }

    fclose(file);
    return count;
}

int getNumColsCSV(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        return -1;
    }

    int count = 0;
    char line[256];
    if (fgets(line, sizeof(line), file)) {
        // Count the commas in the first row to determine the number of columns
        for (int i = 0; i < strlen(line); i++) {
            if (line[i] == ',') {
                count++;
            }
        }
        count++; // Add 1 to the column count for the last column
    }

    fclose(file);
    return count;
}
