#ifndef AST_H
#define AST_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <cstdint>

extern int line_num;
extern std::map<std::string, double> symbolTable;

/* =========================
   LAB 3: Intermediate Representation
   ========================= */

struct IRInst {
    std::string op;   // PUSH, ADD, JZ, JMP, STORE, LOAD, etc.
    int32_t value;    // Literal or address
    int line;         // Source line (Lab 2 metadata)
};

struct IRContext {
    std::vector<IRInst> instructions;
    std::map<std::string, int> varMap;
    int nextAddr = 0;

    void emit(const std::string& op, int32_t val, int ln) {
        instructions.push_back({op, val, ln});
    }

    int currentPos() const {
        return instructions.size();
    }
};

/* =========================
   AST Base Class
   ========================= */

class ASTNode {
public:
    int lineNum;

    ASTNode() : lineNum(line_num) {}
    virtual ~ASTNode() {}

    virtual double eval() = 0;              // Lab 2/3 semantic checking
    virtual void compile(IRContext& ctx) = 0; // Lab 3 lowering
};

/* =========================
   Expression Nodes
   ========================= */

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double v) : value(v) {}

    double eval() override { return value; }

    void compile(IRContext& ctx) override {
        ctx.emit("PUSH", static_cast<int32_t>(value), lineNum);
    }
};

class IdentifierNode : public ASTNode {
    std::string name;
public:
    IdentifierNode(const std::string& n) : name(n) {}

    std::string getName() const { return name; }

    double eval() override {
        if (!symbolTable.count(name))
            throw std::runtime_error(
                "Runtime Error at line " + std::to_string(lineNum) +
                ": Use of undeclared variable " + name
            );
        return symbolTable[name];
    }

    void compile(IRContext& ctx) override {
        if (!ctx.varMap.count(name))
            throw std::runtime_error(
                "Compile Error at line " + std::to_string(lineNum) +
                ": Use of undeclared variable " + name
            );
        ctx.emit("LOAD", ctx.varMap[name], lineNum);
    }
};

/* =========================
   Binary Operations & Assignment
   ========================= */

class BinaryOpNode : public ASTNode {
    std::string op;
    ASTNode *l, *r;
public:
    BinaryOpNode(const std::string& o, ASTNode* a, ASTNode* b)
        : op(o), l(a), r(b) {}

    ~BinaryOpNode() {
        delete l;
        delete r;
    }

    double eval() override {
        if (op == "DECLARE") {
            symbolTable[static_cast<IdentifierNode*>(l)->getName()] = 0;
            return 0;
        }

        if (op == "DECLARE_INIT") {
            double v = r->eval();
            symbolTable[static_cast<IdentifierNode*>(l)->getName()] = v;
            return v;
        }

        if (op == "=") {
            double v = r->eval();
            symbolTable[static_cast<IdentifierNode*>(l)->getName()] = v;
            return v;
        }

        double a = l->eval();
        double b = r->eval();

        if (op == "+") return a + b;
        if (op == "-") return a - b;
        if (op == "*") return a * b;
        if (op == "/") return b != 0 ? a / b : 0;
        if (op == "<") return a < b;
        if (op == ">") return a > b;
        if (op == "==") return a == b;
        if (op == "!=") return a != b;

        return 0;
    }

    void compile(IRContext& ctx) override {
        if (op == "DECLARE" || op == "DECLARE_INIT") {
            std::string id = static_cast<IdentifierNode*>(l)->getName();
            ctx.varMap[id] = ctx.nextAddr++;

            if (op == "DECLARE_INIT") {
                r->compile(ctx);
                ctx.emit("STORE", ctx.varMap[id], lineNum);
            }
            return;
        }

        if (op == "=") {
            r->compile(ctx);
            std::string id = static_cast<IdentifierNode*>(l)->getName();
            ctx.emit("STORE", ctx.varMap[id], lineNum);
            return;
        }

        l->compile(ctx);
        r->compile(ctx);

        if (op == "+") ctx.emit("ADD", 0, lineNum);
        else if (op == "-") ctx.emit("SUB", 0, lineNum);
        else if (op == "*") ctx.emit("MUL", 0, lineNum);
        else if (op == "/") ctx.emit("DIV", 0, lineNum);
        else if (op == "<" || op == ">" || op == "==" || op == "!=")
            ctx.emit("CMP", 0, lineNum);
    }
};

/* =========================
   Control Flow Nodes
   ========================= */

class IfNode : public ASTNode {
    ASTNode *cond, *thenB, *elseB;
public:
    IfNode(ASTNode* c, ASTNode* t, ASTNode* e)
        : cond(c), thenB(t), elseB(e) {}

    ~IfNode() {
        delete cond;
        delete thenB;
        if (elseB) delete elseB;
    }

    double eval() override {
        return cond->eval() ? thenB->eval()
                            : (elseB ? elseB->eval() : 0);
    }

    void compile(IRContext& ctx) override {
        cond->compile(ctx);
        int jzPos = ctx.currentPos();
        ctx.emit("JZ", 0, lineNum);

        thenB->compile(ctx);

        if (elseB) {
            int jmpEnd = ctx.currentPos();
            ctx.emit("JMP", 0, lineNum);

            ctx.instructions[jzPos].value = ctx.currentPos();
            elseB->compile(ctx);

            ctx.instructions[jmpEnd].value = ctx.currentPos();
        } else {
            ctx.instructions[jzPos].value = ctx.currentPos();
        }
    }
};

class WhileNode : public ASTNode {
    ASTNode *cond, *body;
public:
    WhileNode(ASTNode* c, ASTNode* b)
        : cond(c), body(b) {}

    ~WhileNode() {
        delete cond;
        delete body;
    }

    double eval() override {
        double v = 0;
        while (cond->eval())
            v = body->eval();
        return v;
    }

    void compile(IRContext& ctx) override {
        int loopStart = ctx.currentPos();
        cond->compile(ctx);

        int exitPos = ctx.currentPos();
        ctx.emit("JZ", 0, lineNum);

        body->compile(ctx);
        ctx.emit("JMP", loopStart, lineNum);

        ctx.instructions[exitPos].value = ctx.currentPos();
    }
};

/* =========================
   Statement List
   ========================= */

class StatementListNode : public ASTNode {
public:
    std::vector<ASTNode*> stmts;

    ~StatementListNode() {
        for (auto s : stmts) delete s;
    }

    void add(ASTNode* s) {
        stmts.push_back(s);
    }

    double eval() override {
        double v = 0;
        for (auto s : stmts)
            v = s->eval();
        return v;
    }

    void compile(IRContext& ctx) override {
        for (auto s : stmts)
            s->compile(ctx);
    }
};

#endif
