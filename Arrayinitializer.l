%option noyywrap
%option yylineno

/* Start conditions for each part of the declaration */
%x EXPECT_NAME
%x EXPECT_LBRACKET
%x IN_SIZE
%x EXPECT_RBRACKET
%x EXPECT_EQUALS
%x EXPECT_LBRACE
%x IN_VALUES
%x EXPECT_COMMA_OR_RBRACE
%x EXPECT_SEMI

%{
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
using namespace std;

int expected_size = 0;
int element_count = 0;
ofstream logFile;
%}

%%

<INITIAL>{
    "int" | "float" | "double" {
        BEGIN(EXPECT_NAME);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    . {
        cout << "Error: unsupported type" << endl;
        yyterminate();
    }
}

<EXPECT_NAME>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    [a-zA-Z_][a-zA-Z0-9_]* {
        BEGIN(EXPECT_LBRACKET);
    }
    .|\n {
        cout << "Error: invalid array name" << endl;
        yyterminate();
    }
}

<EXPECT_LBRACKET>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    "[" {
        BEGIN(IN_SIZE);
    }
    .|\n {
        cout << "Error: expected [" << endl;
        yyterminate();
    }
}

<IN_SIZE>{
    [1-9][0-9]* {
        expected_size = atoi(yytext);
        BEGIN(EXPECT_RBRACKET);
    }
    "0" | [0-9]+ {
        /* Catches '0' or numbers with leading zeros like '05' */
        cout << "Error: invalid size" << endl;
        yyterminate();
    }
    .|\n {
        cout << "Error: expected size" << endl;
        yyterminate();
    }
}

<EXPECT_RBRACKET>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    "]" {
        BEGIN(EXPECT_EQUALS);
    }
    .|\n {
        cout << "Error: expected ]" << endl;
        yyterminate();
    }
}

<EXPECT_EQUALS>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    "=" {
        BEGIN(EXPECT_LBRACE);
    }
    .|\n {
        cout << "Error: expected =" << endl;
        yyterminate();
    }
}

<EXPECT_LBRACE>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    "{" {
        element_count = 0; /* Reset counter for the values */
        BEGIN(IN_VALUES);
    }
    .|\n {
        cout << "Error: expected {" << endl;
        yyterminate();
    }
}

<IN_VALUES>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    [0-9]+(\.[0-9]+)? {
        /* Matches integers and floats */
        element_count++;
        BEGIN(EXPECT_COMMA_OR_RBRACE);
    }
    "}" {
        /* Empty array or end of values */
        if (element_count == expected_size) {
            BEGIN(EXPECT_SEMI);
        } else {
            cout << "Error: expected " << expected_size << " elements, found " << element_count << endl;
            yyterminate();
        }
    }
    .|\n {
        cout << "Error: invalid value" << endl;
        yyterminate();
    }
}

<EXPECT_COMMA_OR_RBRACE>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    "," {
        BEGIN(IN_VALUES); /* Expect next value */
    }
    "}" {
        /* End of values. Check if count matches size */
        if (element_count == expected_size) {
            BEGIN(EXPECT_SEMI);
        } else {
            cout << "Error: expected " << expected_size << " elements, found " << element_count << endl;
            yyterminate();
        }
    }
    .|\n {
        /* Catches trailing commas like "1, 2, }" or invalid chars */
        cout << "Error: expected , or }" << endl;
        yyterminate();
    }
}

<EXPECT_SEMI>{
    [ \t\r\n]+ { /* Ignore whitespace */ }
    ";" {
        cout << "Valid Array Initialization" << endl;
        yyterminate();
    }
    .|\n {
        cout << "Error: expected ;" << endl;
        yyterminate();
    }
}

<<EOF>> {
    cout << "Error: incomplete declaration" << endl;
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
