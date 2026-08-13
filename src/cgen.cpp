#include "cgen.h"
#include "ast.h"
#include <iostream>
#include <algorithm>

#define align16(n) ( (n + 15) & ~15 )

std::string Cgen::getLocalLocation(int idx) {
    int index = 1; // this should never be positive
    int scope = currentScope;
    while (scope >= 0) {
        if(localIndex_.find(scope) != localIndex_.end()) {
            if (localIndex_[scope].find(idx) != localIndex_[scope].end()) {
                index = localIndex_[scope][idx];
                break;
            }
        }
        scope--;
    }
    if (index == 0) {
        return std::string("[rbp]");
    } else if (index < 0) {
        return std::string("[rbp" + std::to_string(index) + "]");
    } else {
        std::cerr << "Cgen Error: Error calculating local index! got " << index << "\n" << std::endl;
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
    assembly_ << ".globl _start\n";
    for (auto& f : program.functions) {
        f->accept(*this);
    }
    assembly_ << ".section .note.GNU-stack,\"\",@progbits\n";
    return this->data.str() + this->assembly_.str();
}

Type Cgen::visit(ProgramNode& p) {
    this->rFuncs = p.rFunctions;
    for (auto& functions : p.functions) {
        functions->accept(*this);
    }
    return Type::Void();
}

Type Cgen::visit(FunctionNode& p) {
    hasReturn = false;
    localIndex_.clear();
    operandIndex_.clear();
    isConst_.clear();
    firstIndex = -4;
    currentFunction = table_.findStringByIdx(p.nameIdx);
    std::size_t opCount = p.operands.size();
    int baseIdx = (8 * opCount) + 8; // for first
    for (auto& e : p.operands) {
        operandIndex_[e->identIdx] = baseIdx;
        baseIdx -= 8;
        if (baseIdx <= 0) {
            std::cout << "Cgen Error: operand index maths seems to be wrong\n" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    emitLabel("_" + currentFunction);
    emitPush(R_RBP);
    emitMov(R_RBP, R_RSP);
    assembly_ << "sub rsp, " << align16((8 * p.localCount)) << "\n"; // fix for number of locals
    emitMov(R_RAX, "0"); 
    p.statement->accept(*this);
    if (hasReturn == false) {
        emitMov(R_RAX, "0");
    }
    emitLabel(currentFunction + F_DONE);
    emitMov(R_RSP, R_RBP);
    emitPop(R_RBP);
    emitLine("ret");
    return Type::Void();
}

Type Cgen::visit(OperandNode&) {
    return Type::Void();
}

Type Cgen::visit(LetStatementNode& p) {
    p.expr->accept(*this);
    localIndex_[currentScope][p.idx] = firstIndex;
    isConst_[currentScope][p.idx] = p.isConst;
    firstIndex -= 8;
    emitPop(R_RAX);
    emitMov(getLocalLocation(p.idx), R_RAX);
    return Type::Void();
}

Type Cgen::visit(BlockStatementNode& b) {
    currentScope++;
    for (auto& stmt : b.statements) 
        stmt->accept(*this);
    localIndex_[currentScope].clear();
    isConst_[currentScope].clear();
    currentScope--;
    return Type::Void();
}

Type Cgen::visit(ReassignStatementNode& s) {
    s.expr->accept(*this);
    emitPop(R_RAX);
    if (auto* idx = dynamic_cast<IndexExpressionNode*>(s.lhs.get())) {
        emitPush(R_RAX);
        idx->indexExpr->accept(*this);
        idx->primaryExpr->accept(*this);
        emitPop(R_RBX);
        emitPop(R_RCX);
        int stride = typeSize(*idx->primaryExpr->type.element);
        emitLine("imul rcx, " + std::to_string(stride));
        emitLine("add rbx, rcx");
        emitPop(R_RAX);
        emitMov("[rbx]", R_RAX);
    } else {
        emitMov(getLocalLocation(s.lhs->value), R_RAX);
    }
    return Type::Void();
}

Type Cgen::visit(ReturnStatementNode& s) {
    hasReturn = true;
    s.expr->accept(*this);
    emitPop(R_RAX);
    if (currentFunction.compare("start") == 0) {
        emitMov(R_RDI, R_RAX);
        emitMov(R_RAX, SYS_EXIT);
        emitSyscall();
    } else {
        emitLine("jmp " + currentFunction + F_DONE);
    }
    return Type::Void();
}

Type Cgen::visit(ExpressionStatementNode& s) {
    s.expr->accept(*this);
    return Type::Void();
}

Type Cgen::visit(IfStatementNode& s) {
    s.cond->accept(*this);
    emitPop(R_RAX);
    std::string else_start = getFreshLabel();
    std::string else_end = getFreshLabel();
    emitCmp(R_RAX, "0");
    emitJe(else_start);
    s.ifNode->accept(*this);
    emitJmp(else_end);
    emitLabel(else_start);
    if (s.hasElse) {
        s.elseNode->accept(*this);
    }
    emitLabel(else_end);
    
    return Type::Void();
}

Type Cgen::visit(BinaryExpressionNode& e) {
    e.l->accept(*this);
    e.r->accept(*this);
    emitPop(R_RBX);
    emitPop(R_RAX);
    if (e.op == BinOp::Add) {
        emitLine("add rax, rbx");
    } else if (e.op == BinOp::Sub) {
        emitLine("sub rax, rbx");
    } else if (e.op == BinOp::Eq) {
        emitCmp(R_RAX, R_RBX);
        emitLine("sete al");
        emitLine("movzx rax, al");
    } else {
        std::cerr << "Cgen Error: Error finding op for binary operation!\n";
    }
    emitPush(R_RAX);
    return Type::Void();
}

Type Cgen::visit(TermExpressionNode& e) {
    if (e.isIdent) {
        if (operandIndex_.find(e.value) != operandIndex_.end()) {
            emitPush(getOperandLocation(e.value));
        } else {
            emitPush(getLocalLocation(e.value));
        }
        return Type::Void();
    }
    if (e.type.type == TypeKind::Array && e.type.element->type == TypeKind::u8 && e.isConst == true) {
        if (std::find(stringsEmitted.begin(), stringsEmitted.end(), e.value) == stringsEmitted.end()) {
            data << "string_" << e.value << ":\n";
            data << ".ascii \"" << table_.findStringByIdx(e.value) << "\"" << "\n";
            data << "len_" << e.value << " = . - " << "string_" << e.value << "\n";
            stringsEmitted.push_back(e.value);
        }
        emitLine("lea rax, [rip + string_" + std::to_string(e.value) + "]");
        emitPush(R_RAX);
        return Type::Void();
    }

    emitPush(e.value);
    
    return Type::Void();
}

Type Cgen::visit(CallExpressionNode& e) {
    int stackSlots = 0;
    for (std::size_t i = 0; i < e.operands.size(); i++) {
        stackSlots++; // For now, all operands take 8 bytes of space
    }
    int padding = ((stackDepth + stackSlots) % 2 == 0 ? 0 : 1);
    if (padding) {
        emitLine("sub rsp, 8");
        stackDepth++;
    }
    int before = stackDepth;
    for (auto& op : e.operands)
        op->accept(*this);
    int slotsTaken = stackDepth - before;
    if (slotsTaken != stackSlots) {
        std::cerr << "Cgen Warning: Slots Taken not equal to calculated stack slots!\n";
    }
    emitCall(e.value);

    int cleanUp = slotsTaken + padding;
    if (cleanUp > 0) {
        emitLine("add rsp, " + std::to_string(8*cleanUp));
        stackDepth -= cleanUp;
    }
    emitPush(R_RAX);
    return Type::Void();
}

Type Cgen::visit(CastExpressionNode& e) {
    e.expr->accept(*this);
    emitPop(R_RAX);
    switch(e.type.type) {
        case TypeKind::u8:
            emitLine("movzx rax, al");
            break;
        case TypeKind::i32:
            emitLine("movsx rax, eax");
            break;
        case TypeKind::i64:
            //already i64;
            break;
        case TypeKind::Array:
            //pointer to pointer cast nothing needed
            break;
        default:
            break;
    }
    emitPush(R_RAX);
    return Type::Void();
}

Type Cgen::visit(IndexExpressionNode& e) {
    e.indexExpr->accept(*this);
    e.primaryExpr->accept(*this);
    emitPop(R_RBX);
    emitPop(R_RCX);
    int stride = typeSize(*e.primaryExpr->type.element);
    emitLine("imul rcx, " + std::to_string(stride));
    emitLine("add rbx, rcx");
    emitMov(R_RAX, "[rbx]");
    emitPush(R_RAX);
    return Type::Void();
}
