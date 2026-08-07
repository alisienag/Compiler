#pragma once
#include "ast.h"
#include <iostream>

class Printer : public Visitor {
public:
    void print(ProgramNode& program) { program.accept(*this); }

    void visit(ProgramNode& p) override {
        indent(); std::cout << "Program\n";
        depth_++;
        for (auto& funcs : p.functions) funcs->accept(*this);
        depth_--;
    }
    
    void visit(FunctionNode& f) override {
        indent(); std::cout << "Func #" << f.nameIdx << " : " << typeName(f.type) << "\n";
        for (auto& op : f.operands)
            op->accept(*this);

        depth_++;
        f.statement->accept(*this);
        depth_--;
    }

    void visit(OperandNode& o) override {
        indent();
        std::cout << "Op " << o.identIdx << " type " << typeName(o.type) << "\n";
    }


    void visit(LetStatementNode& s) override {
        indent(); std::cout << "Let #" << s.idx << " : " << typeName(s.type) << "\n";
        depth_++;
        s.expr->accept(*this);
        depth_--;
    }

    void visit(BlockStatementNode& s) override {
        indent(); std::cout << " { }" << "\n";
        depth_++;
        for (auto& statements : s.statements) statements->accept(*this);
        depth_--;
    }
    
    void visit(ReassignStatementNode& s) override {
        indent(); std::cout << "#" << s.idx << " = " << "\n";
        depth_++;
        s.expr->accept(*this);
        depth_--;
    }

    void visit(ReturnStatementNode& s) override {
        indent(); std::cout << "return" << std::endl;
    }

    void visit(ExpressionStatementNode& s) override {
        depth_++;
        s.expr->accept(*this);
        depth_--;
    }

    void visit(BinaryExpressionNode& e) override {
        indent(); std::cout << e.op << std::endl;
        depth_++;
        e.l->accept(*this);
        e.r->accept(*this);
        depth_--;
    }

    void visit(TermExpressionNode& e) override {
        indent(); std::cout << "Number " << e.value << "\n";
    }

    void visit(CallExpressionNode& e) override {
        indent(); std::cout << " calling: " << e.value << "\n";
    }

private:
    int depth_ = 0;
    void indent() { for (int i = 0; i < depth_; i++) std::cout << "  "; }

    static const char* typeName(Type t) {
        switch (t) {
            case Type::i32:     return "i32";
            case Type::Unknown: return "unknown";
        }
        return "?";
    }
};
