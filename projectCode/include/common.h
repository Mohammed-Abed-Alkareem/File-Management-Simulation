#ifndef COMMON_H
#define COMMON_H

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
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "utils.h"
#include <semaphore.h>
#include <fcntl.h>
#include <math.h>
#include <limits.h>


typedef struct {
    int total_csv_generated;
    int total_csv_calculated;
    int unprocessed_csv;
    int files_moved_to_backup;
    int files_deleted;
    float max_avg;
    float min_avg;
    int  max_avg_file;
    int  min_avg_file;
    int max_avg_col;
    int min_avg_col;

} SharedData;


#define filesDir "./files"
#define homeDir "./files/home" // home directory can be added in another directory if needed
#define processesdDir "./files/home/Processed"
#define unprocessedDir "./files/home/unprocessed"
#define backupDir "./files/home/backup"
#define logDir "./files/log"
#define logFile "./files/log/log.txt" 
#define logFileSem "/log_sem"
#endif // COMMON_H