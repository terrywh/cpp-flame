
#include "hex.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string r = "中文", o;
    
    o = encoding::hex::encode(r);
    std::cout << r << " bin2hex: " << o << std::endl;
    r = encoding::hex::decode(o);
    std::cout << o << " hex2bin: " << r << std::endl;
    return 0;
}