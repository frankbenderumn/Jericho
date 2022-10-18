Jericho

## Distributed System Framework

This project consists of 4 libraries:

nebulaui
prizm
celerity
jericho

Nebulaui is a frontend framework utilizing webgl and jquery
Prizm is a testing framework and benchmarking library
Celerity is a universal database orm that currently supports postgres and mongo
Jericho is a distributed server architecture

Jericho is near completion of supporting federated learning through execution of tensorflow python scripts

The main mechanisms are as follows:


```
./script/py.sh
```
to compile the project for python, run 
```
./script/cpp.sh
```
to compile and run the project with cpp

The included shared object library in the prizm folder exists to help debug, test, and log code and 
information as the development process continues. Log files will be generated in the log directory.
More details regarding testing will be posted later.

To export a function over to c++, modifying the file prizm/cpp/include/lib.h and adding the qualifier
extern "C" will allow you to call the function in python
