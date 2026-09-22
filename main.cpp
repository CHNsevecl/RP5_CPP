#include "mission.hpp"


int main() {
    

    std::thread t1(mission1);
    std::thread t2(mission2);

    t1.join();
    t2.join();
    return 0;
}