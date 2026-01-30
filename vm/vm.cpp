#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <chrono>

#include "vm.h"

using namespace std;
using namespace std::chrono;

/* =======================
   Constructor / Destructor
   ======================= */

VM::VM(vector<uint8_t> bytecode) : code(bytecode) {
    heap = nullptr;
    numObjects = 0;
    pc = 0;
    running = true;
    debug = false;
    reset();
}

VM::~VM() {
    Obj* current = heap;
    while (current) {
        Obj* next = current->next;
        delete current;
        current = next;
    }
    heap = nullptr;
}

/* =======================
   VM Core
   ======================= */

void VM::reset() {
    operandStack.clear();
    callStack.clear();

    for (int i = 0; i < 1024; i++) {
        memory[i].type = VAL_INT;
        memory[i].i = 0;
    }

    pc = 0;
    running = true;
}

void VM::run() {
    while (running && pc < code.size()) {
        step();
    }
}

/* =======================
   Heap Allocation
   ======================= */
Obj* VM::allocObject(ObjType type) {
    Obj* obj = new Obj();
    obj->type = type;
    obj->marked = false;
    obj->next = heap;
    heap = obj;
    numObjects++;

    obj->left = obj->right = nullptr;
    obj->function = obj->env = nullptr;

    if (debug) {
        cout << "[ALLOC] object=" << obj
             << " type=" << type
             << " total=" << numObjects << endl;
    }

    return obj;
}


Obj* VM::new_pair(Obj* a, Obj* b) {
    Obj* o = allocObject(OBJ_PAIR);
    o->left = a;
    o->right = b;
    return o;
}

Obj* VM::new_function() {
    return allocObject(OBJ_FUNCTION);
}

Obj* VM::new_closure(Obj* fn, Obj* env) {
    Obj* o = allocObject(OBJ_CLOSURE);
    o->function = fn;
    o->env = env;
    return o;
}

/* =======================
   Garbage Collection
   ======================= */

void VM::mark(Obj* root) {
    vector<Obj*> stack;
    stack.push_back(root);

    while (!stack.empty()) {
        Obj* o = stack.back();
        stack.pop_back();

        if (!o || o->marked) continue;
        o->marked = true;

        if (o->type == OBJ_PAIR) {
            stack.push_back(o->left);
            stack.push_back(o->right);
        } else if (o->type == OBJ_CLOSURE) {
            stack.push_back(o->function);
            stack.push_back(o->env);
        }
    }
}

void VM::markRoots() {
    for (auto& v : operandStack)
        if (v.type == VAL_OBJ)
            mark(v.obj);

    for (int i = 0; i < 1024; i++)
        if (memory[i].type == VAL_OBJ)
            mark(memory[i].obj);
}

void VM::sweep() {
    Obj** curr = &heap;

    while (*curr) {
        if (!(*curr)->marked) {
            Obj* dead = *curr;
            *curr = dead->next;
            delete dead;
            numObjects--;
        } else {
            (*curr)->marked = false;
            curr = &(*curr)->next;
        }
    }
}

void VM::gc() {
    auto start = high_resolution_clock::now();
    size_t before = numObjects;

    markRoots();
    sweep();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "[GC] Collected: " << (before - numObjects)
         << " | Remaining: " << numObjects
         << " | Time: " << duration.count() << " us" << endl;
}

/* =======================
   Helpers
   ======================= */

int32_t VM::fetchInt32() {
    if (pc + 4 > code.size()) return 0;

    int32_t val =
        (static_cast<int32_t>(code[pc]) << 24) |
        (static_cast<int32_t>(code[pc + 1]) << 16) |
        (static_cast<int32_t>(code[pc + 2]) << 8) |
        (static_cast<int32_t>(code[pc + 3]));

    pc += 4;
    return val;
}

Value VM::safe_pop() {
    if (operandStack.empty()) {
        cerr << "Runtime Error: Stack Underflow\n";
        running = false;
        return { VAL_INT, {.i = 0} };
    }

    Value v = operandStack.back();
    operandStack.pop_back();
    return v;
}

/* =======================
   Instruction Execution
   ======================= */

void VM::step() {
    uint8_t opcode = code[pc++];

    switch (opcode) {

        case 0x01: { // PUSH
            operandStack.push_back({ VAL_INT, {.i = fetchInt32()} });
            break;
        }

        case 0x02: safe_pop(); break; // POP
        case 0x03: { // DUP
            if (!operandStack.empty())
                operandStack.push_back(operandStack.back());
            else running = false;
            break;
        }

        case 0x10: { // ADD
            int32_t b = safe_pop().i;
            int32_t a = safe_pop().i;
            operandStack.push_back({ VAL_INT, {.i = a + b} });
            break;
        }

        case 0x11: { // SUB
            int32_t b = safe_pop().i;
            int32_t a = safe_pop().i;
            operandStack.push_back({ VAL_INT, {.i = a - b} });
            break;
        }

        case 0x12: { // MUL
            int32_t b = safe_pop().i;
            int32_t a = safe_pop().i;
            operandStack.push_back({ VAL_INT, {.i = a * b} });
            break;
        }

        case 0x13: { // DIV
            int32_t b = safe_pop().i;
            int32_t a = safe_pop().i;
            if (b == 0) {
                cerr << "Division by zero\n";
                running = false;
                break;
            }
            operandStack.push_back({ VAL_INT, {.i = a / b} });
            break;
        }

        case 0x14: { // CMP
            int32_t b = safe_pop().i;
            int32_t a = safe_pop().i;
            int32_t r = (a < b) ? -1 : (a > b) ? 1 : 0;
            operandStack.push_back({ VAL_INT, {.i = r} });
            break;
        }

        case 0x20: { // JMP
            pc = fetchInt32();
            break;
        }

        case 0x21: { // JZ
            uint32_t addr = fetchInt32();
            if (safe_pop().i == 0) pc = addr;
            break;
        }

        case 0x22: { // JNZ
            uint32_t addr = fetchInt32();
            if (safe_pop().i != 0) pc = addr;
            break;
        }

        case 0x30: { // STORE
            uint32_t idx = fetchInt32();
            memory[idx] = safe_pop();
            break;
        }

        case 0x31: { // LOAD
            uint32_t idx = fetchInt32();
            operandStack.push_back(memory[idx]);
            break;
        }

        case 0x40: { // CALL
            callStack.push_back(pc);
            pc = fetchInt32();
            break;
        }

        case 0x41: { // RET
            pc = callStack.back();
            callStack.pop_back();
            break;
        }

        case 0x50: { // OUT
            cout << "OUT: " << safe_pop().i << endl;
            break;
        }

        case 0x60: { // CONS
            Value b = safe_pop();
            Value a = safe_pop();
            Obj* o = new_pair(
                a.type == VAL_OBJ ? a.obj : nullptr,
                b.type == VAL_OBJ ? b.obj : nullptr
            );
            operandStack.push_back({ VAL_OBJ, {.obj = o} });
            break;
        }

        case 0x61: { // GC
            gc();
            break;
        }

        case 0x62: { // NIL
            operandStack.push_back({ VAL_OBJ, {.obj = nullptr} });
            break;
        }

        case 0x70: { // NEW_FUNCTION
            operandStack.push_back({ VAL_OBJ, {.obj = new_function()} });
            break;
        }

        case 0x71: { // NEW_CLOSURE
            Value env = safe_pop();
            Value fn = safe_pop();
            operandStack.push_back({
                VAL_OBJ,
                {.obj = new_closure(fn.obj, env.obj)}
            });
            break;
        }

        case 0xFF:
            running = false;
            break;

        default:
            cerr << "Unknown opcode: " << (int)opcode << endl;
            running = false;
    }
}

/* =======================
   Standalone VM (disabled in shell)
   ======================= */

#ifndef GC_TEST
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./vm <file.bin>\n";
        return 1;
    }

    ifstream file(argv[1], ios::binary);
    vector<uint8_t> buffer(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );

    VM vm(buffer);
    vm.run();
    return 0;
}
#endif
