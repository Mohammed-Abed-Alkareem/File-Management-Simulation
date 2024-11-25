#include "inspectors.h"

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

    if(!dirExists(backupDir)){
        
            // Create directories for the files
            if (createDirectory(backupDir) == -1) {
                perror("Error creating files directory");
                return 1;
            }
        
    }

    semaphore_signal(sem_id);

    sleep(15);

    return 0;
}