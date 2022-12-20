#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <chrono>

#include "lr.hpp"
#include "io/io_impl.hpp"

std::string fileToString(const char* filename) {
    std::ifstream t(filename);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());

    return str;
}

int main(int argc, const char* argv[] ) {
    lr::Enviroment env(44100);

    std::string codes;
    for (int i = 1; i < argc; i++) {
        auto txt = fileToString(argv[i]);
        codes = codes + "\n" + txt;
    }

    auto rt = env.build(codes);
    rt.run();
}


