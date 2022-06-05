#!/bin/bash
make clean
make

rm -f cgi/login
g++ ./backend/src/prizm/file_system.cc \
    ./cgi/login.cc -I./backend/include -I./deps -Ipqxx \
    -I./psql-install/include -L./psql-install/lib -lpqxx -lssl -lcrypto -o cgi/login 

rm -f db/migrate
g++ ./backend/src/prizm/file_system.cc \
    ./db/migrate.cc -I./backend/include -I./deps -Ipqxx \
    -I./psql-install/include -L./psql-install/lib -lpqxx -lssl -lcrypto -o db/migrate

rm -f deps/iris/main
g++ ./backend/src/prizm/file_system.cc \
    ./deps/iris/main.cc -I./backend/include -I./deps -Ipqxx \
    -I./psql-install/include -L./psql-install/lib -lpqxx -lssl -lcrypto -o deps/iris/main

SAMPLE_ENV="Hello environment"

./build/bin/prizm 10 ./frontend