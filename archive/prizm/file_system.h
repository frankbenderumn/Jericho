#ifndef UTIL_FILE_SYSTEM_C_H_
#define UTIL_FILE_SYSTEM_C_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t thread_log_mutex = PTHREAD_MUTEX_INITIALIZER;

void write_file(const char* path, const char* content);
void write_thread(int thread_id, const char* string);

#endif