
/* 
Frank Bender
Prizm software testing
5-5-2022: May make a c only version of this file to support portability 
5-20-2022: Converted to c for python ctypes and portability 
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#include "prizm/error.h"

const char* log_get_label(LogType type) {
    static char buffer[16];
    switch (type) {
        case LFAIL:
            snprintf(buffer, sizeof("FAIL"), "FAIL");
            break;
        case LINFO:
            snprintf(buffer, sizeof("INFO"), "INFO");
            break;
        case LERR:
            snprintf(buffer, sizeof("ERROR"), "ERROR");
            break;
        case LWARN:
            snprintf(buffer, sizeof("WARN"), "WARN");
            break;
        case LDB:
            snprintf(buffer, sizeof("DATABASE"), "DATABASE");
            break;
        case LSERVER:
            snprintf(buffer, sizeof("SERVER"), "SERVER");
            break;
        case LENTITY:
            snprintf(buffer, sizeof("ENTITY"), "ENTITY");
            break;
        case LTHREAD:
            snprintf(buffer, sizeof("THREAD"), "THREAD");
            break;
    }
    return buffer;
} 

const char* err_get_label(ErrType type) {
    static char buffer[32];
    switch (type) {
        case ENULL:
            snprintf(buffer, sizeof(ENULL_MSG), ENULL_MSG);
            break;
        case EINVARG:
            snprintf(buffer, sizeof(EINVARG_MSG), EINVARG_MSG);
            break;
        case EUNDEFREF:
            snprintf(buffer, sizeof(EUNDEFREF_MSG), EUNDEFREF_MSG);
            break;
        case EDNE:
            snprintf(buffer, sizeof(EDNE_MSG), EDNE_MSG);
            break;
        case EMIG:
            snprintf(buffer, sizeof(EMIG_MSG), EMIG_MSG);
            break;
        case EDB:
            snprintf(buffer, sizeof(EDB_MSG), EDB_MSG);
            break;
        case ENEM:
            snprintf(buffer, sizeof(ENEM_MSG), ENEM_MSG);
            break;
        case ENULLPTR:
            snprintf(buffer, sizeof(ENULLPTR_MSG), ENULLPTR_MSG);
            break;
        case ECOMPILE:
            snprintf(buffer, sizeof(ECOMPILE_MSG), ECOMPILE_MSG);
            break;
        case ESERVER:
            snprintf(buffer, sizeof(ESERVER_MSG), ESERVER_MSG);
            break;
        case ETHREAD:
            snprintf(buffer, sizeof(ETHREAD_MSG), ETHREAD_MSG);
            break;
        case EWS:
            snprintf(buffer, sizeof(EWS_MSG), EWS_MSG);
            break;
        case EFR:
            snprintf(buffer, sizeof(EFR_MSG), EFR_MSG);
            break;
        case ESOCK:
            snprintf(buffer, sizeof(ESOCK_MSG), ESOCK_MSG);
            break;
        case ECONN:
            snprintf(buffer, sizeof(ECONN_MSG), ECONN_MSG);
            break;
        case ESZ:
            snprintf(buffer, sizeof(ESZ_MSG), ESZ_MSG);
            break;
    }
    return buffer;
} 

void err_set_file(Err* err, const char* file) {
    if (strlen(file) > 255) {
        magenta(); printf("ErrType file name truncated\n"); clearcolor();
    }
    strncpy(err->file, file, 255);
    err->file[256] = '\0';
}

void err_set_msg(Err* err, const char* msg) {
    if (strlen(msg) > 255) {
        magenta(); printf("ErrType message name truncated\n"); clearcolor();
    }
    strncpy(err->msg, msg, 255);
    err->msg[256] = '\0';
}

void err_set_func(Err* err, const char* func) {
    if (strlen(func) > 255) {
        magenta(); printf("ErrType buffer overflow on function name\n"); clearcolor();
    }
    strncpy(err->func, func, 255);
    err->func[256] = '\0';
}

void err_print(Err* err, int error) {
    const char* buffer = err_get_label(err->type);
    if (error) {
        BRED("ERROR: ");
        RED("%s @ %s (LINE:%i)\n", err->file, err->func, err->line);
        printf("\t%s -- %s\n", buffer, err->msg);
    } else {
        BMAG("WARNING: ");
        MAG("%s @ %s (LINE:%i)\n", err->file, err->func, err->line);
        printf("\t%s -- %s\n", buffer, err->msg);
    }
}

void err_construct(Err* err, ErrType type, const char* file, const char* func, 
                int line, const char* msg) {
    err_set_file(err, file);
    err_set_msg(err, msg);
    err_set_func(err, func);
    err->type = type;
    err->line = line;
}

void log_construct(LogType _type, const char* file, const char* func, int line, const char* msg) {
    const char* type = log_get_label(_type);
    char buffer[256];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time[128];
    snprintf(time, 128, "[\033[0;33m%d-%02d-%02d %02d:%02d:%02d\033[0m]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    snprintf(buffer, 256, "%s-%s-%s-%s-%i-%s", type, time, file, func, line, msg);
    char path[64];
    snprintf(path, 64, "log/%s.log", type);
    write_file(path, buffer);
    write_file("log/master.log", buffer);
#ifdef VERBOSE
    printf("%s\n", buffer);
#endif
}

void log_construct(PrizmProfile* _profile, LogType _type, const char* subtype, int category, const char* file, const char* func, int line, const char* msg) {
    const char* type = log_get_label(_type);
    char buffer[256];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char time[128];
    snprintf(time, 128, "[\033[0;32m%d-%02d-%02d\033[0m \033[0;33m%02d:%02d:%02d\033[0m]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    switch (category) {
        case 0: 
            snprintf(buffer, 256, "%s-%s-%s-%s-%i: %s-%s", type, time, file, func, line, subtype, msg);
            break;
        case 1: 
            snprintf(buffer, 256, "%s-%s-%s-%s-%i:\033[0;31m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
            break;
        case 2: 
            snprintf(buffer, 256, "%s-%s-%s-%s-%i:\033[0;32m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
            break;
        case 3: 
            snprintf(buffer, 256, "%s-%s-%s-%s-%i:\033[0;33m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
            break;
        case 4: 
            snprintf(buffer, 256, "%s-%s-%s-%s-%i:\033[0;34m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
            break;
        case 5: 
            snprintf(buffer, 256, "%s-%s-%s-%s-%i:\033[0;35m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
            break;
        case 6: 
            snprintf(buffer, 256, "%s-%s-%s-%s-%i:\033[0;36m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
            break;
    }
    // profile_set_state(_profile, true, category);
    // if (strcmp(subtype, "ERROR") == 0) {
    //     snprintf(buffer, 256, "%s-%s-%s-%s-%i: \033[0;31m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
    // } else {
    //     snprintf(buffer, 256, "%s-%s-%s-%s-%i: \033[0;35m%s\033[0m-%s", type, time, file, func, line, subtype, msg);
    // }
    char path[64];
    snprintf(path, 64, "log/%s.log", type);
    write_file(path, buffer);
    if (strcmp(subtype, "ERROR") == 0) {
        write_file("log/error.log", buffer);
    }
    write_file(path, buffer);
    write_file("log/master.log", buffer);
#ifdef VERBOSE
    printf("%s\n", buffer);
#endif
}

void profile_print(PrizmProfile* _profile) {
    printf("Red: %i\n", _profile->red);
    printf("Green: %i\n", _profile->green);
    printf("Yellow: %i\n", _profile->yellow);
    printf("Blue: %i\n", _profile->blue);
    printf("Magenta: %i\n", _profile->magenta);
    printf("Cyan: %i\n", _profile->cyan);
}

void profile_set_state(PrizmProfile* _profile, bool inc, int color) {
    profile_print(_profile);
    int mult;
    (inc) ? mult = 1 : mult = -1;
    pthread_mutex_lock(&profile_mutex);
    switch (color) {
        case 1:
            _profile->red += 1;
            break;
        case 2:
            SEGH
            _profile->green += 1;
            break;
        case 3:
            _profile->yellow += 1;
            break;
        case 4:
            _profile->blue += 1;
            break;
        case 5:
            _profile->magenta += 1;
            break;
        case 6:
            _profile->cyan += 1;
            break;
        default:
            break;
    }
    profile_print(_profile);
    pthread_mutex_unlock(&profile_mutex);
}

