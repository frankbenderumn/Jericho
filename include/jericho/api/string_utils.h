#ifndef JERICHO_API_STRING_UTILS_H_
#define JERICHO_API_STRING_UTILS_H_

#include <sstream>
#include <vector>
#include <string>
#include <deque>
#include <utility>
#include <regex>
#include <iterator>
#include <algorithm>

#include "prizm/prizm.h"

#ifndef LEXES
#define LEXES std::deque<std::pair<std::string, int>>
#endif

#ifndef LEX
#define LEX std::pair<std::string, int>
#endif

std::string reverse(std::string s);

std::vector<std::string> reverse(std::vector<std::string> s);
 
std::string ltrim(const std::string &s);
 
std::string rtrim(const std::string &s);
 
std::string trim(const std::string &s);

void ltrim_force(std::string& s);

void rtrim_force(std::string& s);

void trim_force(std::string& s);

void erase(std::string& str, char toRemove);

void trim_tokens(std::vector<std::string>& toks);

void dump_tokens(std::vector<std::string> toks);

std::vector<std::string> tokenize(std::string in, char delim);

std::vector<std::string> tokenize(std::string in, std::string delim);

LEXES lex_locs(std::string substr, std::regex rgx);

bool lex_contains(std::string substr, std::regex rgx);

bool lex_is(std::string substr, std::regex rgx);

std::string replace(std::string s, char a, char b);

#endif