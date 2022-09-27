// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "options_parser.h"
#include "errors.h"

#include <iostream>
#include <ios>
#include <memory>
#include <algorithm>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstddef>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <iomanip>

using uniq_char_ptr = std::unique_ptr<char[]>;

void print_buffer(uniq_char_ptr &buf, size_t buf_size) {
    size_t total_wrote = 0;
    size_t cur_wrote;
    while (total_wrote < buf_size) {
        cur_wrote = write(STDOUT_FILENO, &buf[total_wrote], buf_size - total_wrote);
        if (cur_wrote == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("Error while printing buffer");
                exit(errno);
            }
        }
        total_wrote += cur_wrote;
    }
}

size_t read_buffer(int fd, uniq_char_ptr &buf_ptr, size_t buf_capacity) {
    size_t total_read = 0;
    size_t cur_read = 1;
    while (cur_read != 0 and total_read < buf_capacity) {
        cur_read = read(fd, buf_ptr.get(), buf_capacity - total_read);
        if (cur_read == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("Error while reading file in buffer");
                exit(errno);
            }
        }
        total_read += cur_read;
    }
    return total_read;
}

size_t convert_invisible_chars(uniq_char_ptr &buf_ptr, size_t buf_size, uniq_char_ptr &inv_chars_buf) {
    size_t len_with_repr = 0;
    for (size_t i = 0; i < buf_size; i++) {
        char cur_char = buf_ptr[i];
        if (!std::isprint(cur_char) && !std::isspace(cur_char)) {
            std::stringstream ss;
            ss << "\\x" << std::hex << std::uppercase << std::setw(2)
               << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(cur_char));
            std::string code = ss.str();
            memcpy(&inv_chars_buf[len_with_repr], code.c_str(), code.size());
            len_with_repr += code.size();
        } else {
            inv_chars_buf[len_with_repr] = cur_char;
            len_with_repr++;
        }
    }
    return len_with_repr;
}

int main(int argc, char *argv[]) {
    std::unique_ptr<command_line_options_t> command_line_options;
    try {
        command_line_options = std::make_unique<command_line_options_t>(argc, argv);
    }
    catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        exit(Errors::ECLOPTIONS);
    }
    std::vector<std::string> paths = command_line_options->files;
    bool print_inv = command_line_options->print_trans;

    const size_t buf_capacity = 10e6;
    auto buf_ptr = std::make_unique<char[]>(buf_capacity);
    auto inv_chars_buf_ptr = std::make_unique<char[]>(buf_capacity*4);

    std::vector<int> fds(paths.size());
    for (size_t i = 0; i < paths.size(); i++) {
        std::string &cur_path = paths[i];
        int cur_fd = open(cur_path.c_str(), O_RDONLY);
        if (cur_fd == -1) {
            perror(("Error while opening file: " + cur_path).c_str());
            exit(errno);
        }
        fds[i] = cur_fd;
    }

    for (int fd: fds) {
        size_t read_buf_size = buf_capacity;
        while (read_buf_size == buf_capacity) {
            read_buf_size = read_buffer(fd, buf_ptr, buf_capacity);
            if (print_inv) {
                size_t inv_buf_size = convert_invisible_chars(buf_ptr, read_buf_size, inv_chars_buf_ptr);
                print_buffer(inv_chars_buf_ptr, inv_buf_size);
            } else {
                print_buffer(buf_ptr, read_buf_size);
            }
        }
    }

    for (int fd: fds) {
        int res = close(fd);
        if (res == -1) {
            perror("Error while closing file");
            exit(errno);
        }
    }

    return 0;
}

// time ./mycat ../data/extra_large_1.txt > out.txt