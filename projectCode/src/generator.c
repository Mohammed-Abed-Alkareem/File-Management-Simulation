
#include "generator.h"


Config config;

// Semaphore operations
void sem_wait(int sem_id) {
    struct sembuf sb = {0, -1, 0}; // Decrement semaphore
    if (semop(sem_id, &sb, 1) == -1) {
        perror("Semaphore wait operation failed");
        exit(1); // Exit the program if semaphore operation fails
    }
    // printf("\033[0;31mProcess:%d => Semaphore wait operation\033[0m\n", getpid());
}

void sem_signal(int sem_id) {
    struct sembuf sb = {0, 1, 0}; // Increment semaphore
    if (semop(sem_id, &sb, 1) == -1) {
        perror("Semaphore signal operation failed");
        exit(1); // Exit the program if semaphore operation fails
    }
    // printf("\033[0;31mProcess:%d => Semaphore signal operation\033[0m\n", getpid());
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        perror("Invalid number of arguments");
        return 1;
    }

    if (load_config(argv[1], &config) == -1) {
        perror("Error loading configuration");
        return 1;
    }

    srand(time(NULL) + getpid());

    if (!dirExists(homeDir)) {
        createDirectory(homeDir);
    }

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

    key_t shm_key = atoi(argv[2]);
    key_t sem_key = atoi(argv[3]);

//    printf("Process %d: Attempting to retrieve shared memory with key: %d\n", getpid(), shm_key);
int shm_id = shmget(shm_key, sizeof(int), 0666);
if (shm_id == -1) {
    perror("Shared memory retrieval failed");
    return 1;
}
// printf("Process %d: Retrieved shared memory with ID: %d\n", getpid(), shm_id);


int *file_counter = (int *)shmat(shm_id, NULL, 0);
if (file_counter == (void *)-1) {
    perror("Shared memory attach failed");
    return 1;
}
// printf("Process %d: Attached shared memory. file_counter address: %p\n", getpid(), (void *)file_counter);


//     printf("Process %d: Current file_counter value: %d (address: %p)\n", getpid(), *file_counter, (void *)file_counter);
// fflush(stdout);

    int sem_id = semget(sem_key, 1, 0666);
    if (sem_id == -1) {
        perror("Semaphore retrieval failed");
        return 1;
    }

    printf("\033[0;31mProcess:%d => Generator process started\033[0m\n", getpid());


int sem_value = semctl(sem_id, 0, GETVAL);
if (sem_value == -1) {
    perror("Failed to get semaphore value");
    exit(1);
}
// printf("Initial semaphore value: %d\n", sem_value);

    for (int i = 0; i < 10; i++) {
        sleep(1);
        sem_wait(sem_id);
        // printf("\033[0;34mProcess:%d => Semaphore value: %d\033[0m\n", getpid(), semctl(sem_id, 0, GETVAL));
        int file_number = (*file_counter)++;
        // printf("\033[0;31mProcess:%d => File number: %d\033[0m\n", getpid(), file_number);
        sem_signal(sem_id);

        // printf("\033[0;31mProcess:%d => Generating CSV file: %d\033[0m\n", getpid(), file_number);

        generateCSV(file_number);

        struct msgbuf message;
        message.mtype = 1;
        message.file_number = file_number;

        if (msgsnd(msgid, &message, sizeof(message.file_number), 0) == -1) {
            perror("Message send failed");
            exit(1);
        } else {
            printf("Process:%d => Sent file number to queue: %d\n", getpid(), message.file_number);
        }

        sleep(5);
    }

    if (shmdt(file_counter) == -1) {
        perror("Shared memory detach failed");
    }

    return 0;
}




float getRandomFloat(int min, int max) //if any other file requiers this it can be moved to utils.c
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
    // code here
}

int getRandomInt(int min, int max)
{

    return rand() % (max - min + 1) + min;
}

void generateCSV(int fileNum)
{
    int i = fileNum;
    char * HOME_DIR = homeDir;
    char filename[100];

    int Rows = 10000, Cols = 10;
    int minValue = -100, maxValue = 100;
    // int Rows = 3, Cols = 2;

     sprintf(filename, "%s/%d.csv", HOME_DIR, i);

        FILE *file = fopen(filename, "w");
        if (!file) {
            perror("File creation failed");
            exit(1);
        }

        if (!(config.MIN_ROW ==-1 || config.MAX_ROW ==-1))
        {
            Rows = getRandomInt(config.MIN_ROW, config.MAX_ROW);
           
        }

        if(!(config.MIN_COLUMN ==-1 || config.MAX_COLUMN ==-1))
        {
            Cols = getRandomInt(config.MIN_COLUMN, config.MAX_COLUMN);
        }

        if(!(config.MIN_VALUE ==-1 || config.MAX_VALUE ==-1))
        {
            minValue = config.MIN_VALUE;
            maxValue = config.MAX_VALUE;
        }

        // printf("\033[0;34mRows: %d, Cols: %d, MinValue: %d, MaxValue: %d\033[0m\n", Rows, Cols, minValue, maxValue);
        // //print missing percentage
        // printf("\033[0;34mMissing percentage: %f\033[0m\n", config.MISS_PERCENTAGE);



 
        

        for (int i = 0; i < Rows; i++)
        {
            for (int j = 0; j < Cols; j++)
            {

                if (getRandomFloat(0, 1) < config.MISS_PERCENTAGE)// missing value
                {
                    if (j < Cols - 1)
                     fprintf(file, " ,");
 
                }
                else
                {
                    fprintf(file, "%f", getRandomFloat(minValue, maxValue)); // random value
                    // fprintf(file, "%f", getRandomFloat(3, 15)); // random value
                    if (j < Cols - 1)
                    {
                        fprintf(file, ",");
                    }
                }


            }
            fprintf(file, "\n");
        }

        fclose(file);
        printf("\033[0;32mProcess:%d => CSV file: %s generated successfully\033[0m\n",getpid(), filename);

       




}
