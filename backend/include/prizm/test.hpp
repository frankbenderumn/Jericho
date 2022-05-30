/* 
Frank Bender
Prizm software testing
*/

#ifndef TEST_H_
#define TEST_H_

#include "prizm/print_color.h"
#include "prizm/error.h"

#define typename(x) _Generic((x),                                                         \
        unsigned char: "unsigned char",                                                   \
        char: "char",                   signed char: "signed char",                       \
        short int: "short int",         unsigned short int: "unsigned short int",         \
        int: "int",                     unsigned int: "unsigned int",                     \
        long int: "long int",           unsigned long int: "unsigned long int",           \
        long long int: "long long int", unsigned long long int: "unsigned long long int", \
        float: "float",                 double: "double",                                 \
        long double: "long double",     char *: "pointer to char",                        \
        void *: "pointer to void",      int *: "pointer to int",                          \
        default: "other")

#ifdef CPP_VERSION

#include <vector>
#include <string>
#include <map>
#include <typeinfo>

enum TestType {
    TEST_UNIT,
    TEST_REGRESSION,
    TEST_INTEGRATION,
    TEST_MOCK,
    TEST_ASYNC,
    TEST_FIXTURE,
    TEST_INTEGRITY,
    TEST_UNDEFINED
};

bool assert_eq(int a, int b) { return (a == b); }
bool assert_eq(std::string a, std::string b) { return (a == b); }
bool assert_eq(float a, float b) { return (a == b); }
bool assert_eq(double a, double b) { return (a == b); }
bool assert_eq(bool a, bool b) { return (a == b); }
void test_f(float a, float b) {
    if (assert_eq(a, b)) {
        // Console::success("PASSED");
    } else {
        // Console::failure("FAILURE");
        printf("MockValue: %f\n", a);
        printf("Expected: %f\n", b);
    }
}

typedef union {
    int value_int;
    char* value_char;
    float value_float;
    double value_double;
    unsigned int value_uint;
    long value_long;
} MockValue ;

typedef bool (*Assertion)(MockValue);

typedef struct {
    enum TestType type;
    bool result;
    Assertion assertion; 
    char name[16];
    void run(MockValue v, void (*callback)(void)) {
        assertion(v);
        callback();
    }
} Test;

class Suite {
  public:
    Suite() {}
    ~Suite() { printf("Destroying Suite\n"); }
    void run(void) { MockValue v; for (auto t : _tests) t.assertion(v); }
    void setName(std::string s) { name = s; }
    const char* getName() const { return &name[0]; }
    void addTest(Test t) { _tests.push_back(t); _testsHash[std::string(t.name)] = &t; }
    void registerTest(std::string testName) { 
        if (exists(testName, _testsHash)) {
            //  Console::error("Test name already exists!"); exit(1); 
        }
    }

  private:
    bool exists(std::string name, std::map<std::string, Test*> map) {
        return !( map.find(name) == map.end() );
    }
    std::vector<Test> _tests;
    std::map<std::string, Test*> _testsHash;
    std::string name;
};

class SuiteManager {
  public:
    SuiteManager() {}
    ~SuiteManager() { printf("Destroying SuiteManager\n"); }
    void addSuite(Suite s) { _suites.push_back(s); _suitesHash[s.getName()] = &s; }
    const std::vector<Suite> getSuites() const { return _suites; }
    void run(void) { for (auto s : _suites) s.run(); }
    void registerSuite(std::string suiteName, std::string testName) {
        // if (!exists(suiteName, _suitesHash)) { Console::error("invalid map key"); exit(1); }
        Suite* s = _suitesHash[suiteName];
    }

  private:
    bool exists(std::string name, std::map<std::string, Suite*> map) {
        return !( map.find(name) == map.end() );
    }
    std::vector<Suite> _suites;
    std::map<std::string, Suite*> _suitesHash;
};

Stack testStack(5);

static int stackId = -1;

static int testCount = 0;
static int testPassed = 0;

SuiteManager suiteManager;

static std::string _suite = "free";
static std::string _instance = "free";

static std::vector<std::pair<std::string, int>> _suiteCount = {};
static std::vector<std::pair<std::string, int>> _suitePassed = {};


// next two functions added 50 ms
int get_from_map_by_key(std::vector<std::pair<std::string, int>> map, std::string name) {
    for (auto& e : map) {
         if (e.first == name) {
             return e.second;
         }
    }
    return 0;
}

void find_in_test_stack(std::vector<std::pair<std::string, int>>* map, std::string name) {
    int hit = 0;
    for (auto& e : *map) {
         if (e.first == name) {
             e.second++;
             hit = 1;
         }
    }
    if (!hit) {
        std::pair<std::string, int> p{name, 1};
        map->push_back(p);
    }
}

std::map<std::string, std::string> _nameMap = {
    {typeid(int).name(), "int"},
    {typeid(bool).name(), "bool"},
    {typeid(const char*).name(), "string"},
    {typeid(std::string).name(), "string"},
    {typeid(float).name(), "float"},
    {typeid(double).name(), "double"},
    {typeid(unsigned int).name(), "unsigned int"},
    // {typeid(VEC3F).name(), "vec3f"},
    {typeid(int*).name(), "int*"},
    {typeid(bool*).name(), "bool*"},
    {typeid(std::string*).name(), "string*"},
    {typeid(float*).name(), "float*"},
    {typeid(double*).name(), "double*"},
    {typeid(unsigned int*).name(), "unsigned int*"},
    // {typeid(VEC3F*).name(), "vec3f*"},
    // {typeid(VEC3I).name(), "vec3i"},
    // {typeid(VEC3I*).name(), "vec3i*"},
    // {typeid(VEC4F).name(), "vec4f"},
    // {typeid(VEC4F*).name(), "vec4f*"},
    // {typeid(VEC2F).name(), "vec2f"},
    // {typeid(VEC2F*).name(), "vec2f*"},
    // {typeid(VEC2I).name(), "vec2i"},
    // {typeid(VEC2I*).name(), "vec2i*"}
};

void push_suite(SuiteManager* manager, std::string instance, std::string testSuite, std::string testName) {
    _instance = testName;
    _suite = testSuite;
    // find_in_test_stack(testSuite);
    manager->registerSuite(testSuite, testName);
    testStack.push(++stackId);
}

template <typename T>
int compare(T l, T r) {
    if (typeid(l).name() != typeid(r).name()) {
        //  Console::error("Invalid type comparison!"); 
        return 0; 
    }
    return (l == r);
}

int compare(std::string l, const char* r) {
    std::string s(r);
    return (l == r);
}

int compare(const char* l, std::string r) {
    std::string s(l);
    return (l == r);
}

int compare(const char* l, const char* r) {
    std::string s(l);
    std::string t(r);
    return (s == t);
}

int compare(std::string l, std::string r) {
    return (l == r);
}

// int compare(glm::mat4 a, glm::mat4 b) {
//     return (a == b);
// }

void demo(std::string s) {
    printf("yabba dabba doo: %s\n", s.c_str());
}

template <typename T>
void print(T t) {
    std::cout << t << std::endl;
}

// template <glm::vec3>
// void print(VEC3F a) {
//     printf("<%f,%f,%f>\n", a.x, a.y, a.z);
// }

// void print(VEC4F a) {
//     printf("<%f,%f,%f,%f>\n", a.x, a.y, a.z, a.w);
// }

// void print(VEC2F a) {
//     printf("<%f,%f,%f>\n", a.x, a.y);    
// }

// void print(Mat4 m) {
//     m.print();
// }

// void print(MAT4F m) {
//     printf("|%4.f %4.f %4.f %4.f|\n", m[0][0], m[1][0], m[2][0], m[3][0]);
//     printf("|%4.f %4.f %4.f %4.f|\n", m[0][1], m[1][1], m[2][1], m[3][1]);
//     printf("|%4.f %4.f %4.f %4.f|\n", m[0][2], m[1][2], m[2][2], m[3][2]);
//     printf("|%4.f %4.f %4.f %4.f|\n", m[0][3], m[1][3], m[2][3], m[3][3]);
// }

void pass() {
    green(); bold();
    printf("[1/1 PASSED]\n");
    clearcolor();
}

void fail() {
    red(); bold();
    printf("[0/1 FAILED]\n");
    clearcolor();
}

template <typename T>
void assertion(T l, T r) {
    testCount++;
    find_in_test_stack(&_suiteCount, _suite);
    if (compare(l, r) > 0) {
        ++testPassed;
        find_in_test_stack(&_suitePassed, _suite);
        // std::string suiteName = "SUITE";
        printf("%-10s:: %-16s\t", _suite.c_str(), _instance.c_str());
        pass();

    } else {
        // std::string suiteName = "SUITE";
        printf("%-10s:: %-16s\t", _suite.c_str(), _instance.c_str());
        fail();
        cyan();
        printf("Returned: \n"); clearcolor();
        print(l);
        purple();
        printf("Expected: \n"); clearcolor();
        print(r);
        printf("\n");
        // printf("%s\n", typeid(l).name());
        // printf("%s\n", typeid(r).name());
    }
}

void stub() {
    ++testCount;
    find_in_test_stack(&_suiteCount, _suite);
    printf("%-10s:: %-16s\t", _suite.c_str(), _instance.c_str());
    yellow(); bold();
    printf("[STUB]\n");
    clearcolor();
}

template <typename T>
void falsify(T l, T r) {
    ++testCount;
    find_in_test_stack(&_suiteCount, _suite);
    if (compare(l, r) > 0) {
        // std::string suiteName = "SUITE";
        printf("%-10s:: %-16s\t", _suite.c_str(), _instance.c_str());
        fail();
        cyan(); printf("Operands are equal\n"); clearcolor();

    } else {
        ++testPassed;
        find_in_test_stack(&_suitePassed, _suite);
        // std::string suiteName = "SUITE";
        printf("%-10s:: %-16s\t", _suite.c_str(), _instance.c_str());
        pass();
    }
}

#define ASSERT(lOperand, rOperand) assertion(lOperand, rOperand);

#define ASSERT_EQ(lOperand, rOperand) assertion(lOperand, rOperand);

#define REFUTE(lOperand, rOperand) falsify(lOperand, rOperand);

#define ASSERT_NE(lOperand, rOperand) falsify(lOperand, rOperand);

#define TEST(testSuite, testName) push_suite(&suiteManager, _instance, testSuite, testName);

#define STUB stub();

#endif

#endif