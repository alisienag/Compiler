#pragma once

#include "ast.h"
#include "stringtable.h"

namespace Runtime {
    void install(Program& p, StringTable& table);
};
