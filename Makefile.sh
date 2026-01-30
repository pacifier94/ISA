CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -DGC_TEST
INCLUDES = -Iparser -Iir -Ivm -Iruntime

all: ipshell

parser/parser.tab.c parser/parser.tab.h: parser/parser.y
	bison -d -o parser/parser.tab.c parser/parser.y

parser/lex.yy.c: parser/lexer.l parser/parser.tab.h
	flex -o parser/lex.yy.c parser/lexer.l

ipshell: shell/shell.cpp vm/vm.cpp parser/parser.tab.c parser/lex.yy.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) \
	-o ipshell \
	shell/shell.cpp \
	vm/vm.cpp \
	parser/parser.tab.c \
	parser/lex.yy.c

clean:
	rm -f ipshell \
	parser/parser.tab.c \
	parser/parser.tab.h \
	parser/lex.yy.c

.PHONY: all clean
