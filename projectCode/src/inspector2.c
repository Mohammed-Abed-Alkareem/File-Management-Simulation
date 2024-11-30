#include "inspectors.h"


Config config;
int sem_id, msg_id; // Global for cleanup during signal handling
MinHeap *heap;
int msg_id_insp2_insp3;



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
        if (min_time + config->INSPECTOR2_THRESHOLD < time(NULL)) {
            HeapNode min_node = min_heap_extract(heap);

            char filename[20];
            sprintf(filename, "%d.csv", min_node.file_number);


            if (movefile(filename,processesdDir, backupDir ) == -1) {
                perror("Error moving file");
                return;
            }

            // send a message to inspector 3
            struct msgbuf2 message_insp2_insp3;
            message_insp2_insp3.mtype = 1;
            message_insp2_insp3.file_number = min_node.file_number;
            message_insp2_insp3.time = time(NULL);

            if (msgsnd(msg_id_insp2_insp3, &message_insp2_insp3, sizeof(message_insp2_insp3.file_number) + sizeof(message_insp2_insp3.time), 0) == -1) {
                perror("Message send failed");
                exit(1);
            } else {
                printf("Inspector2 %d sent file number and time to queue: %d\n", getpid(), min_node.file_number);
            }

            printf("Mover %d moved file number: %d\n", getpid(), min_node.file_number);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
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

    // Get semaphore value
    int sem_value = semctl(sem_id, 0, GETVAL);
    if (sem_value == -1) {
        perror("Failed to get semaphore value");
        exit(EXIT_FAILURE);
    }

    // Initialize directories
    semaphore_wait(sem_id);
    if (!dirExists(backupDir)) {
        if (createDirectory(backupDir) == -1) {
            perror("Error creating backup directory");
            semaphore_signal(sem_id);
            exit(EXIT_FAILURE);
        }
    }
    semaphore_signal(sem_id);

    // Retrieve message queue key from environment
    char *key_str = getenv("MSG_QUEUE_MI2_KEY");
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


    // insp2 , inpsp3 message queue
    char *key_str_insp2_insp3 = getenv("MSG_QUEUE_I2I3_KEY");
    if (key_str_insp2_insp3 == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int msg_key_insp2_insp3 = atoi(key_str_insp2_insp3);

    // Get message queue ID
    msg_id_insp2_insp3 = msgget(msg_key_insp2_insp3, 0666);
    if (msg_id_insp2_insp3 == -1) {
        perror("Message queue retrieval failed");
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
                // No message in the queue, process files from the heap
                process_files_from_heap(heap, &config);
                //sleep(1);
                continue;
            } else {
                perror("Message receive failed");
                break;
            }
        }

        // Log message details
        printf("Inspector1: File Number: %d, Creation Time: %ld\n",
               message.file_number, (long)message.time);

        // Insert received message into the heap
        min_heap_insert(heap, message.file_number, message.time);

        // Process files from the heap
        process_files_from_heap(heap, &config);
    }

    // Cleanup resources
    free_min_heap(heap);
    return 0;
}
