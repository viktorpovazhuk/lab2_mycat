//
// Created by vivi on 23.09.22.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

std::string gen_random_str(const size_t len, std::string &str) {
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    for (size_t i = 0; i < len; ++i) {
        str[i] = alphanum[std::rand() % (sizeof(alphanum) - 1)];
    }

    return str;
}

int main() {
    size_t str_len = 1e7;
    size_t nec_len = 2e9;

    std::ofstream myfile;
    myfile.open("extra_large_1.txt");
    std::string str;
    str.resize(str_len);
    size_t total_len = 0;
    while (total_len < nec_len) {
        str = gen_random_str(str_len, str);
        std::cout << str.size();
        myfile << str;
        total_len += str_len;
    }
    myfile.close();
    return 0;
}