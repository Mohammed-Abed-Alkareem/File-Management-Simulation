#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>   
#include <string.h>
#include "config.h"
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>

#ifndef _SEM_SEMUN_UNDEFINED
 // Define the semaphore structure
        struct sembuf {
        unsigned short sem_num;  // Semaphore number in the set
        short sem_op;            // Semaphore operation
        short sem_flg;           // Operation flags
        };
#endif

int main(int argc, char *argv[]);
key_t key_generator(char letter);
void cleanup();
void handle_usr1(int signal);
void handle_signal(int signal);

#endif // MAIN_H