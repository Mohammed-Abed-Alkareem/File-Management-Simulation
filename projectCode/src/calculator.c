#include "calculator.h"

Config config;
//const char *logFileSem = "/logFileSem";
int shm_data_id;
int sem_data_id;
SharedData *shared_data ;


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
    int msgid_generator = msgget(key, 0666);
    if (msgid_generator == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }
    printf("key str %s\n" , key_str);
    printf ("cla key %d \n", key);
    printf("message queue id %d \n",msgid_generator);



    //access the message queue between clac and insp1
    key_str = getenv("MSG_QUEUE_I1C_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    key = atoi(key_str);
    int msgid_insp1 = msgget(key, 0666);

    if (msgid_insp1 == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }

    ////////////////////////////
    // Access the message queue


    key_str = getenv("MSG_QUEUE_CM_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    key = atoi(key_str);
    int msgid_mover = msgget(key, 0666);

    if (msgid_mover == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }



//get the shared memory id from environment
    char *shm_data_key_str = getenv("SHM_DATA_KEY");
    if (shm_data_key_str == NULL) {
        fprintf(stderr, "Error: SHM_DATA_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int shm_data_key = atoi(shm_data_key_str);

    // Get shared memory ID
    shm_data_id = shmget(shm_data_key, sizeof(SharedData), 0666);
    if (shm_data_id == -1) {
        perror("Shared memory retrieval failed");
        exit(EXIT_FAILURE);
    }

    //Attach shared memory to shared_data
    shared_data = (SharedData *)shmat(shm_data_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Shared memory attach failed");
        exit(EXIT_FAILURE);
    }


    //get the semaphore id from environment
    char *sem_data_key_str = getenv("SEM_DATA_KEY");
    if (sem_data_key_str == NULL) {
        fprintf(stderr, "Error: SEM_DATA_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int sem_data_key = atoi(sem_data_key_str);

    // Get semaphore ID
    sem_data_id = semget(sem_data_key, 1, 0666);
    if (sem_data_id == -1) {
        perror("Semaphore retrieval failed");
        exit(EXIT_FAILURE);
    }



    // Message structure
    struct msgbuf message;
    char  fileName [200];
    int file_number;
      while (1) {
        // Receive a message from the queue
 
        //print the key of the message queue and message id
        usleep(20000);


        if (msgrcv(msgid_generator, &message, sizeof(message.file_number), 1, 0) < 0) {
            perror("Error receiving message from queue");
            return 1;
        }

        file_number = message.file_number;
        sprintf(fileName, "%s/%d.csv", homeDir, file_number); // change the path if needed 
        printf("Calculator %d received file number: %d\n",getpid(), file_number);
        //calculate the average
        

        
        // send the file number to the inspector 1 from 1 to the number of inspector1 as msg type
        for (int i = 0; i < config.NUM_INSPECTOR1; i++) {
            message.mtype = i + 1;
            if (msgsnd(msgid_insp1, &message, sizeof(message.file_number), 0) == -1) {
                perror("Message send failed");
                return 1;
            } else {
                printf("Calculator %d sent file number to Inspector queue : %d\n", getpid(), file_number);
            }
        }



        if (calculateAvgCSV(fileName, file_number) == -1 ){
            continue;
        } 



        //when cannot open file skip
        //sleep(2);//dummy 


        // Send the file number to the mover
        message.mtype = 1;
        message.file_number = file_number;
        if (msgsnd(msgid_mover, &message, sizeof(message.file_number), 0) == -1) {
            perror("Message send failed");
            return 1;
        } else {
            printf("Calculator %d sent file number to queue: %d\n", getpid(), file_number);
        }

      }

    // Detach the shared memory
    if (shmdt(shared_data) == -1) {
        perror("Shared memory detach failed");
        return 1;
    }


    return 0;
}

float calculateAvgCSV(char *filename , int file_number) {
    //int rows = getNumRowsCSV(filename); // Get the number of rows -- not used
    int rows = 0;
    int cols = getNumColsCSV(filename); // Get the number of columns

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        return -1;
    }
    //open the named semaphore that is created by the generator wich coresponed to the file number
    char sem_name[150];
    sprintf(sem_name, "/sem_%d", file_number);
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("Semaphore creation failed");
        exit(1);
    }
    // change to try wait if not aquiared move on .
    // try wait if not aquired then move on
    if (sem_trywait(sem) == -1) {
        sem_close(sem);
        return -1;
    }





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
        rows++;
    }
    // printf("exit file : %s \n ", filename);
    fclose(file);

    //!!!!
    //sem_post(sem);// release the semaphore
    sem_close(sem); // close the semaphore

    // // Calculate and print the average for each column
    // for (int i = 0; i < cols; i++) {
    //         if (count[i] == 0) {

    //             //printf("file: %s Column %d average: %.6f\n",filename , i + 1, 0.0);
    //             //printf("file : %s Column %d has %d values\n", filename , i + 1, 0);
    //             continue;
    //         }
    //         //printf("file: %s Column %d average: %.6f\n",filename , i + 1, sum[i] / count[i]);
    //         //printf("file : %s Column %d has %d values\n", filename , i + 1, count[i]);
         
    // }

    // Acquire the log file semaphore

    sem_t *log_sem = sem_open(logFileSem, O_CREAT, 0666, 1);
    if (log_sem == SEM_FAILED) {
        perror("Semaphore creation failed");
        exit(1);
    }
    sem_wait(log_sem); // Lock the semaphore
    printf("Log semaphore acquired\n");

    // Open the log file for append
    FILE *log_file = fopen(logFile, "a");
    if (log_file == NULL) {
        perror("Error: Unable to open log file\n");
        sem_post(log_sem); // Release the semaphore before exiting
        sem_close(log_sem); // Close the semaphore before exiting
        return -1;
    }
    printf("Log file opened for appending\n");

    // Write the CSV file name, number of rows, and columns
    fprintf(log_file, "File: %s\n", filename);
    fprintf(log_file, "Number of rows: %d\n", rows);
    fprintf(log_file, "Number of columns: %d\n", cols);
    printf("Logged file name, number of rows, and columns\n");

    // Write the average of each column and number of rows it has in one row for each column
    for (int i = 0; i < cols; i++) {
        fprintf(log_file, "Column %d average: %.6f, Number of values: %d\n", i + 1, sum[i] / count[i], count[i]);
        printf("Logged column %d average and number of values\n", i + 1);
    }
    fprintf(log_file, "\n");
    fclose(log_file);
    printf("Log file closed\n");

    sem_post(log_sem); // Release the semaphore

    printf("Log semaphore released\n");
    sem_close(log_sem); // Close the semaphore
    
    semaphore_wait(sem_data_id);
    shared_data->total_csv_calculated++;
    semaphore_signal(sem_data_id);
    
    printf("Log semaphore closed\n");

    


    free(sum); // Free the allocated memory for sums
    free(count); // Free the allocated memory for counts
    printf("finishding file  : %s \n ", filename);
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
