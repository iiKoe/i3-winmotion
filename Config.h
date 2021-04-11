#include <iostream>
#include <string>
#include <list>
#include <cstdlib>
#include <getopt.h>

struct Config {
    bool fullscreen_overlay = true;
    std::list<char> hint_keys = {'j','k','l','f','d','s','a'};
    std::string font = "monospace";
    int font_size = 100;
    bool hint_floating = true;

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
        std::cout << "Usage: " << std::endl;
    }

    Config() {}
    Config(int argc, char *argv[]) {
        int c;
        while (1) {
            int option_index = 0;
            static struct option long_options[] = {
                {"no-fullscreen-overlay",   no_argument,        NULL, 'n'},
                {"font",                    required_argument,  NULL, 'f'},
                {"font-size",               required_argument,  NULL, 's'},
                {"hint-keys",               required_argument,  NULL, 'k'},
                {"hint-floating",           required_argument,  NULL, 'w'},
                {0, 0, NULL, 0}
            };

            c = getopt_long(argc, argv, "nf:s:k:w", long_options, &option_index);
            if (c == -1) {
                break;
            }

            std::cout << "option: " << (char)c << std::endl;
            switch (c) {
                case 'n':
                    fullscreen_overlay = false;
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
                    hint_floating = false;
                    break;

                case '?':
                    print_usage();
                    exit(EXIT_FAILURE);
                    break;

                default:
                    printf("?? getopt returned character code 0%o ??\n", c);
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
