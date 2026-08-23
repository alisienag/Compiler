#include "runtime.h"

namespace Runtime {
    static void add(Program& p, StringTable& table, const std::string& name,
            const std::string& asmName, Type ret, std::vector<Type> params) {
        auto f = std::make_unique<FunctionDecl>();
        f->nameIdx = table.addString(name);
        f->asmName = asmName;
        f->returnType = ret;
        f->body = nullptr;
        for (size_t i = 0; i < params.size(); i++) {
            f->params.push_back(std::make_unique<Param>(table.addString(
                            "_arg_" + std::to_string(i)), params[i], false));
        }
        p.functions.push_back(std::move(f));
    }
};

 void Runtime::install(Program& p, StringTable& table) {
    add(p, table, "print", "_print", Type::i64(),
            { Type::array(Type::u8()), Type::i64()});
    add(p, table, "malloc", "_malloc", Type::array(Type::voidType()),
            { Type::i64()});
    add(p, table, "free", "_free", Type::voidType(),
            { Type::array(Type::voidType())});
}
