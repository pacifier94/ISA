#pragma once

#include <vector>
#include <string>
#include "gc.h"
#include "ast.h"   // <-- IRInst comes from here

class VM {
public:
    explicit VM(const std::vector<IRInst>& instrs);

    void run();
    void step();

    // GC hooks
    void gc_collect();
    size_t heap_size() const;

private:
    std::vector<IRInst> code;
    std::vector<int32_t> stack;
    std::vector<int32_t> memory;
    size_t pc = 0;

    GC gc;
    std::vector<GCObject*> roots;
};
