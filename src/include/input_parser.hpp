#pragma once
#include <string>
#include <vector>

class InputParser {
public:
    InputParser(int &argc, char **argv);
    const std::string& getCmdOption(const std::string &option) const;
    bool cmdOptionExists(const std::string &option) const;
private:
    std::vector<std::string> tokens;
};
