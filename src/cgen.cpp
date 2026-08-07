#include "cgen.h"
#include "ast.h"
#include <iostream>

#define align16(n) ( (n + 15) & ~15 )

std::string Cgen::getLocalLocation(int idx) {
    int index = localIndex_[idx];
    if (index == 0) {
        return std::string("[rbp]");
    } else if (index < 0) {
        return std::string("[rbp" + std::to_string(index) + "]");
    } else {
        std::cerr << "Cgen Error: Error calculating local index!\n" << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::string Cgen::getOperandLocation(int idx) {
    int index = operandIndex_[idx];
    return std::string("[rbp+" + std::to_string(index) + "]");
}

std::string Cgen::generate(ProgramNode& program) {
    data << ".intel_syntax noprefix\n";
    data << ".section .data\n";
    //STRINGS
    assembly_ << ".section .text\n";
    assembly_ << ".globl main\n";
    for (auto& f : program.functions) {
        f->accept(*this);
    }
    assembly_ << ".section .note.GNU-stack,\"\",@progbits\n";
    return this->data.str() + this->assembly_.str();
}

void Cgen::visit(ProgramNode& p) {
    for (auto& functions : p.functions) {
        functions->accept(*this);
    }
}

void Cgen::visit(FunctionNode& p) {
    hasReturn = false;
    localIndex_.clear();
    operandIndex_.clear();
    firstIndex = -4;
    currentFunction = table_.findStringByIdx(p.nameIdx);
    if (currentFunction.compare("print") == 0) {
        return emitPrint();
    }

    std::size_t opCount = p.operands.size();
    int baseIdx = (8 * opCount) + 8; // for first
    for (auto& e : p.operands) {
        operandIndex_[e->identIdx] = baseIdx;
        baseIdx -= 8;
        if (baseIdx <= 0) {
            std::cout << "operand index maths seems to be wrong\n" << std::endl;
        }
    }
    emitLabel(currentFunction);
    emitPush(R_RBP);
    emitMov(R_RBP, R_RSP);
    assembly_ << "sub rsp, " << align16((4 * p.localCount)) << "\n"; // fix for number of locals
    emitMov(R_EAX, "0"); 
    p.statement->accept(*this);
    if (hasReturn == false) {
        emitMov(R_RAX, 0);
    }
    emitLabel(currentFunction + F_DONE);
    emitMov(R_RSP, R_RBP);
    emitPop(R_RBP);
    emitLine("ret");
    std::cout << "done first func";
}

void Cgen::visit(OperandNode& op) {
    emitPush(op.identIdx);
}

void Cgen::visit(LetStatementNode& p) {
    p.expr->accept(*this);
    localIndex_[p.idx] = firstIndex;
    firstIndex -= 4;
    emitPop(R_RAX);
    emitMov(getLocalLocation(p.idx), "eax");
}

void Cgen::visit(BlockStatementNode& b) {
    for (auto& stmt : b.statements) 
        stmt->accept(*this);
}

void Cgen::visit(ReassignStatementNode& s) {
    s.expr->accept(*this);
    emitPop(R_RAX);
    emitMov(getLocalLocation(s.idx), "eax");
}

void Cgen::visit(ReturnStatementNode& s) {
    hasReturn = true;
    s.expr->accept(*this);
    emitPop(R_RAX);
    if (currentFunction.compare("main") == 0) {
        emitMov(R_RDI, R_RAX);
        emitMov(R_RAX, SYS_EXIT);
        emitSyscall();
    } else {
        emitLine("jmp " + currentFunction + F_DONE);
    }
}

void Cgen::visit(ExpressionStatementNode& s) {
    s.expr->accept(*this);
}

void Cgen::visit(BinaryExpressionNode& e) {
    e.l->accept(*this);
    e.r->accept(*this);
    emitPop(R_RBX);
    emitPop(R_RAX);
    if (e.op == '+') {
        emitLine("add rax, rbx");
    } else if (e.op == '-') {
        emitLine("sub rax, rbx");
    } else {
        std::cerr << "Cgen Error: Error finding op for binary operation!\n";
    }
    emitPush(R_RAX);
}

void Cgen::visit(TermExpressionNode& e) {
    if (e.isIdent) {
        if (operandIndex_.find(e.value) != operandIndex_.end()) {
            emitPush(getOperandLocation(e.value));
        } else {
            emitPush(getLocalLocation(e.value));
        }
    } else if (e.type == Type::String) {
        data << "string_" << e.value << ":\n";
        data << ".ascii \"" << table_.findStringByIdx(e.value) << "\"" << "\n";
        data << "len_" << e.value << " = . - " << "string_" << e.value << "\n";
        emitLine("lea rax, [rip + string_" + std::to_string(e.value) + "]");
        emitPush(R_RAX);
        emitMov(R_RAX, "len_" + std::to_string(e.value));
        emitPush(R_RAX);
    } else {
        emitPush(e.value);
    }
}

void Cgen::visit(CallExpressionNode& e) {
    for (auto& e : e.operands) {
        e->accept(*this); // top of the stack
    }
    emitCall(e.value);
    emitPush(R_RAX);
}

void Cgen::emitPrint() {
    emitLabel("print");
    emitPush(R_RBP);
    emitMov(R_RBP, R_RSP);
    emitMov(R_RAX, "1");
    emitMov(R_RDI, "1");
    emitMov(R_RSI, "[rbp+24]");
    emitMov(R_RDX, "[rbp+16]");
    emitLine("syscall");
    emitMov(R_RAX, "0");
    emitMov(R_RSP, R_RBP);
    emitPop(R_RBP);
    emitLine("ret");
}

Cgen::Cgen(StringTable table) {
    this->table_ = table;
}
