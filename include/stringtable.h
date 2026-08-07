#pragma once

#include <string>
#include <vector>

class StringTable {
    public:
    StringTable();
    int addString(std::string);
    std::string findStringByIdx(int);
    const std::vector<std::string>& getVector() { return this->strings_; }
    private:
    std::vector<std::string> strings_;
};
