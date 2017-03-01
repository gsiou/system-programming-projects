#ifndef _UTILS_H
#define _UTILS_H

// More reasonable way to compare strings for equality
#define streqn !strncmp

// Safe version of read.
bool
safeRead(int fd, char *buffer, int length);


// Read file in chunks.
bool
safeFileRead(int fd, char *buffer, long fileSize);

#endif
