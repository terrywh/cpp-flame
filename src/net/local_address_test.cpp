#include "local_address.hpp"
#include <iostream>
using namespace flame;

int main(int argc, char* argv[]) {
    std::cout << net::local_address().to_string() << std::endl;
    return 0;
}