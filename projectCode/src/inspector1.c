#include "inspectors.h"


Config config;
int sem_id, msg_id; // Global for cleanup during signal handling
int shm_data_id;
int sem_data_id;
MinHeap *heap;

// Signal handler for cleanup
void handle_signal(int sig) {
    printf("Cleaning up resources...\n");
    printf("Exiting gracefully on signal  %d.\n", sig);
    if (heap) free_min_heap(heap);

    exit(0);
}

// Function to process files from the heap
void process_files_from_heap(MinHeap *heap, const Config *config) {

    if (heap->size > 0) {
        time_t min_time = get_min_time(heap);
         //open the named semaphore that is created by the generator wich coresponed to the file number
 
        if (min_time + config->INSPECTOR1_THRESHOLD < time(NULL)) { // if the sem is not aquired , if the file is processed 
                                                                    // make sure to take the semaphor , if not taken , then remove it 
                                                                    // from the heap 

        char sem_name[150];
        sprintf(sem_name, "/sem_%d", heap->data[0].file_number);
        // open the named semaphore
        sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
        if (sem == SEM_FAILED) {
            perror("Semaphore creation failed");
            exit(1);
        }


        // aquire the semaphore if not aquired then remove the file from the heap and return 
        if (sem_trywait(sem) == -1) {
            remove_node(heap, heap->data[0].file_number);
            sem_close(sem);
            return;
        }                                                       


            HeapNode min_node = min_heap_extract(heap);

            char filename[100];
            sprintf(filename, "%d.csv", min_node.file_number);

            if (movefile(filename, homeDir, unprocessedDir) == -1) {
                perror("Error moving file");
                return;
            }

            semaphore_wait(sem_data_id);// wait for the semaphore to be available
            SharedData *shared_data = (SharedData *)shmat(shm_data_id, NULL, 0);
            if (shared_data == (void *)-1) {
                perror("Shared memory attach failed");
                return;
            }
            shared_data->unprocessed_csv++;
            shmdt(shared_data);
            semaphore_signal(sem_data_id);



            #ifdef __CLI
            printf("\033[0;33mInsperctor1: File %d moved to unprocessed directory\033[0m\n", min_node.file_number);
            fflush(stdout);
            #endif
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <config file> <semaphore key>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Load configuration
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }

    // Retrieve semaphore ID
    key_t sem_key = atoi(argv[2]);
    sem_id = semget(sem_key, 1, 0666);
    if (sem_id == -1) {
        perror("Semaphore retrieval failed");
        exit(EXIT_FAILURE);
    }

    int insp_number = atoi(argv[3]);


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

    // Get semaphore value
    int sem_value = semctl(sem_id, 0, GETVAL);
    if (sem_value == -1) {
        perror("Failed to get semaphore value");
        exit(EXIT_FAILURE);
    }

    // Initialize directories
    semaphore_wait(sem_id);
    if (!dirExists(unprocessedDir)) {
        if (createDirectory(unprocessedDir) == -1) {
            perror("Error creating unprocessed directory");
            semaphore_signal(sem_id);
            exit(EXIT_FAILURE);
        }
    }
    semaphore_signal(sem_id);

    // Retrieve message queue key from environment
    char *key_str = getenv("MSG_QUEUE_GI1_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int msg_key = atoi(key_str);

    // Get message queue ID
    msg_id = msgget(msg_key, 0666);
    if (msg_id == -1) {
        perror("Message queue retrieval failed");
        exit(EXIT_FAILURE);
    }
    
    //access the message queue between clac and insp1
    key_str = getenv("MSG_QUEUE_I1C_KEY");
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        return 1;
    }

    int key = atoi(key_str);
    int msgid_insp1 = msgget(key, 0666);

    if (msgid_insp1 == -1) {
        perror("Message queue retrieval failed");
        return 1;
    }

    // Initialize min-heap
    heap = create_min_heap();
    if (!heap) {
        fprintf(stderr, "Error initializing heap\n");
        exit(EXIT_FAILURE);
    }

    // Set up signal handling for graceful termination
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    struct msgbuf2 message;
    struct msgbuf calc_insp_msg ;
    // Main loop
    while (1) {
        usleep(20000);

        if (msgrcv(msg_id, &message, sizeof(message.file_number) + sizeof(message.time), 1, IPC_NOWAIT) == -1) {
            if (errno != ENOMSG) {
                perror("Message receive failed");
                break;
            }
        }else {

            if (find_node(heap, message.file_number) != -1) {

                #ifdef __DEBUG
                printf("\033[0;31mFile number already in heap %d\033[0m\n", message.file_number);
                fflush(stdout);
                #endif

                remove_node(heap, message.file_number);
            }else {
                min_heap_insert(heap, message.file_number, message.time);
            }

        }


        //recive message from the calculator if there is any remove the node with file number from the heap
        if (msgrcv(msgid_insp1, &calc_insp_msg, sizeof(calc_insp_msg.file_number), insp_number , IPC_NOWAIT) == -1) {
            if (errno != ENOMSG) {
                perror("Message receive failed");
                break;
            }
        }else { // if there is a message from the calculator remove the file from the heap
            if (find_node(heap, calc_insp_msg.file_number) == -1) {
                
                #ifdef __DEBUG
                printf("\033[0;31mFile not in the heap %d\033[0m\n", message.file_number);
                fflush(stdout);
                #endif
                min_heap_insert(heap, calc_insp_msg.file_number, time(NULL) + 365 * 24 * 60 * 60  );
            }else {
                remove_node(heap, calc_insp_msg.file_number);
            }
        }

        process_files_from_heap(heap, &config);

    }

    return 0;
}
