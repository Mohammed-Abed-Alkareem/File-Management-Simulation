#ifndef UTILS_H
#define UTILS_H


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
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

int dirExists(const char *path);
int fileExists(const char *path);
int createDirectory(const char *path);
int movefile(const char *filename, const char* srcDir, const char* destDir);
void semaphore_wait(int sem_id);
void semaphore_signal(int sem_id);
int deleteFile(const char *filename, const char* dir);
int removeDirectory(const char *path);
#endif // UTILS_H

