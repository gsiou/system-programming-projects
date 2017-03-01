#include "utils.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

bool
safeRead(int fd, char *buffer, int length){
    int bytesRead;
    int nRead;
    bytesRead = 0;
    nRead = 0;
    
    while(bytesRead < length){
        nRead = read(fd, buffer, length);
        if(nRead == -1 && errno != EAGAIN){
            perror("Pipe reading failed");
            return false;
        }
        bytesRead += nRead;
    }
    return true;
}

bool
safeFileRead(int fd, char *buffer, long fileSize){
    int chunkSize = 4096;
    char chunk[chunkSize];
    int bytesRead = 0;
    int n;
    
    while(bytesRead < fileSize){
        n = read(fd, chunk, chunkSize);
        if(n == -1 && errno != EAGAIN){
            perror("File sending over pipe failed");
            return false;           
        }
        memcpy(buffer + bytesRead, chunk, n);
        bytesRead += n;
    }
    return true;
}
