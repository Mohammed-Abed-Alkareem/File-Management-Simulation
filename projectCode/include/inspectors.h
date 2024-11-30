#ifndef INSPECTORS_H
#define INSPECTORS_H

#include "common.h"
#include "heap.h"


int main (int argc , char * argv[]);

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


#endif // INSPECTORS_H