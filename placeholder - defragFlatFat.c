#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "fatOperations.h"

#define FAT_WIDTH 256
#define BLOCK_SIZE 1024
#define BLOCK_COUNT 256

#define SYS_CALL_GENERIC_ERR_THRESHOLD 0
#define ERROR_MSG "Invalid"
#define ARGCOUNT_ERROR_MSG "Invalid argument input, correct usage is: \"./defragFlatFat [filesystem]\""
#define OPEN_ERROR_MSG "Error at opening fs"
#define WRITE_ERROR_MSG "Error at write to fs"


int fileSystemFd;
unsigned char fat[FAT_WIDTH];
unsigned char *fatAddress;

int main(int argc, char **argv) {
    //check arguments
    if (argc != 2) {
        perror(ARGCOUNT_ERROR_MSG);
        exit(EXIT_FAILURE);
    }

    //./defragFlatFat fs

    char *fsName = argv[1];

    fileSystemFd = openFs(fsName);
    if (fileSystemFd == -1) {
        return fileSystemFd;
    }

    fatAddress = readFatFromDisk(fileSystemFd);
    if (fatAddress == 0) {
        perror("readFatFromDisk error");
        return -1;
    }
    memcpy(fat, fatAddress, FAT_WIDTH);

    int errCheck = 0;
    errCheck = readDirectoryListIntoMemory(fileSystemFd, fat);
    if (errCheck != 0) {
        return errCheck;
        //!this specific function does not do error checking, assuming that the link in storage
        //is correct. Ultimately checking correctness of the linkedlist in storage would be beyond
        //the scope of a function like this, so I don't know what the correct response would be
    }

    errCheck = -1;

    DEBUG(printf("got to DEFRAG stage\n"));

    errCheck = defragFileSystem(fileSystemFd, fat);
    if (errCheck < 0) {
        DEBUG(printf("\nError in defrag->defragFS: %d\n", errCheck));
    }

    errCheck = freeDirectoryFromMemory();
    if (errCheck != 0) {
        return errCheck;
    }
    close(fileSystemFd);
    return 0;
}