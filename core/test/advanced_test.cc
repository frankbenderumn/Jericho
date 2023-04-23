#include "prizm/prizm.h"
#include "util/trace.h"

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>

std::string PUBLIC_DIRECTORY = "./public/frontend";

struct Meta {
    std::string file;
    int line;
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
    AssertionType2 _type;
    std::string _test;
    std::string _method;
  public:
    template <typename L, typename R>
    Assertion2(std::string test, std::string method, AssertionType2 type, L lvalue, R rvalue, const char* file, int line) {
        PCREATE;
        _eq = (lvalue == rvalue);
        _type = type;
        _line = line;
        _file = std::string(file);
        _actual = std::to_string(lvalue);
        _expected = std::to_string(rvalue);
        _test = test;
        _method = method;
    }
    ~Assertion2() { PDESTROY; }
    
    void dump_assertion() {
        BCYA("Assertion %s:%s\n", _test.c_str(), _method.c_str());
        CYA("\tequal    : %i\n",(int)_eq);
        CYA("\tactual   : %s\n",_actual.c_str());
        CYA("\texpected : %s\n",_expected.c_str());
        CYA("\ttype     : %i\n",(int)_type);
        CYA("\tline     : %i\n",(int)_line);
        CYA("\tfile     : %s\n",_file.c_str());
    }

    void dump_refutation() {
        BCYA("Refutation %s:%s\n", _test.c_str(), _method.c_str());
        CYA("\tequal    : %i\n",(int)_eq);
        CYA("\tactual   : %s\n",_actual.c_str());
        CYA("\texpected : %s\n",_expected.c_str());
        CYA("\ttype     : %i\n",(int)_type);
        CYA("\tline     : %i\n",(int)_line);
        CYA("\tfile     : %s\n",_file.c_str());
    }

    void dump(int& passed) {
        switch (_type) {
            case ASSERTION_PRIM:
                passed += (_eq);
                if (!_eq) dump_assertion();
                break;
            case REFUTATION_PRIM:
                passed += (!_eq);
                if (_eq) dump_refutation();
                break;
        }
    }
};

class ITest {
    std::vector<Assertion2*> _assertions;
    int _passed = 0;
  protected:
    template <typename L, typename R>
    void registrate(std::string test, std::string method, AssertionType2 type, L lvalue, R rvalue, const char* file, int line) {
        _assertions.push_back(new Assertion2(test, method, type, lvalue, rvalue, file, line));
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
    void assertions() {
        for (auto& assertion : _assertions) {
            assertion->dump(_passed);
            delete assertion;
        }
        BGRE("TOTAL [%i/%li]\n", _passed, _assertions.size());
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
    void registrate(std::string test, std::string method, const char* file, int line, Args&&... args) {
        std::string name = test + method;
        _classes[name] = std::bind(&create<Args..., std::forward<Args>(args)...);
        Meta m;
        m.file = std::string(file);
        m.line = line;
        _meta[name] = m;
        std::cout << "Registered " << name << std::endl;
    }

    ITest* create(std::string name) {
        auto it = _classes.find(name);
        if (it != _classes.end()) {
            return it->second(name, name);
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
    std::map<std::string, ITest* (*)(std::string, std::string, std::tuple<Args...>)> _classes;
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
class test##method : test { \
    MTRACE(test##method); \
  protected: \
    virtual void block(); \
    static SuiteRegister<test##method> registry; \
}; \
SuiteRegister<test##method> \
    test##method::registry(#test, #method, __FILE__, __LINE__); \
void test##method::block()

#define PASSERT(lvalue, rvalue) \
this->registrate(this->test, this->method, ASSERTION_PRIM, lvalue, rvalue, __FILE__, __LINE__);

#define RUN_ALL \
    for (auto& entry : SuiteRegistry::get().classes()) { \
        ITest* exhibit = entry.second(entry.first, entry.first); \
        exhibit->precondition(); \
        exhibit->block(); \
        exhibit->assertions(); \
        exhibit->postcondition(); \
        delete exhibit; \
    } \
    BMAG("\n\nCLEANUP:\n");

#define MTRACE(classname) \
  public: \
    classname() { PCREATE; } \
    ~classname() override { PDESTROY; }

class Exhibit {
    std::string _name;
    int _val;
  public:
    Exhibit(std::string name, int val) {
        PCREATE;
        _name = name;
        _val = val;
    }
    ~Exhibit() { PDESTROY; }
    void dump() {
        BCYA("Exhibit Dump\n");
        CYA("\tName: %s\n", _name.c_str());
        CYA("\tVal: %i\n", _val);
    }
};

class TestA : public ITest {
    MTRACE(TestA);
  protected:
    void precondition() {
        MAG("This is a precondition!: %i\n", a);
    }

    void postcondition() {
        MAG("This is a postcondition!: %i\n", b);
    }

    int a = 0;
    int b = 2;
};


// I have a creative idea for integration testing
class TestB : public ITest {
    MTRACE(TestB);
  protected:
    void precondition() {
        MAG("This is a precondition!: %i\n", a);
        ex = new Exhibit("museum", 5);
    }

    void postcondition() {
        BGRE("Deleting exhibit!: %i\n", b);
        delete ex;
    }

    Exhibit* ex;
    int a = 0;
    int b = 2;
};

PTEST(TestA, Constructor) {
    BYEL("Get some!\n");
    PASSERT(1, 1);
}

PTEST(TestA, Constructor2) {
    BYEL("No fucking way!\n");
    PASSERT(1, 1);
}

PTEST(TestB, Dump) {
    BYEL("Welcome to Prizm!\n");
    PASSERT(2, 1);
    // ex->dump();
}

int main() {
    TRACEMEM;
    RUN_ALL;
    return 0;
}