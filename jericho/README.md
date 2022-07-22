# Backend

## Introduction

Three folders are available: cpp, c, and deps. The c directory is for c only backend, the cpp directory 
is for a cpp version. Both have been designed to compile with python ctypes. The deps directory is 
for dependencies but may create conflict with python ctypes. Multiple targets will be designed in 
the makefile at a later point to make some builds more niche and lightweight

## Issues

Have not gotten the server to work with both OpenSSL and ctypes. A solution may involve installing 
the OpenSSL includes directory locally in the deps directory and externalizing some of the functions 
to C so the g++ compiler does not mangle their names.