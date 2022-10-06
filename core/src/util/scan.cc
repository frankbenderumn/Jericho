#include "util/scan.h"

bool Jericho::jscan(const char* regex_val, char* request) {
    // the following regex scanners detect if websocket protocol and extracts key if so
    // const char* regex = "Upgrade: websocket"
    bool ws_protocol = false;

    regex_t regex;
    regmatch_t match[4];
    regoff_t off, len;
    char* str = request;
    char* s = str;
    char ws_key[100];
    int value = regcomp(&regex, regex_val, 0);
    if (value) {
        fprintf(stderr, "Failed to compile regex!\n");
        exit(1);
    }

    char tbuffer[1024];

    yellow(); 
    printf("Regex: %s\n", regex_val);
    printf("Matches: \n");
    for (int i = 0; ; i++) {
        if (regexec(&regex, s, ARRAY_SIZE(match), match, 0))
            break;
        ws_protocol = TRUE;
        off = match[0].rm_so + (s - str);
        len = match[0].rm_eo - match[0].rm_so;
        printf("#%d:\n", i);
        printf("offset = %jd; length = %jd\n", (intmax_t) off,
                (intmax_t) len);
        printf("substring = \"%.*s\"\n", len, s + match[0].rm_so);

        s += match[0].rm_eo;
    }
    clearcolor();
    // SEGH
    // regfree(&regex);

    return ws_protocol;
}