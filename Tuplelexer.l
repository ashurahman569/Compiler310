%option noyywrap
%option yylineno

/* Start conditions for parsing the tuple structure */
%x EXPECT_LT
%x IN_BRACKETS
%x EXPECT_COMMA_OR_GT

%{
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

string strbuf;      // Accumulates the clean lexeme
int startLine = 0;  // Line where the tuple started
int depth = 0;      // Tracks nested '<' and '>'
ofstream logFile;
%}

%%

<INITIAL>{
    "tuple" {
        strbuf = "tuple";
        startLine = yylineno;
        depth = 0;
        BEGIN(EXPECT_LT);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    [^ \t\r\n]+ {
        /* Any non-whitespace sequence that isn't "tuple" is invalid */
        cout << "Line No. " << yylineno << ": Invalid input" << endl;
    }
}

<EXPECT_LT>{
    "<" {
        strbuf += "<";
        depth++;
        BEGIN(IN_BRACKETS);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    .|\n {
        cout << "Line No. " << startLine << ": Error in <TUPLE_TYPE> declaration" << endl;
        yyterminate();
    }
}

<IN_BRACKETS>{
    "int" | "float" | "double" | "char" {
        strbuf += string(yytext);
        BEGIN(EXPECT_COMMA_OR_GT);
    }
    "tuple" {
        strbuf += "tuple";
        BEGIN(EXPECT_LT); /* Nested tuple, must be followed by '<' */
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    .|\n {
        /* Catches empty tuples like tuple<>, trailing commas, or invalid chars */
        cout << "Line No. " << startLine << ": Error in <TUPLE_TYPE> declaration" << endl;
        yyterminate();
    }
}

<EXPECT_COMMA_OR_GT>{
    "," {
        strbuf += ",";
        BEGIN(IN_BRACKETS); /* Expect another type */
    }
    ">" {
        strbuf += ">";
        depth--;
        if (depth == 0) {
            cout << "Line No. " << startLine << ": Token<TUPLE_TYPE> Lexeme " << strbuf << " found" << endl;
            BEGIN(INITIAL);
        } else {
            /* Closed a nested tuple, now expect ',' or '>' for the outer tuple */
            BEGIN(EXPECT_COMMA_OR_GT);
        }
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    .|\n {
        cout << "Line No. " << startLine << ": Error in <TUPLE_TYPE> declaration" << endl;
        yyterminate();
    }
}

<<EOF>> {
    if (YY_START != INITIAL) {
        cout << "Line No. " << startLine << ": Error in <TUPLE_TYPE> declaration" << endl;
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
