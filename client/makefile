# Makefile for lab2
CC = gcc
LEX = flex
YACC = bison
CFLAGS = -g -DYYDEBUG=1
LIBS = -lxml2

PROGRAM = lab3

.PHONY: all clean debug
default: all

all: clean ${PROGRAM}

${PROGRAM}: ast.h xml.h parser.tab.c lexer.c 
	${CC} -o ${PROGRAM} main.c ast.c xml.c parser.tab.c lexer.c ${LIBS}

parser.tab.c parser.tab.h: parser.y
	${YACC} -d -Wcounterexamples parser.y

lexer.c: lexer.l
	${LEX} -o lexer.c lexer.l

clean:
	rm -f ${PROGRAM} parser.tab.c parser.tab.h lexer.c

debug: ast.h parser.tab.c lexer.c
	${CC} -o ${PROGRAM} ${CFLAGS} ast.c parser.tab.c lexer.c



