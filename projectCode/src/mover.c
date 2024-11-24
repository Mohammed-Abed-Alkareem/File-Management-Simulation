#include "mover.h"

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


    if(!dirExists(processesdDir)){
        
        // Create directories for the files
        createDirectory(processesdDir);
        
    }
    

        ////////////////////////////
    // Access the message queue


    char* key_str = getenv("MSG_QUEUE_CM_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    int key = atoi(key_str);
    int msgid_mover = msgget(key, 0666);

    if (msgid_mover == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }

    // Message structure
    struct msgbuf message;

    while (1) {
        if (msgrcv(msgid_mover, &message, sizeof(message.file_number), 1, 0) == -1) {
            perror("Error receiving message from queue");
            return 1;
        }

        int file_number = message.file_number;
        printf("Mover %d received file number: %d\n",getpid(), file_number);
        //move the file to the processed directory
        char filename[100];
        sprintf(filename, "%d.csv",file_number);

        movefile(filename, homeDir, processesdDir);

        printf("Mover %d moved file number: %d\n", getpid(), file_number);
    }

    

    return 0;
}