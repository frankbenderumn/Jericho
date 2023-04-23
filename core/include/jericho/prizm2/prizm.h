#ifndef PRIZM2_PRIZM_H_ 
#define PRIZM2_PRIZM_H_ 

#include <unistd.h>

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <chrono>

#include "prizm/prizm.h"
#include "util/trace.h"

struct Meta {
    std::string file;
    int line;
};

struct Benchmark2 {
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> end;
    std::string test;
    std::string method;
    std::string name;
    double elapsed;
    Benchmark2(std::string test, std::string method, std::string name) {
        this->test = test;
        this->method = method;
        this->name = name;
        this->start = std::chrono::high_resolution_clock::now();
    }

    // double x = 1234.56789;
    // int digits = 1 + floor(log10(abs(x)));
    // std::cout << "Number of digits in whole part: " << digits << std::endl;

    void stop() {
        this->end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(this->end - this->start);
        this->elapsed = std::chrono::duration<double, std::nano>(elapsed_seconds).count();
    }
};

enum AssertionType2 {
    ASSERTION_STR,
    ASSERTION_PRIM,
    ASSERTION_CONTAINER,
    REFUTATION_STR,
    REFUTATION_PRIM,
    REFUTATION_CONTAINER
};

class Assertion2 {
    std::string _file;
    int _line;
    bool _eq;
    std::string _actual;
    std::string _expected;
    std::string _llabel;
    std::string _rlabel;
    AssertionType2 _type;
    std::string _test;
    std::string _method;
  public:
    template <typename L, typename R>
    Assertion2(std::string test, std::string method, AssertionType2 type, std::string llabel, std::string rlabel, L lvalue, R rvalue, const char* file, int line) {
        PCREATE;
        _eq = (lvalue == rvalue);
        _type = type;
        _line = line;
        _file = std::string(file);
        _actual = std::to_string(lvalue);
        _expected = std::to_string(rvalue);
        _test = test;
        _method = method;
        _llabel = llabel;
        _rlabel = rlabel;
    }
    ~Assertion2() { PDESTROY; }
    
    void dump_assertion() {
        printf("    [\033[1;35mASSERTION\033[0m]");
        printf(" @ %s:%i\n", _file.c_str(), (int)_line);
        printf("\t\033[1;33mactual   \033[0m\033[1;37m%s\033[0m: %s\n", _rlabel.c_str(), _actual.c_str());
        printf("\t\033[1;36mexpected \033[0m\033[1;37m%s\033[0m: %s\n", _llabel.c_str(), _expected.c_str());
    }

    void dump_refutation() {
        printf("    [\033[1;35mREFUTATION\033[0m]");
        printf(" @ %s:%i\n", _file.c_str(), (int)_line);
        printf("\t\033[1;33mactual:\033[0m\n\t\t\033[1;37m%s\033[0m: %s\n", _rlabel.c_str(), _actual.c_str());
        printf("\t\033[1;36mexpected:\033[0m\n\t\t\033[1;37m%s\033[0m: %s\n", _llabel.c_str(), _expected.c_str());
    }

    void dump() {
        switch (_type) {
            case ASSERTION_PRIM:
                if (!_eq) dump_assertion();
                break;
            case REFUTATION_PRIM:
                if (_eq) dump_refutation();
                break;
        }
    }

    bool passed(int& passed) {
        switch (_type) {
            case ASSERTION_PRIM:
                passed += _eq;
                if (_eq) return true;
                break;
            case REFUTATION_PRIM:
                passed += !_eq;
                if (!_eq) return true;
                break;
            default:
                return false;
                break;
        }
        return false;
    }
};

class ITest {
    std::vector<Assertion2*> _assertions;
    std::map<std::string, Benchmark2*> _benchmarks;
  protected:
    template <typename L, typename R>
    void registrate(std::string test, std::string method, AssertionType2 type, std::string llabel, std::string rlabel, L lvalue, R rvalue, const char* file, int line) {
        _assertions.push_back(new Assertion2(test, method, type, llabel, rlabel, lvalue, rvalue, file, line));
    }
    void bench(std::string test, std::string method, std::string name) {
        _benchmarks[name] = new Benchmark2(test, method, name);
    }
    void stop(std::string name) {
        _benchmarks[name]->stop();
    }
  public:
    std::string test = "null";
    std::string method = "null";
    ITest() {}
    ITest(std::string test, std::string method) : test(test), method(method) {}
    virtual ~ITest() {}
    virtual void precondition() = 0;
    virtual void postcondition() = 0;
    virtual void block() = 0;
    const int count() const { return (int)_assertions.size(); }
    void assertions(int& _globalPassed) {
        int _localPassed = 0;
        std::vector<Assertion2*> temp;
        for (auto& assertion : _assertions) {
            if (!assertion->passed(_localPassed)) {
                temp.push_back(std::move(assertion));
            } else { 
                delete assertion;
            }
        }
        _globalPassed += _localPassed;
        std::string name = "\033[1;34m" + test + "\033[0m::\033[1;37m" + method;
        if (_localPassed == _assertions.size()) {
            printf("%-48s\033[0m \033[1;32m[%i/%li] PASSED\033[0m\n", name.c_str(), _localPassed, _assertions.size());
        } else {
            printf("%-48s\033[0m \033[1;31m[%i/%li] FAILED\033[0m\n", name.c_str(), _localPassed, _assertions.size());
            for (auto& t : temp) {
                t->dump();
                delete t;
            }
        }
    }
    void benchmarks() {
        std::string name = "\033[1;34m" + test + "\033[0m::\033[1;37m" + method;
        for (auto bench : _benchmarks) {
            printf("%-48s \033[1;34m[%.4fns] %s\033[0m\n", name.c_str(), bench.second->elapsed, bench.second->name.c_str());
        }
    }
};

// Define a global registry for classes
class SuiteRegistry {
public:
    static SuiteRegistry& get() {
        static SuiteRegistry instance;
        return instance;
    }

    template <typename... Args>
    void registrate(std::string test, std::string method, const char* file, int line, ITest* (*create)(std::string, std::string)) {
        std::string name = test+"|"+method;
        _classes[name] = create;
        Meta m;
        m.file = std::string(file);
        m.line = line;
        _meta[name] = m;
    }

    ITest* create(std::string test, std::string method) {
        auto it = _classes.find(test+"|"+method);
        if (it != _classes.end()) {
            return it->second(test, method);
        } else {
            return nullptr;
        }
    }

    const std::map<std::string, ITest* (*)(std::string, std::string)> classes() const {
        return _classes;
    }

private:
    SuiteRegistry() {}
    std::map<std::string, Meta> _meta;
    std::map<std::string, ITest* (*)(std::string, std::string)> _classes;
};

// Define a helper class that registers the class with the registry
template<typename T>
class SuiteRegister {
public:
    SuiteRegister(std::string fixture, std::string method, const char* file, int line) {
        SuiteRegistry::get().registrate(fixture, method, file, line, &create);
    }
private:
    static ITest* create(std::string test, std::string method) {
        ITest* t = (ITest*)(new T());
        t->test = test;
        t->method = method;
        return t;
    }
};

#define PTEST(test, method) \
class test##method##__Test : test { \
    MTRACE(test##method##__Test); \
  protected: \
    virtual void block(); \
    static SuiteRegister<test##method##__Test> registry; \
}; \
SuiteRegister<test##method##__Test> \
    test##method##__Test::registry(#test, #method, __FILE__, __LINE__); \
void test##method##__Test::block()

#define PASSERT(lvalue, rvalue) \
this->registrate(this->test, this->method, ASSERTION_PRIM, #lvalue, #rvalue, lvalue, rvalue, __FILE__, __LINE__);

#define PREFUTE(lvalue, rvalue) \
this->registrate(this->test, this->method, REFUTATION_PRIM, #lvalue, #rvalue, lvalue, rvalue, __FILE__, __LINE__);

#define PBENCH(name) this->bench(this->test, this->method, #name);

#define PSTOP(name) this->stop(#name);

#define RUN_ALL \
    int PRIZM_GLOBAL_PASSED = 0; \
    int ASSERTION_COUNT = 0; \
    for (auto& entry : SuiteRegistry::get().classes()) { \
        std::string::size_type p = entry.first.find("|"); \
        ITest* exhibit = entry.second(entry.first.substr(0, p), entry.first.substr(p+1,entry.first.size()-p-1)); \
        exhibit->precondition(); \
        exhibit->block(); \
        exhibit->assertions(PRIZM_GLOBAL_PASSED); \
        exhibit->benchmarks(); \
        ASSERTION_COUNT += exhibit->count(); \
        exhibit->postcondition(); \
        delete exhibit; \
    } \
    if (PRIZM_GLOBAL_PASSED == ASSERTION_COUNT) { \
        printf("\033[1;34m%-30s\033[0m \033[1;32m[%i/%i] SUCCESS\033[0m\n", "TOTAL", PRIZM_GLOBAL_PASSED, ASSERTION_COUNT); \
    } else { \
        printf("\033[1;34m%-30s\033[0m \033[1;31m[%i/%i] FAILURE\033[0m\n", "TOTAL", PRIZM_GLOBAL_PASSED, ASSERTION_COUNT); \
    } \
    BMAG("\nCLEANUP:\n");

#define MTRACE(classname) \
  public: \
    classname() { PCREATE; } \
    ~classname() override { PDESTROY; }

#endif