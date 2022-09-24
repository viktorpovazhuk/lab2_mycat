#include "options_parser.h"

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

void print_buffer(const char *buf, size_t buf_size) {
    size_t total_wrote = 0;
    size_t cur_wrote;
    while (total_wrote < buf_size) {
        cur_wrote = write(STDOUT_FILENO, buf + total_wrote, buf_size - total_wrote);
        if (cur_wrote == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror(nullptr);
                exit(errno);
            }
        }
        total_wrote += cur_wrote;
    }
}

size_t read_buffer(int fd, char *buf, size_t buf_capacity) {
    size_t total_read = 0;
    size_t cur_read = 1;
    while (cur_read != 0 and total_read < buf_capacity) {
        cur_read = read(fd, static_cast<void *>(buf), buf_capacity - total_read);
        if (cur_read == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror(nullptr);
                exit(errno);
            }
        }
        total_read += cur_read;
    }
    return total_read;
}

size_t convert_invisible_chars(const char *buf, size_t buf_size, char *inv_chars_buf) {
    size_t len_with_repr = 0;
    for (size_t i = 0; i < buf_size; i++) {
        char cur_char = buf[i];
        if (!std::isprint(cur_char) && !std::isspace(cur_char)) {
            std::stringstream ss;
            ss << "\\x" << std::hex << std::uppercase << std::setw(2)
               << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(cur_char));
            std::string code = ss.str();
            memcpy(inv_chars_buf + len_with_repr, code.c_str(), code.size());
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
        exit(EXIT_FAILURE);
    }
    std::vector<std::string> paths = command_line_options->files;
    bool print_inv = command_line_options->print_trans;

    std::vector<int> fds(paths.size());
    for (size_t i = 0; i < paths.size(); i++) {
        std::string &cur_path = paths[i];
        int cur_fd = open(cur_path.c_str(), O_RDONLY);
        if (cur_fd == -1) {
            perror(cur_path.c_str());
            exit(errno);
        }
        fds[i] = cur_fd;
    }

    const size_t buf_capacity = 10e6;
    static char buf[buf_capacity];
    static char inv_chars_buf[buf_capacity * 4];
    for (int fd: fds) {
        size_t read_buf_size = buf_capacity;
        while (read_buf_size == buf_capacity) {
            read_buf_size = read_buffer(fd, buf, buf_capacity);
            if (print_inv) {
                size_t inv_buf_size = convert_invisible_chars(buf, read_buf_size, inv_chars_buf);
                print_buffer(inv_chars_buf, inv_buf_size);
            } else {
                print_buffer(buf, read_buf_size);
            }
        }
    }

    for (int fd: fds) {
        int res = close(fd);
        if (res == -1) {
            perror(nullptr);
            exit(errno);
        }
    }

    return 0;
}

// ./mycat extra_large_1.txt > out.txt