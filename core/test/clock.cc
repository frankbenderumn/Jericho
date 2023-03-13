#include "util/clock.h"
#include "prizm/prizm.h"

int main(int argc, char* argv[]) {
    PrizmInit("Clock");

    // Time is: 1675128615
    // Pretty: Mon, Jan 30, 2023 07:30:15 PM CST
    TEST(Clock, Now) {
        time_t now = Clock::now();
        printf("Time is: %ld\n", now);
        std::string pret = Clock::prettyDate(now);
        printf("Pretty: %s\n", pret.c_str());
        now = 1675128615;
        pret = Clock::prettyDate(now);
        ASSERT_STR(pret, "Mon, Jan 30, 2023 07:30:15 PM CST");
        BBLU("HERE\n");
    }

    TEST(Clock, Future) {
        auto now = Clock::now_chrono();
        now += Min(15);
        time_t time = Clock::sec(now);
        std::string pret = Clock::prettyDate(time);
        printf("15min Future: %s\n", pret.c_str());
        now += Hour(3);
        time = Clock::sec(now);
        pret = Clock::prettyDate(time);
        printf("3hr Future: %s\n", pret.c_str());
        now += Hour(72);
        time = Clock::sec(now);
        pret = Clock::prettyDate(time);
        printf("3day Future: %s\n", pret.c_str());
    }

    PrizmResults();
    PrizmCleanup();
    return 0;
}