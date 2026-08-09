#pragma once
#include "ast.h"
#include <iostream>

class Printer : public Visitor {
public:
    void print(ProgramNode& program) { program.accept(*this); }

    Type visit(ProgramNode& p) override {
        indent(); std::cout << "Program\n";
        depth_++;
        for (auto& funcs : p.functions) funcs->accept(*this);
        depth_--;
        return Type::Unknown;
    }
    
    Type visit(FunctionNode& f) override {
        indent(); std::cout << "Func #" << f.nameIdx << " : " << typeName(f.type) << "\n";
        for (auto& op : f.operands)
            op->accept(*this);

        depth_++;
        f.statement->accept(*this);
        depth_--;
        return Type::Unknown;
    }

    Type visit(OperandNode& o) override {
        indent();
        std::cout << "Op " << o.identIdx << " type " << typeName(o.type) << "\n";
        return Type::Unknown;
    }


    Type visit(LetStatementNode& s) override {
        indent(); std::cout << "Let #" << s.idx << " : " << typeName(s.type) << "\n";
        depth_++;
        s.expr->accept(*this);
        depth_--;
        return Type::Unknown;
    }

    Type visit(BlockStatementNode& s) override {
        indent(); std::cout << " { }" << "\n";
        depth_++;
        for (auto& statements : s.statements) statements->accept(*this);
        depth_--;
        return Type::Unknown;
    }
    
    Type visit(ReassignStatementNode& s) override {
        indent(); std::cout << "#" << s.idx << " = " << "\n";
        depth_++;
        s.expr->accept(*this);
        depth_--;
        return Type::Unknown;
    }

    Type visit(ReturnStatementNode& s) override {
        indent(); std::cout << "return " << std::endl;
        return Type::Unknown;
    }

    Type visit(ExpressionStatementNode& s) override {
        depth_++;
        s.expr->accept(*this);
        depth_--;
        return Type::Unknown;
    }

    Type visit(BinaryExpressionNode& e) override {
        indent(); std::cout << e.op << std::endl;
        depth_++;
        e.l->accept(*this);
        e.r->accept(*this);
        depth_--;
        return Type::Unknown;
    }

    Type visit(TermExpressionNode& e) override {
        indent(); std::cout << "Number " << e.value << "\n";
        return Type::Unknown;
    }

    Type visit(CallExpressionNode& e) override {
        indent(); std::cout << " calling: " << e.value << "\n";
        return Type::Unknown;
    }

private:
    int depth_ = 0;
    void indent() { for (int i = 0; i < depth_; i++) std::cout << "  "; }
};
