#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class StringTable {
    public:
    StringTable();
    int addString(const std::string&);
    const std::string& getName(int);
    private:
    std::vector<std::string> names_;
    std::unordered_map<std::string, int> map_;
};
