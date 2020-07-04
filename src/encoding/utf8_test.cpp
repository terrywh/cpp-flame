
#include "utf8.hpp"
#include <iostream>

int main(int argc, char* argv[]) {

    std::string r = "中文", o;
    std::cout << r << " length: " << encoding::utf8::length(r) << std::endl;
    std::cout << r << " firstc: " << encoding::utf8::substr(r, 0, 1) << std::endl;
    return 0;
}