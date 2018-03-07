// TODO add licence
// PARSER CODE BASED ON MDPSIM obtained from https://github.com/hlsyounes/mdpsim

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>

#include "problems.h"
using namespace ppddl_parser;

/* The parse function. */
extern int ppddl_parse();
/* File to parse. */
extern FILE* ppddl_in;
/* Name of current file. */
std::string current_file;
/* Level of warnings. */
int warning_level;
/* Verbosity level. */
int verbosity;

/* Parses the given file, and returns true on success. */
static bool read_file(const char* name) {
    ppddl_in = fopen(name, "r");
    if (ppddl_in == 0) {
        std::cerr << "mdpclient:" << name << ": " << strerror(errno)
                  << std::endl;
        return false;
    } else {
        current_file = name;
        bool success = (ppddl_parse() == 0);
        fclose(ppddl_in);
        return success;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Error: Wrong arguments. You must provide an argument with the path to the PPDDL file." << std::endl;
        exit(-1);
    }

    /* Set default verbosity. */
    verbosity = 2;
    /* Set default warning level. */
    warning_level = 1;

    if (read_file(argv[1])) {
        std::cout << "File parsed correctly" << std::endl;

        if (verbosity > 1) {
            /*
             * Display domains and problems.
             */
            std::cerr << "----------------------------------------"<< std::endl
                      << "domains:" << std::endl;
            for (Domain::DomainMap::const_iterator di = Domain::begin();
                 di != Domain::end(); di++) {
                std::cerr << std::endl << *(*di).second << std::endl;
            }
            std::cerr << "----------------------------------------"<< std::endl
                      << "problems:" << std::endl;
            for (Problem::ProblemMap::const_iterator pi = Problem::begin();
                 pi != Problem::end(); pi++) {
                std::cerr << std::endl << *(*pi).second << std::endl;
            }
            std::cerr << "----------------------------------------"<< std::endl;
        }
    }
    else std::cout << "Errors while parsing input file!" << std::endl;
}
