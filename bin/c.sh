#!/bin/bash
ROOTDIR=`git rev-parse --show-toplevel`
source $ROOTDIR/config/settings
export CLUSTER

make clean
make FLAGS=-D_$2

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

PORT=$1

if [-z "$1"]
then
    PORT=8080    
fi

./build/bin/prizm $PORT 10 ./frontend