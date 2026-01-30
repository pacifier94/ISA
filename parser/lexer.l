%{
#include <string>
#include "parser.tab.h"
int line_num = 1;
%}

%option noyywrap

%%
[ \t\r]+        ;
\n              { line_num++; }

"//".*          ;
"/*"            {
                    int c;
                    while ((c = yyinput()) != 0) {
                        if (c == '\n') line_num++;
                        else if (c == '*') {
                            int n = yyinput();
                            if (n == '/') break;
                            else unput(n);
                        }
                    }
                }

"var"           return VAR;
"if"            return IF;
"else"          return ELSE;
"while"         return WHILE;

"=="            return EQ;
"!="            return NE;
"<="            return LE;
">="            return GE;

[0-9]+"."[0-9]+ { yylval.fnum = atof(yytext); return FLOAT; }
[0-9]+          { yylval.fnum = atof(yytext); return FLOAT; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = new std::string(yytext);
    return IDENTIFIER;
}

[-+*/=<>(){};,] return yytext[0];

. {
    fprintf(stderr, "Lexical error at line %d: %s\n", line_num, yytext);
    exit(1);
}
%%
