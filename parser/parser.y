%{
#include <cstdio>
#include <cstdlib>
#include <map>
#include <iostream>
#include <string>
#include "ast.h"

extern int yylex();
extern int line_num;
void yyerror(const char*);

ASTNode* root = nullptr;
std::map<std::string, double> symbolTable;
%}

%union {
    double fnum;
    std::string* str;
    ASTNode* node;
    StatementListNode* list;
}

%token <fnum> FLOAT
%token <str> IDENTIFIER
%token VAR IF ELSE WHILE EQ NE LE GE

%type <node> program stmt expr equality comp term factor unary primary var_decl
%type <list> stmt_list decl_list

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

program
    : stmt_list { root = $1; }
    ;

stmt_list
    : stmt_list stmt { $1->add($2); $$ = $1; }
    | /* empty */    { $$ = new StatementListNode(); }
    ;

stmt
    : var_decl
    | expr ';'                     /* assignment is now an expression */
    | IF '(' expr ')' stmt %prec LOWER_THAN_ELSE
        { $$ = new IfNode($3, $5, nullptr); }
    | IF '(' expr ')' stmt ELSE stmt
        { $$ = new IfNode($3, $5, $7); }
    | WHILE '(' expr ')' stmt
        { $$ = new WhileNode($3, $5); }
    | '{' stmt_list '}' { $$ = $2; }
    ;

var_decl
    : VAR decl_list ';'
        { $$ = static_cast<ASTNode*>($2); }
    ;

decl_list
    : decl_list ',' IDENTIFIER
        {
            $$ = $1;
            $$->add(
                new BinaryOpNode(
                    "DECLARE",
                    new IdentifierNode(*$3),
                    nullptr
                )
            );
            delete $3;
        }
    | decl_list ',' IDENTIFIER '=' expr
        {
            $$ = $1;
            $$->add(
                new BinaryOpNode(
                    "DECLARE_INIT",
                    new IdentifierNode(*$3),
                    $5
                )
            );
            delete $3;
        }
    | IDENTIFIER
        {
            $$ = new StatementListNode();
            $$->add(
                new BinaryOpNode(
                    "DECLARE",
                    new IdentifierNode(*$1),
                    nullptr
                )
            );
            delete $1;
        }
    | IDENTIFIER '=' expr
        {
            $$ = new StatementListNode();
            $$->add(
                new BinaryOpNode(
                    "DECLARE_INIT",
                    new IdentifierNode(*$1),
                    $3
                )
            );
            delete $1;
        }
    ;


/* Right-associative assignment */
expr
    : IDENTIFIER '=' expr
        {
            $$ = new BinaryOpNode(
                "=",
                new IdentifierNode(*$1),
                $3
            );
            delete $1;
        }
    | equality { $$ = $1; }
    ;

equality
    : equality EQ comp { $$ = new BinaryOpNode("==", $1, $3); }
    | equality NE comp { $$ = new BinaryOpNode("!=", $1, $3); }
    | comp             { $$ = $1; }
    ;

comp
    : comp '<' term { $$ = new BinaryOpNode("<",  $1, $3); }
    | comp '>' term { $$ = new BinaryOpNode(">",  $1, $3); }
    | comp LE term  { $$ = new BinaryOpNode("<=", $1, $3); }
    | comp GE term  { $$ = new BinaryOpNode(">=", $1, $3); }
    | term          { $$ = $1; }
    ;

term
    : term '+' factor { $$ = new BinaryOpNode("+", $1, $3); }
    | term '-' factor { $$ = new BinaryOpNode("-", $1, $3); }
    | factor          { $$ = $1; }
    ;

factor
    : factor '*' unary { $$ = new BinaryOpNode("*", $1, $3); }
    | factor '/' unary { $$ = new BinaryOpNode("/", $1, $3); }
    | unary            { $$ = $1; }
    ;

unary
    : '-' unary { $$ = new BinaryOpNode("-", new NumberNode(0), $2); }
    | '+' unary { $$ = $2; }
    | primary   { $$ = $1; }
    ;

primary
    : FLOAT
        { $$ = new NumberNode($1); }
    | IDENTIFIER
        {
            $$ = new IdentifierNode(*$1);
            delete $1;
        }
    | '(' expr ')'
        { $$ = $2; }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Error at line " << line_num << ": " << s << "\n";
}
/*
int main() {
    if (yyparse() == 0 && root) {
        try {
            root->eval();
            std::cout << "\nFinal Symbol Table\n";
            for (auto &p : symbolTable)
                std::cout << p.first << " = " << p.second << "\n";
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
        delete root;
    }
    return 0;
}
*/
