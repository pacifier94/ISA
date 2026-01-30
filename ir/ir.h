#pragma once
#include <vector>

enum IROp {
    IR_PUSH,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_JMP,
    IR_JZ
};

struct IRInstr {
    IROp op;
    int arg;
    int line;
};
