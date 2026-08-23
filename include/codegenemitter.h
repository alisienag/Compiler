#pragma once

#include <sstream>

#define DATA_CONTEXT 0
#define TEXT_CONTEXT 1

enum class CodeGenReg {
    RAX, EAX, AX, AH, AL,
    RBX, EBX, BX, BH, BL,
    RCX, ECX, CX, CH, CL,
    RDX, EDX, DX, DH, DL,
    RSI,
    RDI,
    RBP,
    RSP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
};

class CodeGenEmitter {
    public:
        CodeGenEmitter();
        
        void setContext(int ctx);
        std::string getOutput();

        //emitter functions
        void emit(std::string what);
        void emitLine(std::string what);
        void emitLabel(int label);
        void emitLabel(std::string label);

        void emitMov(std::string dest, std::string src);
        void emitLea(std::string dest, std::string src);

        //arithmetic
        void emitAdd(std::string dest, std::string src);
        void emitSub(std::string dest, std::string src);
        void emitInc(std::string dest);
        void emitDec(std::string dest);
        void emitIMul(std::string dest, std::string src);
        void emitDiv(std::string dest);
        //logic
        void emitAnd(std::string dest, std::string src);
        void emitOr(std::string dest, std::string src);
        void emitNot(std::string dest);
        void emitSHR(std::string dest, char imm);
        void emitSHR(std::string dest, std::string cl); //lower 8 bits of a reg
        void emitSHL(std::string dest, char imm);
        void emitSHL(std::string dest, std::string cl); //lower 8 bits of a reg
        void emitSAR(std::string dest, char imm);
        void emitSAR(std::string dest, std::string cl); //lower 8 bits of a reg
        //Jumps
        void emitJmp(int label);
        void emitJmp(std::string label);
        void emitCmp(std::string dest, std::string src);
        void emitJe(int label);
        void emitJne(int label);
        void emitJg(int label);
        void emitJge(int label);
        void emitJl(int label);
        void emitJle(int label);
        //Stack
        void emitCall(std::string funcName);
        void emitRet();
        void emitPush(std::string src); //64 bit reg only
        void emitPop(std::string dest); //64 bit reg only

        std::string getReg(CodeGenReg reg, int size = 64);
        std::string getMem(std::string reg, int offset);
        std::string getMem(std::string base_reg, std::string offset_reg);
        unsigned int getLabel();
    private:
        unsigned int labels_ = 1;
        int context_ = 0;
        int stackDepth_ = 0;
        std::stringstream data_;
        std::stringstream text_;
};
