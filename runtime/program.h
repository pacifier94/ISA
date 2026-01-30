#pragma once

#include <vector>
#include <string>

#include "ast.h"
#include "ir.h"
#include "vm.h"   // ← REQUIRED

enum ProgramState {
    SUBMITTED,
    PARSED,
    READY,
    RUNNING,
    PAUSED,
    TERMINATED
};

struct Program {
    int pid;
    std::string filename;
    ProgramState state;

    ASTNode* ast = nullptr;           // Lab 2
    std::vector<IRInstr> ir;           // Lab 3
    VM* vm = nullptr;                  // Lab 4 + 5
};
