#include "codegenemitter.h"

CodeGenEmitter::CodeGenEmitter() {
    data_ << ".intel_syntax noprefix\n";
    data_ << ".section .data\n";
    text_ << ".section .text\n";
    text_ << ".globl _start\n";
    setContext(TEXT_CONTEXT);
}

std::string CodeGenEmitter::getOutput() {
    text_ << ".section .note.GNU-stack,\"\",@progbits\n";
    return data_.str() + text_.str();
}

void CodeGenEmitter::setContext(int ctx) {
    this->context_ = ctx;
}

void CodeGenEmitter::emit(std::string what) {
    if (context_ == DATA_CONTEXT) {
        data_ << what;
    } else if (context_ == TEXT_CONTEXT) {
        text_ << what;
    }
}

void CodeGenEmitter::emitLine(std::string what) {
    if (context_ == DATA_CONTEXT) {
        data_ << "    " << what << "\n";
    } else if (context_ == TEXT_CONTEXT) {
        text_ << "    " << what << "\n";
    }
}

void CodeGenEmitter::emitLabel(int label) {
    if (context_ == DATA_CONTEXT) {
        data_ << "label_" << label << ":\n";
    } else if (context_ == TEXT_CONTEXT) {
        text_ << "label_" << label << ":\n";
    }
}

void CodeGenEmitter::emitLabel(std::string label) {
    if (context_ == DATA_CONTEXT) {
        data_  << label << ":\n";
    } else if (context_ == TEXT_CONTEXT) {
        text_ << label << ":\n";
    }
}


void CodeGenEmitter::emitMov(std::string dest, std::string src) {
    emitLine("mov " + dest + ", " + src);
}


void CodeGenEmitter::emitLea(std::string dest, std::string src) {
    emitLine("lea " + dest + ", " + src);
}
// arithmetic

void CodeGenEmitter::emitAdd(std::string dest, std::string src) {
    emitLine("add " + dest + "," + src);
}

void CodeGenEmitter::emitSub(std::string dest, std::string src) {
    emitLine("sub " + dest + "," + src);
}

void CodeGenEmitter::emitInc(std::string dest) {
    emitLine("inc " + dest);
}

void CodeGenEmitter::emitDec(std::string dest) {
    emitLine("dec " + dest);
}

void CodeGenEmitter::emitIMul(std::string dest, std::string src) {
    emitLine("imul " + dest + ", " + src);
}

void CodeGenEmitter::emitDiv(std::string dest) {
    emitLine("div " + dest);
}

// logic

void CodeGenEmitter::emitAnd(std::string dest, std::string src) {
    emitLine("and " + dest + ", " + src);
}

void CodeGenEmitter::emitOr(std::string dest, std::string src) {
    emitLine("or " + dest + ", " + src);
}

void CodeGenEmitter::emitNot(std::string dest) {
    emitLine("not " + dest);
}

void CodeGenEmitter::emitSHR(std::string dest, char imm) {
    emitLine("shr " + dest + ", " + imm);
}

void CodeGenEmitter::emitSHR(std::string dest, std::string cl) {
    emitLine("shr " + dest + ", " + cl);
}

void CodeGenEmitter::emitSHL(std::string dest, char imm) {
    emitLine("shl " + dest + ", " + imm);
}

void CodeGenEmitter::emitSHL(std::string dest, std::string cl) {
    emitLine("shl " + dest + ", " + cl);
}

void CodeGenEmitter::emitSAR(std::string dest, char imm) {
    emitLine("sar " + dest + ", " + imm);
}

void CodeGenEmitter::emitSAR(std::string dest, std::string cl) {
    emitLine("sar " + dest + ", " + cl);
}

// Jumps

void CodeGenEmitter::emitJmp(int label) {
    emitLine("jmp " + std::string("label_") + std::to_string(label));
}

void CodeGenEmitter::emitJmp(std::string label) {
    emitLine("jmp " + label);
}

void CodeGenEmitter::emitCmp(std::string dest, std::string src) {
    emitLine("cmp " + dest + ", " + src);
}

void CodeGenEmitter::emitJe(int label) {
    emitLine("je " + std::string("label_") + std::to_string(label));
}

void CodeGenEmitter::emitJne(int label) {
    emitLine("jne " + std::string("label_") + std::to_string(label));
}

void CodeGenEmitter::emitJg(int label) {
    emitLine("jg " + std::string("label_") + std::to_string(label));
}

void CodeGenEmitter::emitJge(int label) {
    emitLine("jge " + std::string("label_") + std::to_string(label));
}

void CodeGenEmitter::emitJl(int label) {
    emitLine("jl " + std::string("label_") + std::to_string(label));
}

void CodeGenEmitter::emitJle(int label) {
    emitLine("jle " + std::string("label_") + std::to_string(label));
}

// stack

void CodeGenEmitter::emitCall(std::string funcName) {
    emitLine("call " + funcName);
}

void CodeGenEmitter::emitRet() {
    emitLine("ret");
}

void CodeGenEmitter::emitPush(std::string src) {
    emitLine("push " + src);
    stackDepth_++;
}

void CodeGenEmitter::emitPop(std::string dest) {
    emitLine("pop " + dest);
    stackDepth_--;
}

std::string CodeGenEmitter::getReg(CodeGenReg reg, int size) {
    switch(reg) {
        case CodeGenReg::RAX:
            switch(size) {
                case 64:
                    return "rax";
                case 32:
                    return "eax";
                case 16:
                    return "ax";
                case 8:
                    return "ah";
                case 0:
                    return "al";
                default: return "rax error";
            }
        case CodeGenReg::RBX:
            switch(size) {
                case 64:
                    return "rbx";
                case 32:
                    return "ebx";
                case 16:
                    return "bx";
                case 8:
                    return "bh";
                case 0:
                    return "bl";
                default: return "rbx error";
            }
        case CodeGenReg::RCX:
            switch(size) {
                case 64:
                    return "rcx";
                case 32:
                    return "ecx";
                case 16:
                    return "cx";
                case 8:
                    return "ch";
                case 0:
                    return "cl";
                default: return "rcx error";
            }
        case CodeGenReg::RDX:
            switch(size) {
                case 64:
                    return "rdx";
                case 32:
                    return "edx";
                case 16:
                    return "dx";
                case 8:
                    return "dh";
                case 0:
                    return "dl";
                default: return "rcx error";
            }
        case CodeGenReg::RSI: return "rsi";
        case CodeGenReg::RDI: return "rdi";
        case CodeGenReg::RBP: return "rbp";
        case CodeGenReg::RSP: return "rsp";
        case CodeGenReg::R8: return "r8";
        case CodeGenReg::R9: return "r9";
        case CodeGenReg::R10: return "r10";
        case CodeGenReg::R11: return "r11";
        case CodeGenReg::R12: return "r12";
        case CodeGenReg::R13: return "r13";
        case CodeGenReg::R14: return "r14";
        case CodeGenReg::R15: return "r15";
        default: return "-REG-";
    }
    return "error finding reg";
}

std::string CodeGenEmitter::getMem(std::string reg, int offset) {
    if (offset < 0) {
        return std::string("[" + reg + std::to_string(offset) + "]");
    } else if (offset == 0) {
        return std::string("[" + reg + "]");
    } else {
        return std::string("[" + reg + "+" + std::to_string(offset) + "]");
    }
}

std::string CodeGenEmitter::getMem(std::string base_reg, std::string offset_reg) {
    return std::string("[" + base_reg + "+" + offset_reg + "]");
}

unsigned int CodeGenEmitter::getLabel() {
    return labels_++;
}
