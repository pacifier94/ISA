#ifndef VM_H
#define VM_H

#include <vector>
#include <cstdint>
#include <vector>
#include <cstdint>
#include <cstddef>

/* =======================
   Object System
   ======================= */

enum ObjType {
    OBJ_PAIR,
    OBJ_FUNCTION,
    OBJ_CLOSURE
};

struct Obj {
    ObjType type;
    bool marked;
    Obj* next;

    Obj* left;
    Obj* right;
    Obj* function;
    Obj* env;
};

/* =======================
   Value Representation
   ======================= */

enum ValueType {
    VAL_INT,
    VAL_OBJ
};

struct Value {
    ValueType type;
    union {
        int32_t i;
        Obj* obj;
    };
};

/* =======================
   Virtual Machine
   ======================= */

class VM {
public:
    /* Execution state */
    std::vector<uint8_t> code;
    std::vector<Value> operandStack;
    std::vector<uint32_t> callStack;

    Value memory[1024];

    /* GC heap */
    Obj* heap;
    size_t numObjects;


    /* Control */
    uint32_t pc;
    bool running;
    bool debug = false;

    /* Constructor / Destructor */
    VM(std::vector<uint8_t> bytecode);
    ~VM();

    /* Core */
    void reset();
    void run();
    void step();

    /* Helpers */
    int32_t fetchInt32();
    Value safe_pop();

    /* Heap / GC */
    Obj* allocObject(ObjType type);
    Obj* new_pair(Obj* a, Obj* b);
    Obj* new_function();
    Obj* new_closure(Obj* fn, Obj* env);

    void mark(Obj* root);
    void markRoots();
    void sweep();
    void gc();
};

#endif // VM_H
