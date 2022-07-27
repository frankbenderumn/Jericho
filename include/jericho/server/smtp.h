/*
* 5-24-2022: WIP, going to need DKIM, DMARC and SPF
* use domain.com DNS. Will need to link separate server 
* with unix socket, and send requests asynchronously.
* May use mpi instead of unix system socket.
*/ 


#ifndef SERVER_SMTP_H_ 
#define SERVER_SMTP_H_

#define MAXINPUT 512
#define MAXRESPONSE 1024

#include <ctype.h>
#include <stdarg.h>

// use port 465 and 587 for spam prevention
// STARTTLS command for secure smtp

#include "server/defs.h"
#include "server/socket.h"

#endif