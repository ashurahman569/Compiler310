%option noyywrap
%option yylineno

/* Start conditions for parsing the strict pair structure */
%x EXPECT_LT
%x EXPECT_FIRST_TYPE
%x EXPECT_COMMA
%x EXPECT_SECOND_TYPE
%x EXPECT_GT

%{
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

string strbuf;      // Accumulates the clean lexeme
int startLine = 0;  // Line where the pair started

/* --- Stack to handle nested pairs --- */
/* Stores the state to return to after a nested pair finishes */
#define MAX_DEPTH 100
int state_stack[MAX_DEPTH];
int top = -1;

void push_state(int s) {
    if (top >= MAX_DEPTH - 1) {
        cout << "Stack overflow" << endl;
        exit(1);
    }
    state_stack[++top] = s;
}

int pop_state() {
    if (top < 0) return -1;
    return state_stack[top--];
}

ofstream logFile;
%}

%%

<INITIAL>{
    "pair" {
        strbuf = "pair";
        startLine = yylineno;
        /* Push INITIAL so that when the outermost '>' is found, we return here */
        push_state(INITIAL); 
        BEGIN(EXPECT_LT);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    [^ \t\r\n]+ {
        /* Any non-whitespace sequence that isn't "pair" is invalid */
        cout << "Line No. " << yylineno << ": Invalid input" << endl;
    }
}

<EXPECT_LT>{
    "<" {
        strbuf += "<";
        BEGIN(EXPECT_FIRST_TYPE);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
