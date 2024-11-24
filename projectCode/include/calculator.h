#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "common.h"

int main(int argc, char *argv[]);
float calculateAvgCSV(char *filename);
int getNumRowsCSV(char *filename);
int getNumColsCSV(char *filename);

// Message structure
struct msgbuf {
    long mtype;
    int file_number;
};

#endif // CALCULATOR_H
