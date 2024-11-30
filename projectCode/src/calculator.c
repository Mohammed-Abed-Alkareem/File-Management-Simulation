#include "calculator.h"

Config config;// Global variable for the configuration

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
    char *key_str = getenv("MSG_QUEUE_GC_KEY");//get the key of the message queue
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }
    
    int key = atoi(key_str);//convert the key to integer
    int msgid_generator = msgget(key, 0666);//get the message queue id
    if (msgid_generator == -1) {//check if the message queue id is valid
        perror("Message queue retrieval failed");
        return 1;
    }
    #ifdef __DEBUG
    printf("key str %s\n" , key_str);
    printf ("cla key %d \n", key);
    printf("message queue id %d \n",msgid_generator);
    #endif



    //access the message queue between clac and insp1
    key_str = getenv("MSG_QUEUE_I1C_KEY");//get the key of the message queue
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    key = atoi(key_str);//convert the key to integer
    int msgid_insp1 = msgget(key, 0666);//get the message queue id

    if (msgid_insp1 == -1) {//check if the message queue id is valid
        perror("Message queue retrieval failed");
        return 1;
    }

    ////////////////////////////
    // Access the message queue


    key_str = getenv("MSG_QUEUE_CM_KEY");//get the key of the message queue
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    key = atoi(key_str);//convert the key to integer
    int msgid_mover = msgget(key, 0666);//get the message queue id

    if (msgid_mover == -1) {//check if the message queue id is valid
        perror("Message queue retrieval failed");
        return 1;
    }



    // Message structure
    struct msgbuf message;
    char  fileName [200];
    int file_number;
      while (1) {
        // Receive a message from the queue
 
        //print the key of the message queue and message id
        usleep(20000);//sleep for 20 ms


        if (msgrcv(msgid_generator, &message, sizeof(message.file_number), 1, 0) < 0) {//receive the message from the generator with type 1
            perror("Error receiving message from queue");
            return 1;
        }

        file_number = message.file_number;//get the file number
        sprintf(fileName, "%s/%d.csv", homeDir, file_number); // change the path if needed 
        #ifdef __DEBUG
        printf("Calculator %d received file number: %d\n",getpid(), file_number);
        #endif
        //calculate the average        
        // send the file number to the inspector 1 from 1 to the number of inspector1 as msg type
        for (int i = 0; i < config.NUM_INSPECTOR1; i++) {//send the file number to the inspector 1
            message.mtype = i + 1;//set the message type
            if (msgsnd(msgid_insp1, &message, sizeof(message.file_number), 0) == -1) {//send the message to the inspector 1
                perror("Message send failed");
                return 1;
            } else {
                #ifdef __DEBUG
                printf("Calculator %d sent file number to queue: %d\n", getpid(), file_number);
                #endif
            }
        }



        calculateAvgCSV(fileName, file_number); // Calculate the average of the CSV file
        //when cannot open file skip
        //sleep(2);//dummy 


        // Send the file number to the mover
        message.mtype = 1;//set the message type
        message.file_number = file_number;//set the file number
        if (msgsnd(msgid_mover, &message, sizeof(message.file_number), 0) == -1) {//send the message to the mover
            perror("Message send failed");
            return 1;
        } else {
            #ifdef __DEBUG
            printf("Calculator %d sent file number to queue: %d\n", getpid(), file_number);
            #endif
        }

      }


    return 0;
}
//calculate the average of the csv file
float calculateAvgCSV(char *filename , int file_number) {
    //int rows = getNumRowsCSV(filename); // Get the number of rows -- not used
    int cols = getNumColsCSV(filename); // Get the number of columns

    FILE *file = fopen(filename, "r");//open the file
    if (file == NULL) {//check if the file is opened
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        return -1;
    }
    //open the named semaphore that is created by the generator wich coresponed to the file number
    char sem_name[150];//create the name of the semaphore
    sprintf(sem_name, "/sem_%d", file_number);//set the name of the semaphore
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 0);//open the semaphore
    if (sem == SEM_FAILED) {
        perror("Semaphore creation failed");
        exit(1);
    }
    // change to try wait if not aquiared move on .
    sem_wait(sem);// lock the semaphore
    float *sum = (float *)malloc(cols * sizeof(float)); // Array to hold the sums for each column
    int *count = (int *)malloc(cols * sizeof(int)); // Array to hold the count of valid numbers for each column

    memset(sum, 0, cols * sizeof(float)); // Initialize sums to 0
    memset(count, 0, cols * sizeof(int)); // Initialize counts to 0

    char line[256];
    char *token;
    int col_index ; 
    while (fgets(line, sizeof(line), file)) {
        // Tokenize the line using ',' as the delimiter
        // replace the new line with end of string 
        line[strcspn(line, "\n")] = '\0';

        token = strtok(line, ",");
        col_index = 0;
        //printf("line: %s file : %s \n ", line , filename);
        while (token != NULL) {
            // If the token is not empty, process it
            if (strcmp(token," ") != 0) {
                sum[col_index] += atof(token); // Add the value to the column sum
                count[col_index]++; // Increment the count for the column
            }
            token = strtok(NULL, ","); // Get the next token (next column)
            col_index++;
        }
    }
    #ifdef __DEBUG
    printf("exit file : %s \n ", filename);
    #endif
    fclose(file);
    sem_post(sem);// release the semaphore
    sem_close(sem); // close the semaphore

    // Calculate and print the average for each column
    for (int i = 0; i < cols; i++) {
            #ifdef __DEBUG
            printf("file: %s Column %d average: %.6f\n",filename , i + 1, sum[i] / count[i]);
            printf("file : %s Column %d has %d values\n", filename , i + 1, count[i]);
            #endif
    }

    free(sum); // Free the allocated memory for sums
    free(count); // Free the allocated memory for counts
    #ifdef __CLI
    printf("finishding file  : %s \n ", filename);
    #endif
    return 0;
}
//get the number of rows in the csv file
int getNumRowsCSV(char *filename) {
    FILE *file = fopen(filename, "r");//open the file
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
//get the number of columns in the csv file
int getNumColsCSV(char *filename) {
    FILE *file = fopen(filename, "r");//open the file
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        return -1;
    }

    int count = 0;
    char line[256];
    if (fgets(line, sizeof(line), file)) {
        // Count the commas in the first row to determine the number of columns
        for (int i = 0; i < (int)strlen(line); i++) {
            if (line[i] == ',') {
                count++;
            }
        }
        count++; // Add 1 to the column count for the last column
    }

    fclose(file);
    return count;
}
