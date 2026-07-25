/*
 * Objective: Detect whether a stream of brackets (), {}, [] is properly
 *            matched/nested. Report "Brackets are matched." or "Unmatched brackets."
 * Approach: Use a stack (implemented as a string) to track opening brackets.
 *           - On '(', '{', '[' → push onto stack.
 *           - On ')', '}', ']' → check if top of stack is the matching opener.
 *             If yes, pop. If no (or stack empty), flag as mismatched.
 *           - At end of input, if stack is non-empty → unmatched.
 */

%option noyywrap
%option yylineno

%{
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/* 
 * bracketStack: acts as a LIFO stack storing opening brackets.
 * We use a string where the last character is the "top" of the stack.
 */
string bracketStack = "";

/* 
 * mismatched: set to true the moment we detect any mismatch
 * (e.g., closing bracket with no opener, or wrong type of opener on top).
 */
bool mismatched = false;

/*
 * isMatchingPair: returns true if 'open' and 'close' form a valid bracket pair.
 */
bool isMatchingPair(char open, char close) {
    return (open == '(' && close == ')') ||
           (open == '{' && close == '}') ||
           (open == '[' && close == ']');
}

/* Output file stream for the log */
ofstream logFile;
%}

%%

/* ============================================================
 * OPENING BRACKETS: Push onto the stack
 * ============================================================ */

"(" {
    /* Push opening parenthesis onto the stack */
    bracketStack += '(';
}

"{" {
    /* Push opening curly brace onto the stack */
    bracketStack += '{';
}

"[" {
    /* Push opening square bracket onto the stack */
    bracketStack += '[';
}

/* ============================================================
 * CLOSING BRACKETS: Validate against top of stack, then pop
 * ============================================================ */

")" {
    /*
     * If stack is empty → no matching opener exists → mismatch.
     * If top of stack is not '(' → wrong bracket type → mismatch.
     * Otherwise, pop the matching opener.
     */
    if (bracketStack.empty() || !isMatchingPair(bracketStack.back(), ')')) {
        mismatched = true;
    } else {
        bracketStack.pop_back();
    }
}

"}" {
    /* Same logic for closing curly brace */
    if (bracketStack.empty() || !isMatchingPair(bracketStack.back(), '}')) {
        mismatched = true;
    } else {
        bracketStack.pop_back();
    }
}

"]" {
    /* Same logic for closing square bracket */
    if (bracketStack.empty() || !isMatchingPair(bracketStack.back(), ']')) {
        mismatched = true;
    } else {
        bracketStack.pop_back();
    }
}

/* ============================================================
 * WHITESPACE: Ignore spaces, tabs, newlines, carriage returns
 * (Input is a "stream of brackets", but we handle whitespace gracefully)
 * ============================================================ */

[ \t\r\n]+ {
    /* Skip all whitespace characters */
}

/* ============================================================
 * FALLBACK: Any other character (shouldn't appear per problem statement,
 * but we handle it gracefully by ignoring)
 * ============================================================ */

. {
    /* Ignore unexpected characters */
}

%%

/* ============================================================
 * MAIN: File I/O, invoke lexer, check final stack state, write result
 * ============================================================ */

int main(int argc, char *argv[]) {

    /* --- Step 1: Validate command-line arguments --- */
    if (argc != 2) {
        printf("Please provide input file name and try again\n");
        return 1;
    }

    /* --- Step 2: Open the input file --- */
    FILE* in = fopen(argv[1], "r");
    if (!in) {
        printf("Cannot open specified file\n");
        return 1;
    }

    /* --- Step 3: Set up Flex input --- */
    yyin = in;

    /* --- Step 4: Open the log file for output --- */
    string studentId = "2205029";
    logFile.open(studentId + "_log.txt");
    if (!logFile.is_open()) {
        printf("Cannot create log file\n");
        fclose(in);
        return 1;
    }

    /* Redirect cout to the log file so all output goes there */
    streambuf* oldCout = cout.rdbuf();
    cout.rdbuf(logFile.rdbuf());

    /* --- Step 5: Run the lexical analyzer --- */
    yylex();

    /* --- Step 6: After scanning, determine final result --- */
    /*
     * Two conditions mean "unmatched":
     *   1. 'mismatched' flag was set during scanning (wrong closer encountered).
     *   2. Stack is not empty after scanning (leftover openers with no closers).
     */
    if (!mismatched && bracketStack.empty()) {
        cout << "Brackets are matched." << endl;
    } else {
        cout << "Unmatched brackets." << endl;
    }

    /* --- Step 7: Cleanup --- */
    cout.rdbuf(oldCout);   /* Restore original cout */
    logFile.close();       /* Close log file */
    fclose(in);            /* Close input file */

    return 0;
}
