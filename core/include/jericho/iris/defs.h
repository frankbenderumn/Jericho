#ifndef JERICHO_IRIS_DEFS_H_
#define JERICHO_IRIS_DEFS_H_

#include <utility>
#include <deque>
#include <unordered_map>
#include <vector>

#ifndef LEX
#define LEX std::pair<std::string, int>
#endif

#ifndef LEXES
#define LEXES std::deque<std::pair<std::string, int>>
#endif

#ifndef TOKS
#define TOKS std::deque<std::string>
#endif

typedef std::unordered_map<std::string, std::vector<std::unordered_map<std::string, const char *>>> Deltas;
typedef std::vector<std::unordered_map<std::string, const char *>> Delta;
typedef std::unordered_map<std::string, const char *> IrisAttrs;

#endif