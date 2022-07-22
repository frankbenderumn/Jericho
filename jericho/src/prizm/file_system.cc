#include "prizm/file_system.h"

void write_file(const char* path, const char* content) {
    int num;
    FILE* fp;
    fp = fopen(path, "a");

    if(fp == NULL) {
        printf("Unable to create file!\n");   
        exit(1);
    }

    fprintf(fp, "%s\n", content);
    fclose(fp);
}

void write_thread(int thread_id, const char* string) {
    int num;
    FILE* fp;
    pthread_mutex_lock(&thread_log_mutex);
    fp = fopen("./log/thread.log", "a");

    if(fp == NULL) {
        printf("Unable to create file!\n");   
        exit(1);             
    }

    fprintf(fp,"%i - %s", thread_id, string);
    fclose(fp);
    pthread_mutex_unlock(&thread_log_mutex);
}