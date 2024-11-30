#include "mover.h"

Config config;// Global variable for the configuration



int main (int argc , char * argv[]){
    // Check the number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Load configuration
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }
    // Retrieve semaphore ID
    key_t sem_key = atoi(argv[2]);
    int sem_id = semget(sem_key, 1, 0666);
    if (sem_id == -1) {
        perror("Semaphore retrieval failed");
        return 1;
    }
    // Get semaphore value
    int sem_value = semctl(sem_id, 0, GETVAL);
    if (sem_value == -1) {
        perror("Failed to get semaphore value");
        exit(1);
    }

    semaphore_wait(sem_id);//lock the semaphore

    if(!dirExists(processesdDir)){//check if the processed directory exists
        
        // Create directories for the files
        createDirectory(processesdDir);//create the processed directory
        
    }
    semaphore_signal(sem_id);//unlock the semaphore
    

        ////////////////////////////
    // Access the message queue


    char* key_str = getenv("MSG_QUEUE_CM_KEY");//get the key of the message queue between calculator and mover
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    int key = atoi(key_str);//convert the key to integer
    int msgid_mover = msgget(key, 0666);

    if (msgid_mover == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }


    //message queue for the movers and inspector2
    key_str = getenv("MSG_QUEUE_MI2_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    key = atoi(key_str);//convert the key to integer
    int msgid_mover_insp2 = msgget(key, 0666);//get the message queue id

    if (msgid_mover_insp2 == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }



    // Message structure
    struct msgbuf message;
    struct msgbuf2 message2;

    while (1) {
        usleep(20000);//sleep for 20 ms
        if (msgrcv(msgid_mover, &message, sizeof(message.file_number), 1, 0) == -1) {//receive the message from the calculator with type 1
            perror("Error receiving message from queue");
            return 1;
        }

        int file_number = message.file_number;//get the file number from the message
        #ifdef __CLI
        printf("Mover %d received file number: %d\n",getpid(), file_number);
        #endif
        //move the file to the processed directory
        char filename[100];
        sprintf(filename, "%d.csv",file_number);//set the file name

        movefile(filename, homeDir, processesdDir);//move the file from the home directory to the processed directory
        //send the file number to the inspector 2
        message2.mtype = 1;
        message2.file_number = file_number;
        message2.time = time(NULL);

        if (msgsnd(msgid_mover_insp2, &message2, sizeof(message2.file_number) + sizeof(message2.time), 0) == -1) {//send the message to the inspector 2
            perror("Message send failed");
            return 1;
        }
        #ifdef __CLI

        printf("Mover %d moved file number: %d\n", getpid(), file_number);
        
        #endif

    }

    

    return 0;
}