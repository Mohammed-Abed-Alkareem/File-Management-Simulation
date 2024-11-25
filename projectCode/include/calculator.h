#ifndef CALCULATOR_H
#define CALCULATOR_H

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
float calculateAvgCSV(char *filename, int file_number);
int getNumRowsCSV(char *filename);
int getNumColsCSV(char *filename);

// Message structure
struct msgbuf {
    long mtype;
    int file_number;
};

#endif // CALCULATOR_H
