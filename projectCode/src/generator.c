
#include "generator.h"


Config config;// Global variable for the configuration


int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc != 4) {
        perror("Invalid number of arguments");
        return 1;
    }
//   load the configuration file
    if (load_config(argv[1], &config) == -1) {
        perror("Error loading configuration");
        return 1;
    }

    key_t shm_key = atoi(argv[2]);//get the key of the shared memory
    key_t sem_key = atoi(argv[3]);//get the key of the semaphore

    srand(time(NULL) + getpid());//set the seed for the random number generator
    int sem_id = semget(sem_key, 1, 0666);//get the semaphore id
    if (sem_id == -1) {
        perror("Semaphore retrieval failed");
        return 1;
    }
    #ifdef __DEBUG
    printf("\033[0;31mProcess:%d => Generator process started\033[0m\n", getpid());
    #endif

    int sem_value = semctl(sem_id, 0, GETVAL);//get the value of the semaphore
    if (sem_value == -1) {
        perror("Failed to get semaphore value");
        exit(1);
    }

    semaphore_wait(sem_id);//lock the semaphore

    if (!dirExists(homeDir)) {//check if the home directory exists
        createDirectory(homeDir);//create the home directory
        //send signal to main process ppid
        kill(getppid(), SIGUSR1);

    }
    semaphore_signal(sem_id);//unlock the semaphore


///message_calc queue for generator and calculator
    char *key_str = getenv("MSG_QUEUE_GC_KEY");//get the key of the message queue between generator and calculator
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    int key = atoi(key_str);//convert the key to integer    
    int msg_calc_id = msgget(key, 0666);//get the message queue id
    if (msg_calc_id == -1) {//check if the message queue id is valid
        perror("Message queue retrieval failed");
        return 1;
    }

    ///////message_calc queue for generator and inspector1
    char *key_str_insp1 = getenv("MSG_QUEUE_GI1_KEY");//get the key of the message queue between generator and inspector1
    if (key_str_insp1 == NULL) {//check if the key is set
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    int key_insp1 = atoi(key_str_insp1);//convert the key to integer for the message queue
    int msg_insp1_id = msgget(key_insp1, 0666);//get the message queue id
    if (msg_insp1_id == -1) {//check if the message queue id is valid
        perror("Message queue retrieval failed");
        return 1;
    }

    #ifdef __DEBUG
   printf("Process %d: Attempting to retrieve shared memory with key: %d\n", getpid(), shm_key);
    #endif
    int shm_id = shmget(shm_key, sizeof(int), 0666);
    if (shm_id == -1) {
        perror("Shared memory retrieval failed");
        return 1;
    }
    #ifdef __DEBUG
    printf("Process %d: Retrieved shared memory with ID: %d\n", getpid(), shm_id);
    #endif

    int *file_counter = (int *)shmat(shm_id, NULL, 0);//attach the shared memory
    if (file_counter == (void *)-1) {//check if the shared memory is attached
        perror("Shared memory attach failed");
        return 1;
    }


    //!! change this to be while loop for the current file num < total num of files
    for (int i = 0; i < 2; i++) {
        //sleep(1);
        semaphore_wait(sem_id);//lock the semaphore
        int file_number = (*file_counter)++;//increment the file counter
        semaphore_signal(sem_id);//unlock the semaphore
        

        generateCSV(file_number);//generate the csv file

        struct msgbuf message_calc;//message structure for the calculator
        message_calc.mtype = 1;//set the message type
        message_calc.file_number = file_number;//set the file number

        if (msgsnd(msg_calc_id, &message_calc, sizeof(message_calc.file_number), 0) == -1) {///send the message to the calculator
            perror("Message send failed");
            exit(1);
        } else {
            #ifdef __DEBUG
            printf("Process:%d => Sent file number to queue: %d\n", getpid(), message_calc.file_number);
            #endif
        }

        struct msgbuf2 message_insp1;//message structure for the inspector 1
        message_insp1.mtype = 1;//set the message type
        message_insp1.file_number = file_number;//set the file number
        message_insp1.time = time(NULL);//set the time of creating the file

        if (msgsnd(msg_insp1_id, &message_insp1, sizeof(message_insp1.file_number) + sizeof(message_insp1.time), 0) == -1) {//send the message to the inspector 1
            perror("Message send failed");
            exit(1);
        } else {
            #ifdef __DEBUG
            printf("Process:%d => Sent file number and time to queue: %d\n", getpid(), message_insp1.file_number);
            #endif
        }
        

        //sleep(5);
    }

    if (shmdt(file_counter) == -1) {//detach the shared memory
        perror("Shared memory detach failed");
    }

    return 0;
}



// Function to get a random float value between min and max
float getRandomFloat(int min, int max) //if any other file requiers this it can be moved to utils.c
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
    // code here
}
// Function to get a random integer value between min and max
int getRandomInt(int min, int max)
{

    return rand() % (max - min + 1) + min;
}
// Function to generate a CSV file with random values
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
    sprintf(sem_name, "/sem_%d", i);//set the name of the semaphore
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);//create the semaphore
    if (sem == SEM_FAILED) {
        perror("Semaphore creation failed");
        exit(1);
    }

     sprintf(filename, "%s/%d.csv", HOME_DIR, i);//set the name of the file

        FILE *file = fopen(filename, "w");//open the file
        if (!file) {
            perror("File creation failed");
            exit(1);
        }

        if (!(config.MIN_ROW ==-1 || config.MAX_ROW ==-1))//check if the min and max rows are set
        {
            Rows = getRandomInt(config.MIN_ROW, config.MAX_ROW);//get the random number of rows
           
        }

        if(!(config.MIN_COLUMN ==-1 || config.MAX_COLUMN ==-1))///check if the min and max columns are set
        {
            Cols = getRandomInt(config.MIN_COLUMN, config.MAX_COLUMN);//get the random number of columns
        }

        if(!(config.MIN_VALUE ==-1 || config.MAX_VALUE ==-1))//check if the min and max values are set
        {
            minValue = config.MIN_VALUE;//get the min value
            maxValue = config.MAX_VALUE;//get the max value
        }


 
        
        // Write the CSV file
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
        #ifdef __DEBUG
        printf("\033[0;32mProcess:%d => CSV file: %s generated successfully\033[0m\n",getpid(), filename);
        #endif
       




}
