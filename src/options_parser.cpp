// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "options_parser.h"
#include <vector>
#include <string>
#include <utility>

namespace po = boost::program_options;

using std::string;

command_line_options_t::command_line_options_t() {
    general_opt.add_options()
            ("help,h",
             "Show help message")
            ("print_trans,A",
             "Print hex representation instead of transparent symbols")
            ("files",
             po::value<std::vector<std::string>>()->
             multitoken()->zero_tokens()->composing(),
             "Files to send in output");

    positional_opt.add("files", -1);
}

command_line_options_t::command_line_options_t(int ac, char **av) :
        command_line_options_t() // Delegate constructor
{
    parse(ac, av);
}

void command_line_options_t::parse(int ac, char **av) {
    try {
        po::parsed_options parsed = po::command_line_parser(ac, av)
                .options(general_opt)
                .positional(positional_opt)
                .run();
        po::store(parsed, var_map);
        notify(var_map);

        if (var_map.count("help")) {
            std::cout << general_opt << "\n";
            exit(EXIT_SUCCESS);
        }

        print_trans = var_map.count("print_trans") > 0;

        if (var_map.count("files") > 0) {
            files = var_map["files"].as<std::vector<std::string>>();
        }
        else {
            throw OptionsParseException("Specify files");
        }
    } catch (std::exception &ex) {
        throw OptionsParseException(ex.what()); // Convert to our error type
    }
}