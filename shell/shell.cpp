#include <iostream>
#include <unordered_map>
#include <string>

#include "vm.h"
#include "program.h"
#include "ast.h"
#include "ir.h"

// parser symbols
extern int yyparse();
extern FILE* yyin;
extern ASTNode* root;

static int next_pid = 1;
static std::unordered_map<int, Program*> programs;

void submit(const std::string& file) {
    FILE* f = fopen(file.c_str(), "r");
    if (!f) {
        std::cerr << "Cannot open file\n";
        return;
    }

    yyin = f;
    root = nullptr;

    if (yyparse() != 0 || !root) {
        std::cerr << "Parse failed\n";
        fclose(f);
        return;
    }

    IRContext ctx;
    root->compile(ctx);               // AST → IR/bytecode
VM* vm = new VM(ctx.instructions);

   // VM is mandatory

    Program* p = new Program();
    p->pid = next_pid++;
    p->vm = vm;

    programs[p->pid] = p;
    std::cout << "PID = " << p->pid << std::endl;

    fclose(f);
}

void run(int pid) {
    if (!programs.count(pid)) {
        std::cerr << "Invalid PID\n";
        return;
    }
    programs[pid]->vm->run();
}
int main() {
    std::string cmd;
    while (true) {
        std::cout << "ipshell> ";
        if (!(std::cin >> cmd)) break;

        if (cmd == "submit") {
            std::string file;
            std::cin >> file;
            submit(file);
        }
        else if (cmd == "run") {
            int pid;
            std::cin >> pid;
            run(pid);
        }
        else if (cmd == "exit") {
            break;
        }
        else if (cmd == "memstat") {
    int pid; std::cin >> pid;
    if (!programs.count(pid)) {
        std::cout << "Invalid PID\n";
    } else {
        std::cout << "Heap objects: "
                  << programs[pid]->vm->heap_size()
                  << std::endl;
    }
}
else if (cmd == "gc") {
    int pid; std::cin >> pid;
    if (!programs.count(pid)) {
        std::cout << "Invalid PID\n";
    } else {
        programs[pid]->vm->gc_collect();
        std::cout << "GC complete\n";
    }
}
else if (cmd == "leaks") {
    int pid; std::cin >> pid;
    if (!programs.count(pid)) {
        std::cout << "Invalid PID\n";
    } else {
        size_t n = programs[pid]->vm->heap_size();
        if (n == 0)
            std::cout << "No leaks detected\n";
        else
            std::cout << "Possible leaks: " << n << "\n";
    }
}

        else {
            std::cout << "Unknown command\n";
        }
    }
    return 0;
}
