#ifndef UTILS_H
#define UTILS_H


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

int dirExists(const char *path);
int fileExists(const char *path);
int createDirectory(const char *path);
int movefile(const char *filename, const char* srcDir, const char* destDir);

#endif // UTILS_H

