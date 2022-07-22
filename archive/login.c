// // C++ and C strings DO NOT play well together, christs sakes

// #include "prizm/print_color.h"
// // #include "prizm/print_color.h"
// // #include "prizm/error.h"
// // getting sick of fucking c
// // #include <vector>
// #include <string.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
// // #include "libpq-fe.h" // deps: libpq-dev, bison, "f-something", libgcrypt-dev, libpqxx-dev for c++
// #include "openssl/sha.h"

// // fucking, c aint this worth it
// #include <vector>
// #include <string>
// #include <utility>
// #include <iostream>

// static void do_exit(PGconn *conn, PGresult* result) {
//     PQclear(result);
//     PQfinish(conn);
//     printf("\033[0;31mExiting\n\033[0m");
//     exit(1);
// }

// void external(std::string user, std::string pass, std::string salt) {
//     std::string s = "INSERT INTO Users VALS('";
//         // std::string st(st1);
//     // s = "1, " + user + "','" + pass + "','" + salt + "')";
//     // st += "Hello";
//     // st += "', '";
//     // st += toIns;
//     // st += "', '";
//     // st += salt;
//     // st += "')";
//     // printf("%s\n", s.c_str());
//     std::cout << s << std::endl;
// }

// int lenHelper(unsigned x) {
//     if (x >= 1000000000) return 10;
//     if (x >= 100000000)  return 9;
//     if (x >= 10000000)   return 8;
//     if (x >= 1000000)    return 7;
//     if (x >= 100000)     return 6;
//     if (x >= 10000)      return 5;
//     if (x >= 1000)       return 4;
//     if (x >= 100)        return 3;
//     if (x >= 10)         return 2;
//     return 1;
// }

// int printLen(int x) {
//     return x < 0 ? lenHelper(-x) + 1 : lenHelper(x);
// }

// int main(int argc, char* argv[]) {
//     // char* str = argv[1];
//     // printf("PATH : %s\n", getenv("ENV_VAR"));
//     // printf("TOKEN: %s\n", str);
//     // write_file("login.log", str);

//     PGconn *conn = PQconnectdb("dbname=demodb hostaddr=127.0.0.1 port=5432");

//     if (PQstatus(conn) == CONNECTION_BAD) {
        
//         fprintf(stderr, "Connection to database failed: %s\n",
//             PQerrorMessage(conn));
//         // do_exit(conn);
//         PQfinish(conn);
//     }

//     int ver = PQserverVersion(conn);

//     printf("Server version: %d\n", ver);

//     char str[] = "username=johnsnow&password=pass1234&hello=something&new=whoa";
//     char* save = str;
//     char* token;
//     char** tokens = (char**)calloc(1, 50);
//     int i = 0;
//     int offset = 0;
//     int max = 0;
// // C IMPLEMENTATION WONT INTEGRATE WITH C++ (WTF)
//     printf("Save is: %s\n", save);
//     while (token = strtok_r(save, "&", &save)) {
//         printf("Token %i: %s\n", i, token);
//         int tsz = (int)strlen(token) + 1;
//         if (tsz > max) max = tsz;
//         char val[tsz];
//         strncpy(val, token, tsz);
//         val[tsz] = 0;
//         i++;
//         if (i == 1) {
//             *tokens = token;
//         } else {
//             char** tokens_copy = (char**)calloc(i, max);
//             for (int j = 0; j < i; j++) {
//                 if (j != i - 1) {
//                     *(tokens_copy + j) = *(tokens + j);
//                 } else {
//                     *(tokens_copy + j) = token;
//                 }
//             }
//             memcpy(tokens, tokens_copy, i * max);
//             free(tokens_copy);
//         }
//     }
//     int len = i;
//     // for (int j = 0; j < len; j++) {
//     //     printf("%i - %s\n", j, (char*)*(tokens + j));
//     //     char* str2 = *(tokens + j);
//     //     char* tok;
//     //     int k = 0;
//     //     while (tok = strtok_r(str2, "=", &str2)) {
//     //         printf("Token %i: %s\n", k, tok);
//     //         // tokens2[i][j % 2] = (char*)malloc(strlen(tok) + 1);
//     //         // strncpy((tokens2)[i][j], tok, strlen(tok) + 1);
//     //         // tokens2[i][j % 2][strlen(tok) + 1] = 0;
//     //         k++;
//     //     }
//     // }

//     // switch to c++
//     std::vector<std::pair<std::string, std::string>> attrs;
//     for (int j = 0; j < len; j++) {
//         char* str2 = *(tokens + j);
//         std::string s(str2);
//         // std::cout << "str2: " << s << std::endl;
//         size_t pos = 0;
//         int k = 0;
//         std::pair<std::string, std::string> attr;
//         while ((pos = s.find("=")) != std::string::npos) {
//             std::string tk = s.substr(0, pos);
//             std::string tk2 = s.substr(pos + 1, s.size());
//             attr.first = tk;
//             attr.second = tk2;
//             s.erase(0, pos + strlen("="));
//             attrs.push_back(attr);
//         }
//     }
//     free(tokens);
//     std::string username;
//     char password[32];

//     for (auto a : attrs) {
//         // printf("first: %s\n", a.first.c_str());
//         // printf("second: %s\n", a.second.c_str());
//         if (a.first == "password") {
//             strncpy(password, a.second.c_str(), 32);
//             password[32] = 0;
//         } else if (a.first == "username") {
//             username = a.second;
//         }
//     }

//     const unsigned char* toHash = (unsigned char*)password;
//     char* hash = NULL;

//     // toHash = calloc(SHA256_DIGEST_LENGTH, sizeof(const unsigned char));
//     size_t plen = strlen((const char*) toHash);

//     long int t = (int)time(NULL);
//     // long int t = 1653854050; // test case
//     printf("Timestamp: %ld\n",t);
//     printf("sizeof time: %li\n",sizeof(t));
//     char tbuffer[printLen(t) + 1];
//     snprintf(tbuffer, printLen(t) + 1, "%ld", t);
//     printf("tbuffer: %s\n", tbuffer);
//     printf("sizeof tbuffer: %i\n", (int)sizeof(tbuffer));
//     size_t tlen = strlen((const char*) tbuffer);

//     char pdigest[tlen + plen + 1];
//     printf("pdigest: %ld\n", sizeof(pdigest));
//     strncpy(pdigest, tbuffer, tlen + 1);
//     printf("pdigest: %s\n", pdigest);
//     strncat(pdigest, (char*)toHash, plen + 1);
//     printf("pdigest: %s\n", pdigest);
//     pdigest[tlen + plen + 1] = 0;
//     printf("pdigest len: %i\n", (int)strlen(pdigest));
    
//     hash = (char*)calloc(SHA256_DIGEST_LENGTH, sizeof(char));
//     size_t len2 = strlen((const char*) pdigest);
//     SHA256((unsigned char*)pdigest, len2, (unsigned char*) hash);

//     int term = strlen(hash) * 2;
//     printf("term: %i\n", term);
//     // while (term != 64) {
//     //     free(hash);
//     //     hash = calloc(SHA256_DIGEST_LENGTH, sizeof(char));
//     //     len2 = strlen((const char*) pdigest);
//     //     SHA256(pdigest, len2, (unsigned char*) hash);
//     //     term = strlen(hash) * 2;
//     //     // printf("NEW TERM: %i\n", term);
//     // }
//     char buffer[65];

//     // buffer overflow risk? 
//     printf("strlen: %i\n", (int)strlen(buffer));
//     int length = 0;
//     for(unsigned int j = 0; j < SHA256_DIGEST_LENGTH; j ++) {
// 	    length += sprintf(buffer + length, "%02hhX", hash[j]);
//         printf("%02hhX", hash[j]);
//     }
//     buffer[65] = 0;
//     printf("\n");
//     printf("buffer: %s\n", buffer);
//     printf("sizeof buffer: %i\n", (int)sizeof(buffer));
//     free(hash);

//     // do some kind of sanity check/checksum

//     // std::string pass(buffer);
//     // std::string salt(tbuffer);
//     // std::string uname(username);

//     // std::cout << "USERNAME IS: " << username << std::endl;
//     // std::cout << "SALT IS: " << salt << std::endl;
//     // std::cout << "HASH IS: " << toIns << std::endl;

//     // PGresult *res;
//     // PGresult *res = PQexec(conn, "DROP TABLE IF EXISTS Cars");
    
//     // if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//     //     do_exit(conn, res);
//     // }
    
//     // PQclear(res);
    
//     // PGresult* res = PQexec(conn, "CREATE TABLE Users(Id INTEGER PRIMARY KEY," \
//     //     "username VARCHAR(16), password VARCHAR(64), salt VARCHAR(10))");
        
//     // if (PQresultStatus(res) != PGRES_COMMAND_OK) {
//     //     do_exit(conn, res); 
//     // }
    
//     // PQclear(res);
//     // external(uname, toIns, salt);
    
//     // std::string sep("','");
//     // std::string begin("INSERT INTO Users VALUES(1,");
//     // std::string end("')");
//     std::string why("This is fucked.");
//     why = why + why;
//     // std::string fin(begin + uname + sep + pass + sep + salt + end);
//     std::cout << why << std::endl;

//     // prevent injection
//     // sanitize
//     // char bgbuffer[512];
//     // sprintf(pgbuffer, "INSERT INTO Users VALUES(%i, '%s', '%s', '%s')", 1, );
//     // pgbuffer[512] = 0; // lazy, need to create better way to build queries
//     // res = PQexec(conn, "INSERT INTO Users VALUES(1,'jonsnow','%s', '%s')");
        
//     // if (PQresultStatus(res) != PGRES_COMMAND_OK) 
//     //     do_exit(conn, res);     
    
//     // PQclear(res);    
//     printf("Not terminating\n");

//     PQfinish(conn);

//     printf("Terminating\n");

//     return 0;
// }