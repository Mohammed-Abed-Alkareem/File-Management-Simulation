#include "mover.h"

Config config;



int main (int argc , char * argv[]){
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }
    
    key_t sem_key = atoi(argv[2]);
    int sem_id = semget(sem_key, 1, 0666);
    if (sem_id == -1) {
        perror("Semaphore retrieval failed");
        return 1;
    }

    int sem_value = semctl(sem_id, 0, GETVAL);
    if (sem_value == -1) {
        perror("Failed to get semaphore value");
        exit(1);
    }

    semaphore_wait(sem_id);

    if(!dirExists(processesdDir)){
        
        // Create directories for the files
        createDirectory(processesdDir);
        
    }
    semaphore_signal(sem_id);
    

    // handel the sigint signal
    signal(SIGINT, sigint_handler);

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


    //message queue for the movers and inspector2
    key_str = getenv("MSG_QUEUE_MI2_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    key = atoi(key_str);
    int msgid_mover_insp2 = msgget(key, 0666);

    if (msgid_mover_insp2 == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }



    // Message structure
    struct msgbuf message;
    struct msgbuf2 message2;

    while (1) {
        usleep(20000);
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

        message2.mtype = 1;
        message2.file_number = file_number;
        message2.time = time(NULL);

        if (msgsnd(msgid_mover_insp2, &message2, sizeof(message2.file_number) + sizeof(message2.time), 0) == -1) {
            perror("Message send failed");
            return 1;
        }

        printf("Mover %d moved file number: %d\n", getpid(), file_number);

    }

    

    return 0;
}




void sigint_handler(int sig)
{
    printf("\033[0;31mProcess:%d => SIGINT received %d \033[0m\n", getpid(), sig);
    exit(0);
}