#include "stringtable.h"

StringTable::StringTable() {
    this->names_.reserve(20);
}

int StringTable::addString(const std::string& s) {
    auto it = map_.find(s);
    if (it != map_.end()) return it->second;
    int idx = static_cast<int>(names_.size());
    names_.push_back(s);
    map_.emplace(s, idx);
    return idx;
}

const std::string& StringTable::getName(int idx) {
    return names_[idx];
}
