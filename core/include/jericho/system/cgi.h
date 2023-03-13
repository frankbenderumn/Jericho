#ifndef JERICHO_SYSTEM_CGI_H_
#define JERICHO_SYSTEM_CGI_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#include <vector>
#include <string>

#include "prizm/prizm.h"

typedef std::vector<char*> args_t;

int pipe(const char* script, char* args[]) {
    pid_t pid;
    int status;
    /* Get and print my own pid, then fork and check for errors */
    // printf("My PID is %d\n", getpid());
    if ( (pid = fork()) == -1 ) {
        perror("Can't fork");
        return 1;
    }
    if (pid == 0) {
        if ( execv(script, args) ) {
            perror("Can't exec");
            return 1;
        }
    } else if (pid > 0) {
        /* In the parent we must wait for the child
            to exit with waitpid(). Afterward, the
            child exit status is written to 'status' */
        waitpid(pid, &status, 0);
        // printf("Child executed with PID %d\n", pid);
        // printf("Its return status was %d\n", status);
        // printf("Its return status was %d\n", status / 256);

        if (status / 256 == 0) {
            // BGRE("Subprocess terminated normally!\n");
        } else if (status == 11) {
            BRED("Segfault detected in subprocess!\n");
        } else {
            BRED("Subprocess did not terminate normally!\n");
        }

    } else {
        fprintf(stderr, "Something went wrong forking\n");
        return 1;
    }
    return status;
}

#endif