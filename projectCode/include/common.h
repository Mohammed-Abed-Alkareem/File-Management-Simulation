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
#include <math.h>

#define filesDir "./files"
#define homeDir "./files/home" // home directory can be added in another directory if needed
#define processesdDir "./files/home/Processed"
#define unprocessedDir "./files/home/unprocessed"
#define backupDir "./files/home/backup"

#endif // COMMON_H