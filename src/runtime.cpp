#include "runtime.h"

void Runtime::implement(ProgramNode& p) {
    RuntimeFunc print;
    print.name = "print";
    print.retType = Type::i64();
    print.args.push_back(Type::array(Type::u8()));
    print.args.push_back(Type::i64());
    p.rFunctions.push_back(print);
    RuntimeFunc malloc;
    malloc.name = "malloc";
    malloc.retType = Type::array(Type::Void());
    malloc.args.push_back(Type::i64());
    p.rFunctions.push_back(malloc);
    RuntimeFunc free;
    free.name = "free";
    free.retType = Type::i64();
    free.args.push_back(Type::array(Type::Void()));
    p.rFunctions.push_back(free);
}
