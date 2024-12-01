
#include "generator.h"


Config config;
int shm_data_id;
int sem_data_id;
SharedData *shared_data ;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        perror("Invalid number of arguments");
        return 1;
    }

    if (load_config(argv[1], &config) == -1) {
        perror("Error loading configuration");
        return 1;
    }
    
    key_t shm_key = atoi(argv[2]);
    key_t sem_key = atoi(argv[3]);

    srand(time(NULL) + getpid());
    int sem_id = semget(sem_key, 1, 0666);
    if (sem_id == -1) {
        perror("Semaphore retrieval failed");
        return 1;
    }

    printf("\033[0;31mProcess:%d => Generator process started\033[0m\n", getpid());

    // handel the sigint signal
    signal(SIGINT, sigint_handler);



    int sem_value = semctl(sem_id, 0, GETVAL);
    if (sem_value == -1) {
        perror("Failed to get semaphore value");
        exit(1);
    }
    // printf("Initial semaphore value: %d\n", sem_value);

    semaphore_wait(sem_id);

    if (!dirExists(homeDir)) {
        createDirectory(homeDir);
        //send signal to main process ppid
        kill(getppid(), SIGUSR1);

    }
    semaphore_signal(sem_id);


///message_calc queue for generator and calculator
    char *key_str = getenv("MSG_QUEUE_GC_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    int key = atoi(key_str);
    int msg_calc_id = msgget(key, 0666);
    if (msg_calc_id == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }

    ///////message_calc queue for generator and inspector1
    char *key_str_insp1 = getenv("MSG_QUEUE_GI1_KEY");
    if (key_str_insp1 == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    int key_insp1 = atoi(key_str_insp1);
    int msg_insp1_id = msgget(key_insp1, 0666);
    if (msg_insp1_id == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }


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



//     printf("Process %d: Current file_counter value: %d (address: %p)\n", getpid(), *file_counter, (void *)file_counter);
// fflush(stdout);

    //!! change this to be while loop for the current file num < total num of files
    while (1) {
        //sleep(1);
        semaphore_wait(sem_id);
        // printf("\033[0;34mProcess:%d => Semaphore value: %d\033[0m\n", getpid(), semctl(sem_id, 0, GETVAL));
        int file_number = (*file_counter)++;
        // printf("\033[0;31mProcess:%d => File number: %d\033[0m\n", getpid(), file_number);
        semaphore_signal(sem_id);
        

        // printf("\033[0;31mProcess:%d => Generating CSV file: %d\033[0m\n", getpid(), file_number);

        generateCSV(file_number);

        // increment total_csv_generated
        semaphore_wait(sem_data_id);
        shared_data->total_csv_generated++;

        semaphore_signal(sem_data_id);

        struct msgbuf message_calc;
        message_calc.mtype = 1;
        message_calc.file_number = file_number;

        if (msgsnd(msg_calc_id, &message_calc, sizeof(message_calc.file_number), 0) == -1) {
            perror("Message send failed");
            exit(1);
        } else {
            printf("Process:%d => Sent file number to queue: %d\n", getpid(), message_calc.file_number);
        }

        struct msgbuf2 message_insp1;
        message_insp1.mtype = 1;
        message_insp1.file_number = file_number;
        message_insp1.time = time(NULL);

        if (msgsnd(msg_insp1_id, &message_insp1, sizeof(message_insp1.file_number) + sizeof(message_insp1.time), 0) == -1) {
            perror("Message send failed");
            exit(1);
        } else {
            printf("Process:%d => Sent file number and time to queue: %d\n", getpid(), message_insp1.file_number);
        }
        

        //sleep(5);
    }

    if (shmdt(file_counter) == -1) {
        perror("Shared memory detach failed");
    }

    if (shmdt(shared_data) == -1) {
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

    //create named semaphore for each csv file
    char sem_name[150];
    sprintf(sem_name, "/sem_%d", i);
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);     
    if (sem == SEM_FAILED) {
        perror("Semaphore creation failed");
        exit(1);
    }

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
                    else
                    {
                        fprintf(file, " ");
                    }
 
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



void sigint_handler(int sig)
{
    
    printf("\033[0;31mProcess:%d => SIGINT received %d \033[0m\n", getpid() , sig);
    if (shmdt(shared_data) == -1) {
        perror("Shared memory detach failed");
    }
    exit(0);
}