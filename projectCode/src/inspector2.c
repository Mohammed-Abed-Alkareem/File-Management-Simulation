#include "inspectors.h"


Config config;// Global variable for the configuration
int sem_id, msg_id; // Global for cleanup during signal handling
MinHeap *heap;// Global for cleanup during signal handling
int msg_id_insp2_insp3;// Global message queue id for inspector2 and inspector3



// Signal handler for cleanup
void handle_signal(int sig) {
    #ifdef __DEBUG
    printf("Cleaning up resources...\n");
    printf("Exiting gracefully on signal %d.\n", sig);
    #endif
    if (heap) free_min_heap(heap);//free the heap
    exit(0);
}

// Function to process files from the heap
void process_files_from_heap(MinHeap *heap, const Config *config) {
    if (heap->size > 0) {//check if the heap is not empty
        time_t min_time = get_min_time(heap);//get the time of the first node
        if (min_time + config->INSPECTOR2_THRESHOLD < time(NULL)) {//check if the time of the first node is greater than the threshold
            HeapNode min_node = min_heap_extract(heap);

            char filename[20];
            sprintf(filename, "%d.csv", min_node.file_number);//    get the file name


            if (movefile(filename,processesdDir, backupDir ) == -1) {//move the file from the processed directory to the backup directory
                perror("Error moving file");
                return;
            }

            // send a message to inspector 3
            struct msgbuf2 message_insp2_insp3;//message structure for the inspector 2 and inspector 3
            message_insp2_insp3.mtype = 1;//set the message type
            message_insp2_insp3.file_number = min_node.file_number;//set the file number
            message_insp2_insp3.time = time(NULL);//set the time of creating the file
            //send the message to the inspector 3
            if (msgsnd(msg_id_insp2_insp3, &message_insp2_insp3, sizeof(message_insp2_insp3.file_number) + sizeof(message_insp2_insp3.time), 0) == -1) {
                perror("Message send failed");
                exit(1);
            } else {
                #ifdef __DEBUG
                printf("Inspector2 %d sent file number and time to queue: %d\n", getpid(), min_node.file_number);
                #endif
            }
            #ifdef __DEBUG
            printf("Mover %d moved file number: %d\n", getpid(), min_node.file_number);

            #endif
        }
    }
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
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
    key_t sem_key = atoi(argv[2]);//get the semaphore key
    sem_id = semget(sem_key, 1, 0666);//get the semaphore id
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
    semaphore_wait(sem_id);//lock the semaphore
    if (!dirExists(backupDir)) {//check if the backup directory exists
        if (createDirectory(backupDir) == -1) { //create the backup directory
            perror("Error creating backup directory");
            semaphore_signal(sem_id);//unlock the semaphore
            exit(EXIT_FAILURE);
        }
    }
    semaphore_signal(sem_id);//unlock the semaphore

    // Retrieve message queue key from environment
    char *key_str = getenv("MSG_QUEUE_MI2_KEY");//get the key of the message queue between mover and inspector2
    if (key_str == NULL) {//check if the key is set
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


    // insp2 , inpsp3 message queue
    char *key_str_insp2_insp3 = getenv("MSG_QUEUE_I2I3_KEY");//get the key of the message queue between inspector2 and inspector3
    if (key_str_insp2_insp3 == NULL) {//check if the key is set
        fprintf(stderr, "Error: MSG_QUEUE_KEY not set.\n");
        exit(EXIT_FAILURE);
    }

    int msg_key_insp2_insp3 = atoi(key_str_insp2_insp3);//convert the key to integer

    // Get message queue ID
    msg_id_insp2_insp3 = msgget(msg_key_insp2_insp3, 0666);//get the message queue id between inspector2 and inspector3
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
    signal(SIGINT, handle_signal);//handle the signal SIGINT
    signal(SIGTERM, handle_signal);//handle the signal SIGTERM

    struct msgbuf2 message;//message structure for the inspector 1


    // Main loop
    while (1) {
        usleep(20000);//sleep for 20 ms
        if (msgrcv(msg_id, &message, sizeof(message.file_number) + sizeof(message.time), 1, IPC_NOWAIT) == -1) {//receive the message from the mover with type 1
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
        #ifdef __DEBUG
        printf("Inspector1: File Number: %d, Creation Time: %ld\n",
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
