#ifndef MULTI-LOOKUP_H
#define MULTI-LOOKUP_H

/* Define the following to enable debug statments */
// #define UTIL_DEBUG

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include "util.h"

#define UTIL_FAILURE -1
#define UTIL_SUCCESS 0



void *readFile(void* buffer);
void *writeFile(void *buffer);

#endif
