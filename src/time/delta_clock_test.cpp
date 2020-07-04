#include "delta_clock.hpp"
#include <iostream>
using namespace flame;

int main(int argc, char* argv[]) {
    time::delta_clock x;
    std::cout << x.iso() << " => " << static_cast<std::int64_t>(x) << std::endl;
    return 0;
}