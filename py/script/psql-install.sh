#!/bin/bash
SOURCE=$PWD
mkdir -p psql-install
cd psql-install
../deps/postgres/configure
sudo make
sudo make install
