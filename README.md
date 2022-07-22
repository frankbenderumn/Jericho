# Python/CPP Interoperability

## Startup

Utilizing a bash shell and linux environment run the script
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
