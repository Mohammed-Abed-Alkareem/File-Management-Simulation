
#include "generator.h"


Config config;
int shm_data_id;
int sem_data_id;
SharedData *shared_data ;
int *file_counter;

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

    #ifdef __CLI
    printf("\033[0;31mProcess:%d => Generator process started\033[0m\n", getpid());
    fflush(stdout);
    #endif

    // handel the sigint signal
    signal(SIGINT, sigint_handler);


    semaphore_wait(sem_id); //wait for the semaphore to be available

    if (!dirExists(homeDir)) {
        createDirectory(homeDir);
        //send signal to main process ppid
        kill(getppid(), SIGUSR1);

    }
    semaphore_signal(sem_id); //release the semaphore


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


int shm_id = shmget(shm_key, sizeof(int), 0666);
if (shm_id == -1) {
    perror("Shared memory retrieval failed");
    return 1;
}


file_counter = (int *)shmat(shm_id, NULL, 0);
if (file_counter == (void *)-1) {
    perror("Shared memory attach failed");
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


    while (1) {
        // get the file number from shared memory and increment it
        semaphore_wait(sem_id);
        int file_number = (*file_counter)++;
        semaphore_signal(sem_id);
        
        #ifdef __DEBUG
        printf("\033[0;31mProcess:%d => Generating CSV file: %d\033[0m\n", getpid(), file_number);
        fflush(stdout);
        #endif

        generateCSV(file_number);

        // increment total_csv_generated
        semaphore_wait(sem_data_id);
        shared_data->total_csv_generated++;
        semaphore_signal(sem_data_id);

        // send the file number to the message queue for calculator
        struct msgbuf message_calc;
        message_calc.mtype = 1;
        message_calc.file_number = file_number;

        if (msgsnd(msg_calc_id, &message_calc, sizeof(message_calc.file_number), 0) == -1) {
            perror("Message send failed");
            exit(1);
        } 
        #ifdef __DEBUG
        else {
            printf("Process:%d => Sent file number to queue: %d\n", getpid(), message_calc.file_number);
        }
        #endif

        // send the file number and time to the message queue for inspector1
        struct msgbuf2 message_insp1;
        message_insp1.mtype = 1;
        message_insp1.file_number = file_number;
        message_insp1.time = time(NULL);

        if (msgsnd(msg_insp1_id, &message_insp1, sizeof(message_insp1.file_number) + sizeof(message_insp1.time), 0) == -1) {
            perror("Message send failed");
            exit(1);
        }
        #ifdef __DEBUG
         else {
            printf("Process:%d => Sent file number and time to queue: %d\n", getpid(), message_insp1.file_number);
        }
        #endif
        

    
    }

    return 0;
}




float getRandomFloat(int min, int max) 
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */

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

        //set the rows, columns, min value and max value
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
 
        //write the csv file with random values and missing values
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
                    if (j < Cols - 1)
                    {
                        fprintf(file, ",");
                    }
                }


            }
            fprintf(file, "\n");
        }

        fclose(file);

        #ifdef __CLI
        printf("\033[0;32mProcess:%d => CSV file: %s generated successfully\033[0m\n",getpid(), filename);
        fflush(stdout);
        #endif
}



void sigint_handler(int sig)
{
    #ifdef __CLI
    printf("\033[0;31mProcess:%d => SIGINT received %d \033[0m\n", getpid() , sig);
    fflush(stdout);
    #endif
    
     if (shmdt(file_counter) == -1) {
        perror("Shared memory detach failed");
    }

    if (shmdt(shared_data) == -1) {
        perror("Shared memory detach failed");
    }
    exit(0);
}