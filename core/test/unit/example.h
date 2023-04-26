#include "prizm2/prizm.h"

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
    }

    void postcondition() {
    }

    int a = 0;
    int b = 2;
};


// I have a creative idea for integration testing
class TestB : public ITest {
    MTRACE(TestB);
  protected:
    void precondition() {
        ex = new Exhibit("museum", 5);
    }

    void postcondition() {
        delete ex;
    }

    Exhibit* ex;
    int a = 0;
    int b = 2;
};

PTEST(TestA, Constructor) {
    PASSERT(1, 1);
}

PTEST(TestA, Constructor2) {
    PASSERT(1, 1);
    PBENCH(Hello);
    for (int i = 0; i < 20; i++) {
        int a = i + 3;
        a = a * a;
        a = a + 4;
        b = a - 5;
        b = b * b;
        a = a * b;
    }
    PSTOP(Hello);
}

PTEST(TestB, Dump) {
    PREFUTE(a, b);
    // ex->dump();
}