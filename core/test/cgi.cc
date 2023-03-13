#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#include <string>
#include <vector>

#include "prizm/prizm.h"
#include "system/cgi.h"
#include "crypt/rsa.h"

int main() {
    std::vector<char*> args = jcrypt::rsa::generate(4096, "brittany_spears", "./temp/keys/");
    int status = pipe("./bin/rsa", args.data());
    BGRE("TERMINATED!\n");
    return 0;
}