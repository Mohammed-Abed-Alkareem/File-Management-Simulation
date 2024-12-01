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

// --- Globals ---
pid_t *generators_pid;
pid_t *calculators_pid;
pid_t *movers_pid;
pid_t *inspectors1_pid;
pid_t *inspectors2_pid;
pid_t *inspectors3_pid;



// --- IPC keys ---

// ----- Shared memory -----
key_t shm_gen_key; // Shared memory key between generators to obtain file counter
key_t shm_data_key; // Shared memory key between all processes to share data

// --- Message queues ---
key_t msg_gen_calc_key; // Message queue key between generators and calculators
key_t msg_calc_mover_key; // Message queue key between calculators and movers
key_t msg_gen_insp1_key;//message queue key between generator and inspector1
key_t msg_insp1_calc_key;//message queue key between inspector1 and calculator
key_t msg_mover_insp2_key;//message queue key between mover and inspector2
key_t msg_insp2_insp3_key;//meesaage queue key between inspector2 and inspector3

// --- Semaphore keys ---
key_t sem_inspector1_key ; // Semaphore key between inspector1
key_t sem_inspector2_key; // Semaphore key between inspector2 
key_t sem_mover_key; // Semaphore key between mover
key_t sem_gen_key; // Semaphore key between generator
key_t sem_data_key; // Semaphore key between all processes



// --- IPC ids ---
int shm_id = -1, sem_id = -1, msg_gen_calc_id = -1, msg_calc_mover_id = -1, msg_gen_insp1_id = -1, msg_mover_insp2_id = -1, msg_insp2_insp3_id = -1; 
int sem_inspector1_id = -1 , sem_inspector2_id = -1 , sem_mover_id = -1;
int msg_insp1_calc_id = -1;
int shm_data_id = -1;
int sem_data_id = -1;


// --- Main ---
int main(int argc, char *argv[]) {
    Config config;

    // validate arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Load config file
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }

    // Create files directory
    if(!dirExists(filesDir)) {
        createDirectory(filesDir);
    }

    // Register signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGUSR1, handle_usr1);


// ============shared memories==================

    // Shmem Between Generators
    shm_gen_key = key_generator('A');

    // num of file generated .
    shm_id = shmget(shm_gen_key, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Shared memory creation failed");
        cleanup();
        exit(1);
    }

    // Attach shared memory to file_counter
    int *file_counter = (int *)shmat(shm_id, NULL, 0);
    if (file_counter == (void *)-1) {
        perror("Shared memory attach failed");
        cleanup();
        exit(1);
    }
    *file_counter = 0; // Initialize counter
    shmdt(file_counter);

    // Shared memory for data
    shm_data_key = ftok(".", 'Z');

    shm_data_id = shmget(shm_data_key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_data_id == -1) {
        perror("Shared memory creation failed");
        cleanup();
        exit(1);
    }

    // Attach shared memory to shared_data
    SharedData *shared_data = (SharedData *)shmat(shm_data_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Shared memory attach failed");
        cleanup();
        exit(1);
    }

    shared_data->total_csv_generated = 0;
    shared_data->total_csv_calculated = 0;
    shared_data->unprocessed_csv = 0;
    shared_data->files_moved_to_backup = 0;
    shared_data->files_deleted = 0;



// ============Semaphores==================

    // Create semaphore key for generator
    sem_gen_key = key_generator('B');

    sem_id = semget(sem_gen_key, 1, IPC_CREAT | 0666);
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
    
    // Create semaphore key for shared data
    sem_data_key = ftok(".", 'Y');

    sem_data_id = semget(sem_data_key, 1, IPC_CREAT | 0666);
    if (sem_data_id == -1) {
        perror("Semaphore creation failed");
        cleanup();
        exit(1);
    }

    if (semctl(sem_data_id, 0, SETVAL, 1) == -1) {
        perror("Semaphore initialization failed");
        cleanup();
        exit(1);
    }


    // Create semaphore key for inspector1
    sem_inspector1_key = key_generator('C');

    sem_inspector1_id = semget(sem_inspector1_key, 1, IPC_CREAT | 0666);
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

    // Create semaphore key for inspector2
    sem_inspector2_key = key_generator('D');

    sem_inspector2_id = semget(sem_inspector2_key, 1, IPC_CREAT | 0666);
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

    // Create semaphore key for mover
    sem_mover_key = key_generator('E');
 
    sem_mover_id = semget(sem_mover_key, 1, IPC_CREAT | 0666);
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


// ============Message Queues==================

    // Create message queue key between generator and calculator
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


    //create A message queue for the calculators and movers
    msg_calc_mover_key = ftok(".", 'C');

     msg_calc_mover_id = msgget(msg_calc_mover_key, IPC_CREAT | 0666);
    if (msg_calc_mover_id == -1) {
        perror("Message queue creation failed");
        cleanup();
        exit(1);
    }


    // create a message queue for the inspector 1 and calculator 
    msg_insp1_calc_key = ftok(".", 'J');

    msg_insp1_calc_id = msgget(msg_insp1_calc_key, IPC_CREAT | 0666);
    if (msg_insp1_calc_id == -1) {
        perror("Message queue creation failed");
        cleanup();
        exit(1);
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


// ============converting keys to string==================
    char shm_gen_key_str[20];
    
    char sem_gen_key_str[20];
    char sem_inspector1_key_str[20]; 
    char sem_inspector2_key_str[20] ;
    char sem_mover_key_str[20];

    char msg_gen_calc_key_str[20];
    char msg_gen_insp1_key_str[20];
    char msg_calc_mover_key_str[20];
    char msg_insp1_calc_key_str[20];
    char msg_insp2_insp3_key_str[20];
    char msg_mover_insp2_key_str[20];
    char shm_data_key_str[20];
    char sem_data_key_str[20];


    snprintf(shm_gen_key_str, sizeof(shm_gen_key_str), "%d", shm_gen_key);
    snprintf(shm_data_key_str, sizeof(shm_data_key_str), "%d", shm_data_key);

    snprintf(sem_gen_key_str, sizeof(sem_gen_key_str), "%d", sem_gen_key);
    snprintf(sem_inspector1_key_str, sizeof(sem_inspector1_key_str), "%d", sem_inspector1_key);
    snprintf(sem_inspector2_key_str, sizeof(sem_inspector2_key_str), "%d", sem_inspector2_key);
    snprintf(sem_mover_key_str, sizeof(sem_mover_key_str), "%d", sem_mover_key);
    snprintf(sem_data_key_str, sizeof(sem_data_key_str), "%d", sem_data_key);
    
    snprintf(msg_gen_calc_key_str, sizeof(msg_gen_calc_key_str), "%d", msg_gen_calc_key);
    snprintf(msg_gen_insp1_key_str, sizeof(msg_gen_insp1_key_str), "%d", msg_gen_insp1_key);
    snprintf(msg_calc_mover_key_str, sizeof(msg_calc_mover_key_str), "%d", msg_calc_mover_key);
    snprintf(msg_insp1_calc_key_str, sizeof(msg_insp1_calc_key_str), "%d", msg_insp1_calc_key);
    snprintf(msg_insp2_insp3_key_str, sizeof(msg_insp2_insp3_key_str), "%d", msg_insp2_insp3_key);
    snprintf(msg_mover_insp2_key_str, sizeof(msg_mover_insp2_key_str), "%d", msg_mover_insp2_key);

  

    // Set environment variables
    setenv("MSG_QUEUE_GC_KEY", msg_gen_calc_key_str, 1);
    setenv("MSG_QUEUE_GI1_KEY", msg_gen_insp1_key_str, 1);
    setenv("MSG_QUEUE_CM_KEY", msg_calc_mover_key_str, 1);
    setenv("MSG_QUEUE_I1C_KEY", msg_insp1_calc_key_str, 1);
    setenv("MSG_QUEUE_I2I3_KEY", msg_insp2_insp3_key_str, 1);
    setenv("MSG_QUEUE_MI2_KEY", msg_mover_insp2_key_str, 1);
    setenv("SHM_DATA_KEY", shm_data_key_str, 1);
    setenv("SEM_GEN_KEY", sem_gen_key_str, 1);


    // create the log dir and file 


// ============Forking Processes==================

    // Fork generator processes
    generators_pid = (pid_t *)malloc(config.NUM_GENERATORS * sizeof(pid_t));
    for (int i = 0; i < config.NUM_GENERATORS; i++) {
        if ((generators_pid[i] = fork()) == 0) {
            execl("./bin/generator", "generator", argv[1], shm_gen_key_str, sem_gen_key_str, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/generator", "generator", argv[1], shm_gen_key_str, sem_gen_key_str, NULL);
            perror("Generator process failed");
            exit(1);
        }
    }
    //wait for generator to create home dir
    pause();

    
    if(!dirExists(logDir)) {
        createDirectory(logDir);
    }

   
    // Fork calculator processes
    calculators_pid = (pid_t *)malloc(config.NUM_CALCULATORS * sizeof(pid_t));
    for (int i = 0; i < config.NUM_CALCULATORS; i++) {
        if ((calculators_pid[i] = fork()) == 0) {
            execl("./bin/calculator", "calculator", argv[1], NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/calculator", "calculator", argv[1], NULL);
            perror("Calculator process failed");
            exit(1);
        }
    }


    // Fork mover processes
    movers_pid = (pid_t *)malloc(config.NUM_MOVERS * sizeof(pid_t));
    for (int i = 0; i < config.NUM_MOVERS; i++) {
        if ((movers_pid[i] = fork()) == 0) {
            execl("./bin/mover", "mover", argv[1], sem_mover_key_str, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/mover", "mover", argv[1],sem_mover_key_str, NULL);
            perror("Mover process failed");
            exit(1);
        }
    }


    // Fork inspector1 processes
    inspectors1_pid = (pid_t *)malloc(config.NUM_INSPECTOR1 * sizeof(pid_t));
    char inspcetor_Number[20];
    for (int i = 0; i < config.NUM_INSPECTOR1; i++) {
        if ((inspectors1_pid[i] = fork()) == 0) {
            sprintf(inspcetor_Number, "%d", i + 1);
            execl("./bin/inspector1", "inspector1", argv[1],sem_inspector1_key_str ,inspcetor_Number, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/inspector1", "inspector1", argv[1],sem_inspector1_key_str, NULL);
            perror("Inspector1 process failed");
            exit(1);
        }
    }


    // Fork inspector2 processes
    inspectors2_pid = (pid_t *)malloc(config.NUM_INSPECTOR2 * sizeof(pid_t));
    for (int i = 0; i < config.NUM_INSPECTOR2; i++) {
        if ((inspectors2_pid[i] = fork()) == 0) {
            execl("./bin/inspector2", "inspector2", argv[1],sem_inspector2_key_str, NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/inspector2", "inspector2", argv[1],sem_inspector2_key_str, NULL);
            perror("Inspector2 process failed");
            exit(1);
        }
    }

    // Fork inspector3 processes
    inspectors3_pid = (pid_t *)malloc(config.NUM_INSPECTOR3 * sizeof(pid_t));
    for (int i = 0; i < config.NUM_INSPECTOR3; i++) {
        if ((inspectors3_pid[i] = fork()) == 0) {
            execl("./bin/inspector3", "inspector3", argv[1] ,  NULL);
            // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/inspector3", "inspector3", argv[1], NULL);
            perror("Inspector3 process failed");
            exit(1);
        }
    }

    //fork the GUI process
    pid_t gui_id;
    if((gui_id=fork()) == 0){
        execl("./bin/gui", "gui", argv[1], NULL);
        // execl("/home/adduser/ENCS4330/Projects/Project2/File-Management-Simulation/projectCode/bin/gui", "gui", argv[1], NULL);
        perror("GUI process failed");
        exit(1);
    }

    while(1){ //always check for the shared data to exit the program
        if(shared_data->files_deleted >= config.DELETED_THRESHOLD){
            
            sendKillSignal(generators_pid, config.NUM_GENERATORS);
            sendKillSignal(calculators_pid, config.NUM_CALCULATORS);
            sendKillSignal(movers_pid, config.NUM_MOVERS);
            sendKillSignal(inspectors1_pid, config.NUM_INSPECTOR1);
            sendKillSignal(inspectors2_pid, config.NUM_INSPECTOR2);
            sendKillSignal(inspectors3_pid, config.NUM_INSPECTOR3);
            kill(gui_id, SIGINT);

            break;
        }
        if(shared_data->files_moved_to_backup >= config.BACKUP_THRESHOLD){
            sendKillSignal(generators_pid, config.NUM_GENERATORS);
            sendKillSignal(calculators_pid, config.NUM_CALCULATORS);
            sendKillSignal(movers_pid, config.NUM_MOVERS);
            sendKillSignal(inspectors1_pid, config.NUM_INSPECTOR1);
            sendKillSignal(inspectors2_pid, config.NUM_INSPECTOR2);
            sendKillSignal(inspectors3_pid, config.NUM_INSPECTOR3);
            kill(gui_id, SIGINT);
            break;
        }
        if(shared_data->total_csv_calculated >= config.MAX_FILES){
            sendKillSignal(generators_pid, config.NUM_GENERATORS);
            sendKillSignal(calculators_pid, config.NUM_CALCULATORS);
            sendKillSignal(movers_pid, config.NUM_MOVERS);
            sendKillSignal(inspectors1_pid, config.NUM_INSPECTOR1);
            sendKillSignal(inspectors2_pid, config.NUM_INSPECTOR2);
            sendKillSignal(inspectors3_pid, config.NUM_INSPECTOR3);
            kill(gui_id, SIGINT);
            break;
        }
        if(shared_data->unprocessed_csv >= config.UNPROCESSED_THRESHOLD){
            sendKillSignal(generators_pid, config.NUM_GENERATORS);
            sendKillSignal(calculators_pid, config.NUM_CALCULATORS);
            sendKillSignal(movers_pid, config.NUM_MOVERS);
            sendKillSignal(inspectors1_pid, config.NUM_INSPECTOR1);
            sendKillSignal(inspectors2_pid, config.NUM_INSPECTOR2);
            sendKillSignal(inspectors3_pid, config.NUM_INSPECTOR3);
            kill(gui_id, SIGINT);
            break;
        }
    }



    // Wait for all child processes to finish
    int total_processes = config.NUM_GENERATORS + config.NUM_CALCULATORS + config.NUM_MOVERS +
                          config.NUM_INSPECTOR1 + config.NUM_INSPECTOR2 + config.NUM_INSPECTOR3+1;
    for (int i = 0; i < total_processes; i++) {
        wait(NULL);
    }

    cleanup();
    return 0;
}

// --- Functions ---

// cleanup function to remove all allocated resources
void cleanup() {
    if (shm_id != -1) shmctl(shm_id, IPC_RMID, NULL);
    if (sem_id != -1) semctl(sem_id, 0, IPC_RMID);
    if (msg_gen_calc_id != -1) msgctl(msg_gen_calc_id, IPC_RMID, NULL);
    if (msg_calc_mover_id != -1) msgctl(msg_calc_mover_id, IPC_RMID, NULL);
    if (sem_inspector1_id != -1) semctl(sem_inspector1_id, 0, IPC_RMID);
    if (sem_inspector2_id != -1) semctl(sem_inspector2_id, 0, IPC_RMID);
    if (sem_mover_id != -1) semctl(sem_mover_id, 0, IPC_RMID);
    if (msg_gen_insp1_id != -1) msgctl(msg_gen_insp1_id, IPC_RMID, NULL);
    if (msg_insp1_calc_id != -1) msgctl(msg_insp1_calc_id, IPC_RMID, NULL);
    if (msg_mover_insp2_id != -1) msgctl(msg_mover_insp2_id, IPC_RMID, NULL);
    if (msg_insp2_insp3_id != -1) msgctl(msg_insp2_insp3_id, IPC_RMID, NULL);
    if (shm_data_id != -1) shmctl(shm_data_id, IPC_RMID, NULL);
    if (sem_data_id != -1) semctl(sem_data_id, 0, IPC_RMID);
    
    
    free(generators_pid);
    free(calculators_pid);
    free(movers_pid);
    free(inspectors1_pid);
    free(inspectors2_pid);
    free(inspectors3_pid);

    printf("Resources cleaned up.\n");
}

// Signal handler for SIGUSR1 
void handle_usr1(int signal) {
    printf("Received SIGUSR1 signal.%d\n", signal);
    printf("Generator process Created Home Dir.\n");
    
}

// Signal handler to cleanup resources and exit gracefully
void handle_signal(int signal) {
    cleanup();
    printf("Exiting gracefully on signal %d.\n", signal);
    exit(0);
}


key_t key_generator(char letter){

    key_t key = ftok(".", letter) ;
    if(key == -1){
        perror("Key generation failed");
        exit(1);
    }

    return key ;
}

void sendKillSignal(pid_t *pid, int num){
    for(int i = 0; i < num; i++){
        kill(pid[i], SIGINT);
    }
}