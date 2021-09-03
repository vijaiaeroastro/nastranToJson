#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <map>

class nastranToJson {
public:
    nastranToJson(const std::string _nastran_file_name) : nastran_file_name(_nastran_file_name) {};

    ~nastranToJson() {};

public:
    void saveJson(const std::string _json_file_name, unsigned int _file_mode);

private:
    template<typename T>
    std::string convertVectorToDelimitedString(std::vector<T> &inputString);

    template<typename T>
    std::string convertDequeToDelimitedString(std::deque<T> &inputString);

    void convertNastranToJson();

private:
    std::string nastran_file_name;
    std::string json_file_name;
    unsigned int json_output_mode;
};