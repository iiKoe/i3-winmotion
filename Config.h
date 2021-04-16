#include <iostream>
#include <string>
#include <list>
#include <cstdlib>
#include <getopt.h>

struct Config {
    bool fullscreen_overlay = false;
    std::list<char> hint_keys = {'j','k','l','f','d','s','a'};
    std::string font = "monospace";
    int font_size = 100;
    bool hint_floating = false;

    void print() {
        std::string hint_key_str;
        for (const auto h : hint_keys) {
            hint_key_str.push_back(h);
        }

        std::cout << "fullscreen overlay: " << (fullscreen_overlay ? "yes" : "no") << std::endl;
        std::cout << "hint keys: " << hint_key_str << std::endl;
        std::cout << "font: " << font << std::endl;
        std::cout << "font size: " << font_size << std::endl;
        std::cout << "hint floating: " << (hint_floating ? "yes" : "no") << std::endl;
    }

    void print_usage() {
        std::cout << "OVERVIEW: " << "i3-winmotion window visible window switcher" << std::endl;
        std::cout << std::endl;
        std::cout << "USAGE: i3-winmotion [options]" << std::endl;
        std::cout << std::endl;

        std::cout << " -k, --hint-keys <keys>       Hint key string used to generate hints (default: 'jklfdsa')" << std::endl;
        std::cout << " -o, --fullscreen-overlay     Add a slightly black transparent background" << std::endl;
        std::cout << " -f, --font <name>            Hint font (default: monospace)" << std::endl;
        std::cout << " -s, --font-size <size>       Hint font size (default: 100)" << std::endl;
        std::cout << " -w, --hint-floating          Draw hints on floating windows" << std::endl;
        std::cout << " -h, --help                   Display this help and exit" << std::endl;
    }

    Config() {}
    Config(int argc, char *argv[]) {
        int c;
        while (1) {
            int option_index = 0;
            static struct option long_options[] = {
                {"fullscreen-overlay",      no_argument,        NULL, 'o'},
                {"font",                    required_argument,  NULL, 'f'},
                {"font-size",               required_argument,  NULL, 's'},
                {"hint-keys",               required_argument,  NULL, 'k'},
                {"hint-floating",           no_argument,        NULL, 'w'},
                {"help",                    no_argument,        NULL, 'h'},
                {0, 0, NULL, 0}
            };

            c = getopt_long(argc, argv, "of:s:k:wh", long_options, &option_index);
            if (c == -1) {
                break;
            }

            switch (c) {
                case 'o':
                    fullscreen_overlay = true;
                    break;

                case 'f':
                    font = std::string(optarg);
                    break;

                case 's':
                    font_size = std::atoi(optarg);
                    break;

                case 'k':
                    hint_keys.clear();
                    while ((c = *optarg++) != '\0') {
                        hint_keys.push_back(c);
                    }
                    break;

                case 'w':
                    hint_floating = true;
                    break;

                case 'h':
                    print_usage();
                    exit(EXIT_SUCCESS);

                case '?':
                    print_usage();
                    exit(EXIT_FAILURE);
                    break;

                default:
                    std::cout << "getopt returned unknown character code: " << c << std::endl;
            }
        }

        if (optind < argc) {
            std::cout << "non-option ARGV-elements: ";
            while (optind < argc)
                std::cout << argv[optind++];
            std::cout << std::endl;
        }

    }
};
