#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "nastranToJson.hpp"

#ifdef _WIN32
#include <io.h>
#define access    _access_s
#else

#include <unistd.h>

#endif

bool FileExists(const std::string &Filename) {
    return access(Filename.c_str(), 0) == 0;
}


int main(int argc, char **argv) {
    if (argc < 4) {
        std::cout << "Call with two arguments as follows" << std::endl;
        std::cout << "./program_name nastran_file.nas json_file.fjson 1" << std::endl;
        std::cout << "The last argument should be 1 for binary and 2 for ascii" << std::endl;
    } else {
        const std::string nastran_file = argv[1];
        const std::string json_file = argv[2];
        unsigned int file_mode = boost::lexical_cast<unsigned int>(argv[3]);
        if (!boost::algorithm::ends_with(nastran_file, ".nas")) {
            std::cout << "Input Nastran file must have a .nas extension" << std::endl;
            return 0;
        }
        if (!boost::algorithm::ends_with(json_file, ".fjson")) {
            std::cout << "Input JSON file must have a .fjson extension" << std::endl;
            return 0;
        }
        if (FileExists(nastran_file)) {
            nastranToJson nas_converter(nastran_file);
            nas_converter.saveJson(json_file, file_mode);
        } else {
            std::cout << "Input Nastran file does not exist" << std::endl;
        }
    }
}