
#include "url.hpp"
// #include "../encoding/utf8.hpp"
#include <iostream>

int main(int argc, char* argv[]) {

    std::string r = "中文", o;

    o = encoding::url::encode(r);
    std::cout << r << " url encode: " << o << std::endl;
    r = encoding::url::decode(o);
    std::cout << o << " url decode: " << r << std::endl;

    return 0;
}