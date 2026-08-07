#include "stringtable.h"

StringTable::StringTable() {
    this->strings_.reserve(20);
}

int StringTable::addString(std::string string) {
    for (std::size_t i = 0; i < this->strings_.size(); i++) {
        const std::string& str = this->strings_.at(i);
        if (string.compare(str) == 0) {
            return i;
        }
    }
    this->strings_.push_back(string);
    return this->strings_.size()-1;
}

std::string StringTable::findStringByIdx(int idx) {
    std::string empty = "No string found in table";
    if (idx < static_cast<int>(this->strings_.size())) {
        return this->strings_.at(idx);
    }
    return empty;
}
