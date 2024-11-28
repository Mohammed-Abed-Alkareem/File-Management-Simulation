#ifndef GENERATOR_H
#define GENERATOR_H

#include "common.h"
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

int main(int argc, char *argv[]);
float getRandomFloat(int min, int max);
void generateCSV(int);
int getRandomInt(int min, int max);

struct msgbuf {
    long mtype;        // Message type (must be > 0)
    int file_number;   // The file number
};

struct msgbuf2 {
    long mtype;        // Message type (must be > 0)
    int file_number;   // The file number
   // time of creating the file
   time_t time;
};

#endif // GENERATOR_H
