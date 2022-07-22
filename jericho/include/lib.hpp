/* 
Frank Bender
Prizm software testing (Python port)
5-5-2022: May make a c only version of this file to support portability 
*/

#ifndef LIB_H_
#define LIB_H_

#include <stdio.h>
#include "util/print_color.h"
#include "util/file_system.hpp"
#include <iostream>

extern "C" void wonder() {
    blue();
    printf("Wonderful!\n");
    clearcolor();
    std::string s = FileSystem::read("./frontend/index.html");
    std::cout << s << std::endl;
}

#endif