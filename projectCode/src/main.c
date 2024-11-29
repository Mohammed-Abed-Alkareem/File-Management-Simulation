#include "main.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

pid_t *generators_pid;
pid_t *calculators_pid;
pid_t *movers_pid;
pid_t *inspectors1_pid;
pid_t *inspectors2_pid;
pid_t *inspectors3_pid;

key_t shm_gen_key; // Shared memory key between generators to obtain file counter
key_t sem_gen_calc_key; // Semaphore key between generators and calculators
key_t msg_gen_calc_key; // Message queue key between generators and calculators
key_t msg_calc_mover_key; // Message queue key between calculators and movers

//message queue key between generator and inspector1
key_t msg_gen_insp1_key;
//message queue key between mover and inspector2
key_t msg_mover_insp2_key;

//meesaage queue key between inspector2 and inspector3
key_t msg_insp2_insp3_key;

key_t sem_inspector1_key ; // Semaphore key between generators and inspector1
key_t sem_inspector2_key; // Semaphore key between generators and inspector2 
key_t sem_mover_key; // Semaphore key between generators and mover

int shm_id = -1, sem_id = -1, msg_gen_calc_id = -1, msg_calc_mover_id = -1, msg_gen_insp1_id = -1, msg_mover_insp2_id = -1, msg_insp2_insp3_id = -1; 

int sem_inspector1_id = -1 , sem_inspector2_id = -1 , sem_mover_id = -1;

void cleanup() {
    if (shm_id != -1) shmctl(shm_id, IPC_RMID, NULL);
    if (sem_id != -1) semctl(sem_id, 0, IPC_RMID);
    if (msg_gen_calc_id != -1) msgctl(msg_gen_calc_id, IPC_RMID, NULL);
    if (msg_calc_mover_id != -1) msgctl(msg_calc_mover_id, IPC_RMID, NULL);
    if (sem_inspector1_id != -1) semctl(sem_inspector1_id, 0, IPC_RMID);
    if (sem_inspector2_id != -1) semctl(sem_inspector2_id, 0, IPC_RMID);
    if (sem_mover_id != -1) semctl(sem_mover_id, 0, IPC_RMID);

    free(generators_pid);
    free(calculators_pid);
    free(movers_pid);
    free(inspectors1_pid);
    free(inspectors2_pid);
    free(inspectors3_pid);

    printf("Resources cleaned up.\n");
}

void handle_usr1(int signal) {
    printf("Received SIGUSR1 signal.%d\n", signal);
    printf("Generator process Created Home Dir.\n");
    
}

void handle_signal(int signal) {
    cleanup();
    printf("Exiting gracefully on signal %d.\n", signal);
    exit(0);
}


int main(int argc, char *argv[]) {
    Config config;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }

    if(!dirExists(filesDir)) {
        createDirectory(filesDir);
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGUSR1, handle_usr1);

    // Create shared memory key
    shm_gen_key = key_generator('A');

    // num of file generated .
    shm_id = shmget(shm_gen_key, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Shared memory creation failed");
        cleanup();
        exit(1);
    }

    int *file_counter = (int *)shmat(shm_id, NULL, 0);
    if (file_counter == (void *)-1) {
        perror("Shared memory attach failed");
        cleanup();
        exit(1);
    }
    *file_counter = 0; // Initialize counter
    shmdt(file_counter);







    // Create semaphore key
    sem_gen_calc_key = key_generator('B');

    // Create semaphore key for inspector1
    key_t sem_gen_insp1_key = key_generator('C');

    // Create semaphore key for inspector2
    key_t sem_gen_insp2_key = key_generator('D');

    // Create semaphore key for mover
    key_t sem_gen_mover_key = key_generator('E');


    // Define the semaphore structure
        struct sembuf {
        unsigned short sem_num;  // Semaphore number in the set
        short sem_op;            // Semaphore operation
        short sem_flg;           // Operation flags
        };
    
    // Create semaphore for generator
    sem_id = semget(sem_gen_calc_key, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Semaphore creation failed");
        cleanup();
        exit(1);
    }

    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("Semaphore initialization failed");
        cleanup();
        exit(1);
    }
    printf("Semaphore initialized with key: %d and id: %d.\n", sem_gen_calc_key, sem_id);

    // Create semaphore for inspector1
    sem_inspector1_id = semget(sem_gen_insp1_key, 1, IPC_CREAT | 0666);
    if (sem_inspector1_id == -1) {
        perror("Semaphore creation failed");
        cleanup();
        exit(1);
    }
    if (semctl(sem_inspector1_id, 0, SETVAL, 1) == -1) {
        perror("Semaphore initialization failed");
        cleanup();
        exit(1);
    }
    // Create semaphore for inspector2
    sem_inspector2_id = semget(sem_gen_insp2_key, 1, IPC_CREAT | 0666);
    if (sem_inspector2_id == -1) {
        perror("Semaphore creation failed");
        cleanup();
        exit(1);
    }
    if (semctl(sem_inspector2_id, 0, SETVAL, 1) == -1) {
        perror("Semaphore initialization failed");
        cleanup();
        exit(1);
    }
    // Create semaphore for mover
    sem_mover_id = semget(sem_gen_mover_key, 1, IPC_CREAT | 0666);
    if (sem_mover_id == -1) {
        perror("Semaphore creation failed");
        cleanup();
        exit(1);
    }
    if (semctl(sem_mover_id, 0, SETVAL, 1) == -1) {
        perror("Semaphore initialization failed");
        cleanup();
        exit(1);
    }


    

    // Create message queue key
    msg_gen_calc_key = key_generator('F');

    msg_gen_calc_id = msgget(msg_gen_calc_key, IPC_CREAT | 0666);
    if (msg_gen_calc_id == -1) {
        perror("Message queue creation failed");
        cleanup();
        exit(1);
    }

    // Create message queue key between generator and inspector1
    msg_gen_insp1_key = key_generator('G');
    
    msg_gen_insp1_id = msgget(msg_gen_insp1_key, IPC_CREAT | 0666);
    if (msg_gen_insp1_id == -1) {
        perror("Message queue creation failed");
        cleanup();
        exit(1);
    }

    

    char shm_key_str[20], sem_key_str[20], msg_gen_calc_key_str[20], msg_gen_insp1_key_str[20];

    snprintf(shm_key_str, sizeof(shm_key_str), "%d", shm_gen_key);
    
    snprintf(msg_gen_calc_key_str, sizeof(msg_gen_calc_key_str), "%d", msg_gen_calc_key);


    
    setenv("MSG_QUEUE_GC_KEY", msg_gen_calc_key_str, 1);

    snprintf(msg_gen_insp1_key_str, sizeof(msg_gen_insp1_key_str), "%d", msg_gen_insp1_key);
    setenv("MSG_QUEUE_GI1_KEY", msg_gen_insp1_key_str, 1);


    snprintf(sem_key_str, sizeof(sem_key_str), "%d", sem_gen_calc_key);
   
    char sem_inspector1_key_str[20] , sem_inspector2_key_str[20] , sem_mover_key_str[20];
    snprintf(sem_inspector1_key_str, sizeof(sem_inspector1_key_str), "%d", sem_gen_insp1_key);
    snprintf(sem_inspector2_key_str, sizeof(sem_inspector2_key_str), "%d", sem_gen_insp2_key);
    snprintf(sem_mover_key_str, sizeof(sem_mover_key_str), "%d", sem_gen_mover_key);

    // Start child processes
    generators_pid = (pid_t *)malloc(config.NUM_GENERATORS * sizeof(pid_t));
    for (int i = 0; i < config.NUM_GENERATORS; i++) {
        if ((generators_pid[i] = fork()) == 0) {
            execl("./bin/generator", "generator", argv[1], shm_key_str, sem_key_str, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/generator", "generator", argv[1], shm_key_str, sem_key_str, NULL);
            perror("Generator process failed");
            exit(1);
        }
    }

    pause();//wait for generator to create home dir

    //create A message queue for the calculators and movers
    key_t msg_calc_mover_key = ftok(".", 'C');
    if (msg_calc_mover_key == -1) {
        perror("Message queue key generation failed");
        cleanup();
        exit(1);
    }

     msg_calc_mover_id = msgget(msg_calc_mover_key, IPC_CREAT | 0666);
    if (msg_calc_mover_id == -1) {
        perror("Message queue creation failed");
        cleanup();
        exit(1);
    }

    char msg_calc_mover_key_str[20];
    snprintf(msg_calc_mover_key_str, sizeof(msg_calc_mover_key_str), "%d", msg_calc_mover_key);
    setenv("MSG_QUEUE_CM_KEY", msg_calc_mover_key_str, 1);



    calculators_pid = (pid_t *)malloc(config.NUM_CALCULATORS * sizeof(pid_t));
    for (int i = 0; i < config.NUM_CALCULATORS; i++) {
        if ((calculators_pid[i] = fork()) == 0) {
            execl("./bin/calculator", "calculator", argv[1], NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/calculator", "calculator", argv[1], NULL);
            perror("Calculator process failed");
            exit(1);
        }
    }

    //create message queue key between mover and inspector2
    msg_mover_insp2_key = key_generator('H');

    msg_mover_insp2_id = msgget(msg_mover_insp2_key, IPC_CREAT | 0666);
    if (msg_mover_insp2_id == -1) {
        perror("Message queue creation failed");
        cleanup();
        exit(1);
    }


    // create message queue key between inspector2 and inspector3
    msg_insp2_insp3_key = key_generator('I');

    msg_insp2_insp3_id = msgget(msg_insp2_insp3_key, IPC_CREAT | 0666);
    if (msg_insp2_insp3_id == -1) {
        perror("Message queue creation failed");
        cleanup();
        exit(1);
    }


    char msg_mover_insp2_key_str[20];
    snprintf(msg_mover_insp2_key_str, sizeof(msg_mover_insp2_key_str), "%d", msg_mover_insp2_key);

    setenv("MSG_QUEUE_MI2_KEY", msg_mover_insp2_key_str, 1);

    

    movers_pid = (pid_t *)malloc(config.NUM_MOVERS * sizeof(pid_t));
    for (int i = 0; i < config.NUM_MOVERS; i++) {
        if ((movers_pid[i] = fork()) == 0) {
            execl("./bin/mover", "mover", argv[1], sem_mover_key_str, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/mover", "mover", argv[1],sem_mover_key_str, NULL);
            perror("Mover process failed");
            exit(1);
        }
    }

    inspectors1_pid = (pid_t *)malloc(config.NUM_INSPECTOR1 * sizeof(pid_t));
    for (int i = 0; i < config.NUM_INSPECTOR1; i++) {
        if ((inspectors1_pid[i] = fork()) == 0) {
            execl("./bin/inspector1", "inspector1", argv[1],sem_inspector1_key_str, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/inspector1", "inspector1", argv[1],sem_inspector1_key_str, NULL);
            perror("Inspector1 process failed");
            exit(1);
        }
    }

    inspectors2_pid = (pid_t *)malloc(config.NUM_INSPECTOR2 * sizeof(pid_t));
    for (int i = 0; i < config.NUM_INSPECTOR2; i++) {
        if ((inspectors2_pid[i] = fork()) == 0) {
            execl("./bin/inspector2", "inspector2", argv[1],sem_inspector2_key_str, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/inspector2", "inspector2", argv[1],sem_inspector2_key_str, NULL);
            perror("Inspector2 process failed");
            exit(1);
        }
    }

    inspectors3_pid = (pid_t *)malloc(config.NUM_INSPECTOR3 * sizeof(pid_t));
    for (int i = 0; i < config.NUM_INSPECTOR3; i++) {
        if ((inspectors3_pid[i] = fork()) == 0) {
            execl("./bin/inspector3", "inspector3", argv[1], NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/inspector3", "inspector3", argv[1], NULL);
            perror("Inspector3 process failed");
            exit(1);
        }
    }

    // Wait for all child processes to finish
    int total_processes = config.NUM_GENERATORS + config.NUM_CALCULATORS + config.NUM_MOVERS +
                          config.NUM_INSPECTOR1 + config.NUM_INSPECTOR2 + config.NUM_INSPECTOR3;
    for (int i = 0; i < total_processes; i++) {
        wait(NULL);
    }

    cleanup();
    return 0;
}



key_t key_generator(char letter){

    //char letter = rand() % 26 + 'A' ;

    key_t msg_gen_calc_key = ftok(".", letter) ;

    return msg_gen_calc_key ;
}