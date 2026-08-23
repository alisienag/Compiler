#pragma once

#include "ast.h"

class SymbolArena {
    public:
        Symbol* make() {
            owned_.push_back(std::make_unique<Symbol>());
            return owned_.back().get();
        }
    private:
    std::vector<std::unique_ptr<Symbol>> owned_;
};
