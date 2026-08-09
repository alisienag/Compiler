#include "typecheck.h"
#include "ast.h"
#include <iostream>

Type TypeCheck::visit(ProgramNode& p) {
    errors = 0;
    for (auto& funcs : p.functions) {
        funcs->accept(*this);
    }
    return Type::Unknown;
}
Type TypeCheck::visit(FunctionNode& f) {
    funcType[f.nameIdx] = f.type;
    for (auto& op : f.operands)
        funcOpCount[f.nameIdx].push_back(op->accept(*this));
    Type stmtType = f.statement->accept(*this);
    if (stmtType != f.type) {
        std::cerr << "TypeCheck Error: function << " << table_.findStringByIdx(f.nameIdx) << " of return type " << typeName(f.type) << " cannot return type of " << typeName(stmtType) << "\n";
        errors++;
    }
    return f.type;
}
Type TypeCheck::visit(OperandNode& op) {
    varType[op.identIdx] = op.type;
    return op.type;
}

Type TypeCheck::visit(LetStatementNode& s) {
    Type exprType = s.expr->accept(*this);
    if (exprType != s.type) {
        std::cerr << "TypeCheck Error: cannot assign type " << typeName(exprType) << " to variable of type " << typeName(s.type) << "\n";
        errors++;
    }
    varType[s.idx] = s.type;
    return s.type;
}
Type TypeCheck::visit(BlockStatementNode& s) {
    Type lastStmtType = Type::Unknown;
    for (auto& stmt : s.statements)
        lastStmtType = stmt->accept(*this);
    return lastStmtType;
}

Type TypeCheck::visit(ReassignStatementNode& s) {
    Type exprType = s.expr->accept(*this);
    if (exprType != varType[s.idx]) {
        std::cerr << "TypeCheck Error: cannot reassign type " << typeName(exprType) << " to variable of type " << typeName(varType[s.idx]);
        errors++;
    }
    return varType[s.idx];
}

Type TypeCheck::visit(ReturnStatementNode& s) {
    return s.expr->accept(*this);
}
Type TypeCheck::visit(ExpressionStatementNode& s) {
    return s.expr->accept(*this);
}

Type TypeCheck::visit(BinaryExpressionNode& e) {
    Type lType = e.l->accept(*this);
    Type rType = e.r->accept(*this);
    if (lType != rType) {
        std::cerr << "TypeCheck Error: cannot perform " << e.op << " operation on differing types!\n";
        errors++;
    }
    return lType;
}
Type TypeCheck::visit(TermExpressionNode& e) {
    if (e.isIdent) {
        return varType[e.value];
    } else {
        return e.type;
    }
}
Type TypeCheck::visit(CallExpressionNode& e) {
    const std::vector<Type>& ops = funcOpCount[e.value];
    if (ops.size() != e.operands.size()) {
        std::cerr << "TypeCheck Error: expected " << ops.size() << " operands but got " << e.operands.size() << "!\n";
        errors++;
    }
    for (std::size_t i = 0; i < e.operands.size(); i++) {
        Type expected = ops.at(i);
        Type got = e.operands.at(i)->accept(*this);
        if(expected != got) {
            std::cerr << "TypeCheck Error: expected operand " << i+1 << " type " << typeName(expected) << " but got type " << typeName(got) << "!\n";
        errors++;
        }
    }
    return funcType[e.value];
}
