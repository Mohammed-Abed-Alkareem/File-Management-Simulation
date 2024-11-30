// a functions that are used in the project are defined here

#include "utils.h"

int dirExists(const char *path) {//function to check if the directory exists
    struct stat info;
  
    if (stat(path, &info) != 0) {
        // Stat failed, the directory might not exist
        return 0;
    }

    // Check if the path is a directory
    return (info.st_mode & S_IFDIR) ? 1 : 0;
}

int fileExists(const char *path) {//function to check if the file exists
    struct stat info;
  
    if (stat(path, &info) != 0) {
        // Stat failed, the file might not exist
        return 0;
    }

    // Check if the path is a file
    return (info.st_mode & S_IFREG) ? 1 : 0;
}


int createDirectory(const char *path) {//function to create a directory

    if (mkdir(path, 0777) == -1) {
        printf("Error creating directory%s ", path);
        return -1;
    }
    #ifdef __CLI
    printf("Directory %s created\n", path);
    #endif
    return 0;
}

int movefile(const char *filename, const char* srcDir, const char* destDir) {//function to move a file from one directory to another
    char srcPath[100];
    char destPath[100];
    sprintf(srcPath, "%s/%s", srcDir, filename);
    sprintf(destPath, "%s/%s", destDir, filename);

    if (!fileExists(srcPath)) {//check if the source file exists
        perror("Source file does not exist");
        return -1;
    }

    if (fileExists(destPath)) {//check if the destination file exists
        perror("Destination file already exists");
        return -1;
    }

    //check if directory exists
    if (!dirExists(srcDir)) {
        //create directory
        if (createDirectory(srcDir) == -1) {
            return -1;
        }
        #ifdef __CLI
        printf("Directory %s created\n", srcDir);
        #endif
    }

    if (!dirExists(destDir)) {
        //create directory
        if (createDirectory(destDir) == -1) {
            return -1;
        }
        #ifdef __CLI
        printf("Directory %s created\n", destDir);
        #endif
    }


      // Move the file
    if (rename(srcPath, destPath) == 0) {
        #ifdef __CLI
        printf("File successfully moved: '%s' -> '%s'\n", srcPath, destPath);
        #endif
        return 0;
    } else {
        perror("Error moving file");
        return -1;
    }
    


}
// Semaphore operations
void semaphore_wait(int sem_id) {
    struct sembuf sb = {0, -1, 0}; // Decrement semaphore
    if (semop(sem_id, &sb, 1) == -1) {
        perror("Semaphore wait operation failed");
        exit(1); // Exit the program if semaphore operation fails
    }
    // printf("\033[0;31mProcess:%d => Semaphore wait operation\033[0m\n", getpid());
}

void semaphore_signal(int sem_id) {
    struct sembuf sb = {0, 1, 0}; // Increment semaphore
    if (semop(sem_id, &sb, 1) == -1) {
        perror("Semaphore signal operation failed");
        exit(1); // Exit the program if semaphore operation fails
    }
    // printf("\033[0;31mProcess:%d => Semaphore signal operation\033[0m\n", getpid());
}

int deleteFile(const char *filename, const char* dir) {
    char path[100];
    sprintf(path, "%s/%s", dir, filename);
    if (remove(path) == 0) {
        printf("File deleted: %s\n", path);
        return 0;
    } else {
        perror("Error deleting file");
        return -1;
    }
}