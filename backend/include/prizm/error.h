
/* 
Frank Bender
Prizm software testing
5-5-2022: May make a c only version of this file to support portability 
5-20-2022: Converted to c for python ctypes and portability 
5-24-2022: TODO, ensure no buffer overflow with VA_ARGS_SUBSTITUION
        Also, check preprocessor macros for int overflow, etc.
*/

#ifndef ERROR_H_
#define ERROR_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#include "prizm/file_system.h"
#include "prizm/print_color.h"

#define ERR_BUF_SIZE 2048
#define VERBOSE

// may need to revamp, compartmentalize
// and diversify to provide better
// distinguished info as project grows
typedef enum ErrType {
    ENULL, // should never happen
    EINVARG, // invalid argument
    EUNDEFREF, // undefined reference
    EDNE, // does not exist
    EMIG, // migration error
    EDB, // database error
    ENEM, // not enough memory
    ENULLPTR, // nullptr
    ECOMPILE, // compilation error
    ESERVER, // Network error
    ETHREAD, // Thread error
    EWS, // Websocket error
    EFR, // Frame error
    ESOCK, // Socket error
    ECONN,
    ESZ, // Size too large (bo prevention)
    ESSH // ssh error
} ErrType;

typedef enum LogType {
    LFAIL,
    LINFO,
    LERR,
    LWARN,
    LDB,
    LENTITY,
    LSERVER,
    LTHREAD
} LogType;

// maybe able to dry up
// with [-1*ERR] = ERR_MSG
// could allow for changing messages on start up
#define ENULL_MSG "Null Error!"
#define EINVARG_MSG "Invalid Argument!"
#define EUNDEFREF_MSG "Undefined Reference!"
#define EDNE_MSG "Does Not exist!"
#define EMIG_MSG "Migration Error!"
#define EDB_MSG "Database Error!"
#define ENEM_MSG "Not Enough Memory!"
#define ENULLPTR_MSG "Nullptr Error!"
#define ECOMPILE_MSG "Compilation Error!"
#define ESERVER_MSG "Server Error!"
#define ETHREAD_MSG "Thread Error!"
#define EWS_MSG "Web Socket Error!"
#define EFR_MSG "Frame Error!"
#define ESOCK_MSG "Socket Error!"
#define ECONN_MSG "Connection Error!"
#define ESZ_MSG "Size too large"

const char* log_get_label(LogType type);
const char* err_get_label(ErrType type);

typedef struct Err {
    ErrType type;
    int line;
    char file[256];
    char msg[256];
    char func[256];
} Err;

typedef struct PrizmProfile {
    int red;
    int green;
    int yellow;
    int blue;
    int magenta;
    int cyan;
} PrizmProfile;

void err_set_file(Err* err, const char* file);
void err_set_msg(Err* err, const char* msg);
void err_set_func(Err* err, const char* func);
void err_print(Err* err, int error);
void err_construct(Err* err, ErrType type, const char* file, const char* func, 
                int line, const char* msg);
void log_construct(LogType _type, const char* file, const char* func, int line, const char* msg);
void log_construct(PrizmProfile* profile, LogType _type, const char* subtype, int type, const char* file, const char* func, int line, const char* msg);
void profile_set_state(PrizmProfile* profile, bool inc, int color);
void profile_print(PrizmProfile* _profile);

static int seg_helper = 0;

// not re-entrant safe
// not thread safe
static Err* err;
static char error_buffer[ERR_BUF_SIZE];
static pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t profile_mutex = PTHREAD_MUTEX_INITIALIZER;

// heap allocation of err may be too slow?

static PrizmProfile* profile;

#define PINIT \
    YEL("Starting prizm profile...\n"); \
    profile = (PrizmProfile*)calloc(1, sizeof(PrizmProfile)); \
    profile->red = 0; \
    profile->green = 0; \
    profile->yellow = 0; \
    profile->blue = 0; \
    profile->magenta = 0; \
    profile->cyan = 0;

#define PSHUTDOWN \
    YEL("Terminating prizm profile...\n"); \
    free(profile);

#define PLOG(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, ERR_BUF_SIZE, __VA_ARGS__); \
    log_construct(_type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err);

#define PLOGV(_type, _subtype, _cat, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, ERR_BUF_SIZE, __VA_ARGS__); \
    log_construct(profile, _type, _subtype, _cat, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err);

#define PERR(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, ERR_BUF_SIZE, __VA_ARGS__); \
    err_construct(err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 1); \
    log_construct(LERR, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err);

#define PFAIL(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, ERR_BUF_SIZE, __VA_ARGS__); \
    err_construct(err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 1); \
    log_construct(LFAIL, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err); \
    exit(1);

#define PWARN(_type, ...) \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, ERR_BUF_SIZE, __VA_ARGS__); \
    err_construct(err, _type, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 0); \
    log_construct(LWARN, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err);

// thread safe, probably not re-entrant safe
#define PLOG_T(...) \
    pthread_mutex_lock(&error_mutex); \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, ERR_BUF_SIZE, __VA_ARGS__); \
    log_construct(LTHREAD, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err); \
    pthread_mutex_unlock(&error_mutex);

#define PERR_T(...) \
    pthread_mutex_lock(&error_mutex); \
    err = (Err*)malloc(sizeof(Err)); \
    snprintf(error_buffer, ERR_BUF_SIZE, __VA_ARGS__); \
    err_construct(err, ETHREAD, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    err_print(err, 0); \
    log_construct(LTHREAD, __FILE__, __PRETTY_FUNCTION__, __LINE__, error_buffer); \
    free(err); \
    pthread_mutex_unlock(&error_mutex);


#ifdef _DEBUG
#define DEBUG(...) \
    BCYA(__VA_ARGS__)
#else
#define DEBUG(...) ;
#endif

#define SEGH \
    BRED("Segfault helper: (HERE: %i, line: %i)\n", seg_helper, __LINE__); \
    seg_helper++;

#endif