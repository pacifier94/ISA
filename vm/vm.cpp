#include "vm.h"
#include <iostream>

VM::VM(const std::vector<IRInst>& instrs)
    : code(instrs), memory(1024, 0) {
    // simulate one root object
    roots.push_back(gc.alloc());
}


void VM::step() {
    if (pc >= code.size()) return;

    const IRInst& ins = code[pc++];

    if (ins.op == "PUSH") {
        stack.push_back(ins.value);
    }
    else if (ins.op == "ADD") {
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(a + b);
    }
    else if (ins.op == "SUB") {
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(a - b);
    }
    else if (ins.op == "MUL") {
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(a * b);
    }
    else if (ins.op == "DIV") {
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(b != 0 ? a / b : 0);
    }
    else if (ins.op == "STORE") {
        memory[ins.value] = stack.back();
        stack.pop_back();
    }
    else if (ins.op == "LOAD") {
        stack.push_back(memory[ins.value]);
    }
    else if (ins.op == "JMP") {
        pc = ins.value;
    }
    else if (ins.op == "JZ") {
        int v = stack.back(); stack.pop_back();
        if (v == 0) pc = ins.value;
    }
    else if (ins.op == "CMP") {
        int b = stack.back(); stack.pop_back();
        int a = stack.back(); stack.pop_back();
        stack.push_back(a < b);
    }
}

void VM::run() {
    while (pc < code.size()) {
        step();
    }

    if (!stack.empty())
        std::cout << stack.back() << std::endl;
}
void VM::gc_collect() {
    gc.collect(roots);
}

size_t VM::heap_size() const {
    return gc.count();
}
