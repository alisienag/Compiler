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
        return Type::Void();
    }
    
    Type visit(FunctionNode& f) override {
        indent(); std::cout << "Func #" << f.nameIdx << " : " << typeName(f.type) << "\n";
        for (auto& op : f.operands)
            op->accept(*this);

        depth_++;
        f.statement->accept(*this);
        depth_--;
        return Type::Void();
    }

    Type visit(OperandNode& o) override {
        indent();
        std::cout << "Op " << o.identIdx << " type " << typeName(o.type) << "\n";
        return Type::Void();
    }


    Type visit(LetStatementNode& s) override {
        indent(); std::cout << "Let #" << s.idx << " : " << typeName(s.type) << "\n";
        depth_++;
        s.expr->accept(*this);
        depth_--;
        return Type::Void();
    }

    Type visit(BlockStatementNode& s) override {
        indent(); std::cout << " { }" << "\n";
        depth_++;
        for (auto& statements : s.statements) statements->accept(*this);
        depth_--;
        return Type::Void();
    }
    
    Type visit(ReassignStatementNode& s) override {
        indent(); std::cout << "#" << s.lhs->value << " = " << "\n";
        depth_++;
        s.expr->accept(*this);
        depth_--;
        return Type::Void();
    }

    Type visit(ReturnStatementNode& s) override {
        indent(); std::cout << "return " << std::endl;
        depth_++;
        s.expr->accept(*this);
        depth_--;
        return Type::Void();
    }

    Type visit(ExpressionStatementNode& s) override {
        depth_++;
        s.expr->accept(*this);
        depth_--;
        return Type::Void();
    }

    Type visit(IfStatementNode& s) override {
        std::cout << "if\n";
        s.cond->accept(*this);
        depth_++;
        s.ifNode->accept(*this);
        if (s.hasElse)
            s.elseNode->accept(*this);
        depth_--;
        return Type::Void();
    }

    Type visit(BinaryExpressionNode& e) override {
        indent(); std::cout << opName(e.op) << std::endl;
        depth_++;
        e.l->accept(*this);
        e.r->accept(*this);
        depth_--;
        return Type::Void();
    }

    Type visit(TermExpressionNode& e) override {
        indent(); std::cout << "Number " << e.value << "\n";
        return Type::Void();
    }

    Type visit(CallExpressionNode& e) override {
        indent(); std::cout << " calling: " << e.value << "\n";
        return Type::Void();
    }
    
    Type visit(CastExpressionNode& e) override {
        indent(); std::cout << " casting to: " << typeName(e.type) << "\n";
        return Type::Void();
    }
    
    Type visit(IndexExpressionNode& e) override {
        e.primaryExpr->accept(*this);
        std::cout << "Indexed by:" << std::endl;
        depth_++;
        e.indexExpr->accept(*this);
        depth_--;
        return Type::Void();
    }

private:
    int depth_ = 0;
    void indent() { for (int i = 0; i < depth_; i++) std::cout << "  "; }
};
