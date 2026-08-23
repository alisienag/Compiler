#pragma once
#include <iostream>
#include <vector>
#include <string>

#include "ast.h"

struct Diagnostic {
    Span span;
    std::string message;
};

class Diagnostics {
    public:
        void error(Span s, std::string msg) { items_.push_back({s, std::move(msg)}); }
        bool hasErrors() const { return !items_.empty(); }
        const std::vector<Diagnostic>& items() const { return items_; }
        
        void print(const std::string& file) const {
            for (const auto& d : items_) {
                std::cerr << file << ':' << d.span.line << ';' << d.span.col << ": error: " << d.message << "\n";
            }
        }

        void listErrors() const {
            for (auto& d : this->items()) {
                std::cerr << "Error at line " << d.span.line << ", column " << d.span.col << ": " << d.message << std::endl;
            }
        }

        void clearErrors() {
            items_.clear();
        }
    private:
        std::vector<Diagnostic> items_;
};
