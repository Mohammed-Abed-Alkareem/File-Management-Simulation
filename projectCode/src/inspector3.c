#include "inspectors.h"


Config config;
int sem_id, msg_id; // Global for cleanup during signal handling
MinHeap *heap;
int shm_data_id;
int sem_data_id;


// Signal handler for cleanup
void handle_signal(int sig) {
    printf("Cleaning up resources...\n");
    printf("Exiting gracefully on signal %d.\n", sig);
    if (heap) free_min_heap(heap);
    exit(0);
}

// Function to process files from the heap
void process_files_from_heap(MinHeap *heap, const Config *config) {
    if (heap->size > 0) {
        time_t min_time = get_min_time(heap);
        if (min_time + config->INSPECTOR3_THRESHOLD < time(NULL)) {
            HeapNode min_node = min_heap_extract(heap);

            char filename[20];
            sprintf(filename, "%d.csv", min_node.file_number);

            if ( deleteFile(filename, backupDir) == -1) {
                perror("Error moving file");
                return;
            }

            
            semaphore_wait(sem_data_id);// wait for the semaphore to be available
            SharedData *shared_data = (SharedData *)shmat(shm_data_id, NULL, 0);
            if (shared_data == (void *)-1) {
                perror("Shared memory attach failed");
                return;
            }
            shared_data->files_deleted++;
            shmdt(shared_data);
            semaphore_signal(sem_data_id);

            #ifdef __CLI
            printf("\033[0;31mInspector3 %d deleted file number: %d\033[0m\n", getpid(), min_node.file_number);
            fflush(stdout);
            #endif
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config file> <semaphore key>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Load configuration
    if (load_config(argv[1], &config) == -1) {
        fprintf(stderr, "Error loading config file\n");
        exit(EXIT_FAILURE);
    }


    // Retrieve message queue key from environment
    char *key_str = getenv("MSG_QUEUE_I2I3_KEY");
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

    // Main loop
    while (1) {
        usleep(20000);
        if (msgrcv(msg_id, &message, sizeof(message.file_number) + sizeof(message.time), 1, IPC_NOWAIT) == -1) {
            if (errno == ENOMSG) {

                process_files_from_heap(heap, &config);

                continue;
            } else {

                perror("Message receive failed");
                break;
            }
        }
        // Insert received message into the heap
        min_heap_insert(heap, message.file_number, message.time);

        // Process files from the heap
        process_files_from_heap(heap, &config);
    }

    // Cleanup resources
    free_min_heap(heap);
    return 0;
}
