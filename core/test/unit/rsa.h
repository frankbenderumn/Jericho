#include "prizm2/prizm.h"

class RWTTest : public ITest {
    MTRACE(RWTTest);
  protected:
    void precondition() {
    }

    void postcondition() {
    }

    int a = 0;
    int b = 2;
};

PTEST(RWTTest, Something) {
    PASSERT(a, 0);
    PBENCH(Okay);
    PSTOP(Okay);
}