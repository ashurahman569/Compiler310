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
    .|\n {
        cout << "Line No. " << startLine << ": Error in <PAIR_TYPE> declaration" << endl;
        yyterminate();
    }
}

<EXPECT_FIRST_TYPE>{
    "int" | "float" | "double" | "char" {
        strbuf += string(yytext);
        BEGIN(EXPECT_COMMA);
    }
    "pair" {
        strbuf += "pair";
        /* If this nested pair finishes, we will be expecting a comma for the outer pair */
        push_state(EXPECT_COMMA); 
        BEGIN(EXPECT_LT);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    .|\n {
        /* Catches empty pairs like pair<>, or invalid chars */
        cout << "Line No. " << startLine << ": Error in <PAIR_TYPE> declaration" << endl;
        yyterminate();
    }
}

<EXPECT_COMMA>{
    "," {
        strbuf += ",";
        BEGIN(EXPECT_SECOND_TYPE);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    .|\n {
        /* Catches pair<int> (missing comma and second type) */
        cout << "Line No. " << startLine << ": Error in <PAIR_TYPE> declaration" << endl;
        yyterminate();
    }
}

<EXPECT_SECOND_TYPE>{
    "int" | "float" | "double" | "char" {
        strbuf += string(yytext);
        BEGIN(EXPECT_GT);
    }
    "pair" {
        strbuf += "pair";
        /* If this nested pair finishes, we will be expecting '>' for the outer pair */
        push_state(EXPECT_GT); 
        BEGIN(EXPECT_LT);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    .|\n {
        /* Catches pair<int, > (missing second type) */
        cout << "Line No. " << startLine << ": Error in <PAIR_TYPE> declaration" << endl;
        yyterminate();
    }
}

<EXPECT_GT>{
    ">" {
        strbuf += ">";
        int return_state = pop_state();
        if (return_state == INITIAL) {
            /* Successfully closed the outermost pair */
            cout << "Line No. " << startLine << ": Token<PAIR_TYPE> Lexeme " << strbuf << " found" << endl;
            BEGIN(INITIAL);
        } else {
            /* Closed a nested pair, return to the outer pair's expected state */
            BEGIN(return_state);
        }
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    .|\n {
        /* Catches pair<int, float, double> (too many types) */
        cout << "Line No. " << startLine << ": Error in <PAIR_TYPE> declaration" << endl;
        yyterminate();
    }
}

<<EOF>> {
    if (YY_START != INITIAL) {
        cout << "Line No. " << startLine << ": Error in <PAIR_TYPE> declaration" << endl;
    }
    yyterminate();
}

%%

int main(int argc, char** argv) {
    if (argc != 2) { printf("Usage: ./a.out <input_file>\n"); return 1; }
    FILE* in = fopen(argv[1], "r");
    if (!in) { printf("Cannot open file\n"); return 1; }
    
    yyin = in;
    logFile.open("2205029_log.txt");
    streambuf* old = cout.rdbuf(logFile.rdbuf());
    
    yylex();
    
    cout.rdbuf(old);
    logFile.close();
    fclose(in);
    return 0;
}
