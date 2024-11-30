#include "inspectors.h"


Config config;// Global variable for the configuration
int sem_id, msg_id; // Global for cleanup during signal handling
MinHeap *heap;// Global for cleanup during signal handling

// Signal handler for cleanup
void handle_signal(int sig) {
    #ifdef __CLI
    printf("Cleaning up resources...\n");
    printf("Exiting gracefully on signal %d.\n", sig);
    #endif
    if (heap) free_min_heap(heap);
    exit(0);
}

// Function to process files from the heap
void process_files_from_heap(MinHeap *heap, const Config *config) {
    if (heap->size > 0) {//check if the heap is not empty
        time_t min_time = get_min_time(heap);//get the time of the first node
        if (min_time + config->INSPECTOR3_THRESHOLD < time(NULL)) {//check if the time of the first node is greater than the threshold
            HeapNode min_node = min_heap_extract(heap);//extract the minimum node from the heap

            char filename[20];
            sprintf(filename, "%d.csv", min_node.file_number);//get the file name

            if ( deleteFile(filename, backupDir) == -1) {//delete the file from the backup directory
                perror("Error moving file");
                return;
            }
            #ifdef __CLI
            printf("delete %d moved file number: %d\n", getpid(), min_node.file_number);
            #endif
        }
    }
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
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
    char *key_str = getenv("MSG_QUEUE_I2I3_KEY");//get the key of the message queue between inspector2 and inspector3
    if (key_str == NULL) {
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int msg_key = atoi(key_str);//convert the key to integer

    // Get message queue ID
    msg_id = msgget(msg_key, 0666);//get the message queue id
    if (msg_id == -1) {
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
    signal(SIGINT, handle_signal);//handle the signal SIGINT
    signal(SIGTERM, handle_signal);//handle the signal SIGTERM

    struct msgbuf2 message;//message structure for the inspector 1
    #ifdef __CLI
    printf("\n\nInspector3: Started\n");
    #endif
    // Main loop
    while (1) {
        usleep(20000);//sleep for 20 ms
        if (msgrcv(msg_id, &message, sizeof(message.file_number) + sizeof(message.time), 1, IPC_NOWAIT) == -1) {//receive the message from the inspector 2 with type 1
            if (errno == ENOMSG) {
                // No message in the queue, process files from the heap
                process_files_from_heap(heap, &config);
                continue;
            } else {//check if the message receive failed
                perror("Message receive failed");
                break;
            }
        }
        #ifdef __CLI
        printf("\n\nInspector3: Received message\n");
        // Log message details
        printf("Inspector3: File Number: %d, Creation Time: %ld\n",
               message.file_number, (long)message.time);
        #endif

        // Insert received message into the heap
        min_heap_insert(heap, message.file_number, message.time);

        // Process files from the heap
        process_files_from_heap(heap, &config);
    }

    // Cleanup resources
    free_min_heap(heap);
    return 0;
}
