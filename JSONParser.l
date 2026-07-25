/*
 * 2105ABC.l
 * CSE 310 Online A2: JSON Validator
 * 
 * Objective: Validate a simplified JSON-like input format containing nested objects,
 *            quoted string keys/values, colons, and commas.
 * 
 * Approach: 
 *   - Use states to track the expected structure (EXPECT_KEY, EXPECT_COLON, etc.).
 *   - Use a stack to track nested {} blocks.
 *   - Ignore whitespace. Report specific errors for malformed structures.
 */

%option noyywrap

%{
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

/* --- Stack Implementation for tracking nested objects --- */
#define MAX_STACK 100
int state_stack[MAX_STACK];
int top = -1;

void push_state(int state) {
    if (top >= MAX_STACK - 1) {
        cout << "Error: stack overflow" << endl;
        exit(0);
    }
    state_stack[++top] = state;
}

int pop_state() {
    if (top < 0) return -1;
    return state_stack[top--];
}

/* Flag to track if the current string being parsed is a key or a value */
bool is_key = false;

/* Output file stream for the log */
ofstream logFile;
%}

/* Start conditions */
%x EXPECT_KEY
%x IN_STRING
%x EXPECT_COLON
%x EXPECT_VALUE
%x EXPECT_COMMA_OR_CLOSE
%x DONE

%%

/* ============================================================
 * INITIAL STATE: Expecting the very first '{'
 * ============================================================ */
<INITIAL>{
    "{" {
        /* Start of the root object. Push the expected next state and enter EXPECT_KEY. */
        push_state(EXPECT_COMMA_OR_CLOSE);
        BEGIN(EXPECT_KEY);
    }
    [ \t\r\n]+ { /* Ignore leading whitespace */ }
    . {
        cout << "Error: must start with {" << endl;
        exit(0);
    }
}

/* ============================================================
 * EXPECT_KEY STATE: Inside an object, expecting a key or '}'
 * ============================================================ */
<EXPECT_KEY>{
    "\"" {
        /* Found the start of a key string */
        is_key = true;
        BEGIN(IN_STRING);
    }
    "}" {
        /* End of the current object */
        int prev_state = pop_state();
        if (prev_state == -1) {
            cout << "Error: unmatched }" << endl;
            exit(0);
        }
        /* If stack is now empty, we closed the root object */
        if (top == -1) {
            BEGIN(DONE);
        } else {
            /* Return to the state expected by the outer object */
            BEGIN(prev_state);
        }
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    . {
        /* Any other character means the key is not a quoted string */
        cout << "Error: must be a quoted string" << endl;
        exit(0);
    }
}

/* ============================================================
 * IN_STRING STATE: Inside a quoted string "..."
 * ============================================================ */
<IN_STRING>{
    "\"" {
        /* End of string. Transition based on whether it was a key or value. */
        if (is_key) {
            BEGIN(EXPECT_COLON);
        } else {
            BEGIN(EXPECT_COMMA_OR_CLOSE);
        }
    }
    \\\" { /* Escaped quote, ignore and stay in string */ }
    \\\\ { /* Escaped backslash, ignore and stay in string */ }
    \n {
        cout << "Error: unclosed string" << endl;
        exit(0);
    }
    [^\"\\\n]+ { /* Normal string content */ }
    . { /* Any other character */ }
}

/* ============================================================
 * EXPECT_COLON STATE: After a key, expecting ':'
 * ============================================================ */
<EXPECT_COLON>{
    ":" {
        BEGIN(EXPECT_VALUE);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    . {
        cout << "Error: Missing separator" << endl;
        exit(0);
    }
}

/* ============================================================
 * EXPECT_VALUE STATE: After ':', expecting a string or nested object
 * ============================================================ */
<EXPECT_VALUE>{
    "\"" {
        /* Start of a value string */
        is_key = false;
        BEGIN(IN_STRING);
    }
    "{" {
        /* Start of a nested object */
        push_state(EXPECT_COMMA_OR_CLOSE);
        BEGIN(EXPECT_KEY);
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    [^ \t\r\n\"{]+ {
        /* Encountered unquoted value (like a number 29) */
        cout << "Error: must be a quoted string" << endl;
        exit(0);
    }
}

/* ============================================================
 * EXPECT_COMMA_OR_CLOSE STATE: After a value, expecting ',' or '}'
 * ============================================================ */
<EXPECT_COMMA_OR_CLOSE>{
    "," {
        /* Another key-value pair is coming */
        BEGIN(EXPECT_KEY);
    }
    "}" {
        /* End of the current object */
        int prev_state = pop_state();
        if (prev_state == -1) {
            cout << "Error: unmatched }" << endl;
            exit(0);
        }
        if (top == -1) {
            BEGIN(DONE);
        } else {
            BEGIN(prev_state);
        }
    }
    [ \t\r\n]+ { /* Ignore whitespace */ }
    . {
        cout << "Error: expected , or }" << endl;
        exit(0);
    }
}

/* ============================================================
 * DONE STATE: Root object is closed, expecting EOF
 * ============================================================ */
<DONE>{
    [ \t\r\n]+ { /* Ignore trailing whitespace */ }
    . {
        cout << "Error: unexpected characters after valid JSON" << endl;
        exit(0);
    }
}

/* ============================================================
 * EOF HANDLING: Check if we successfully reached the DONE state
 * ============================================================ */
<<EOF>> {
    if (YY_START == DONE) {
        cout << "Valid JSON Structure" << endl;
    } else {
        /* Reached EOF while still inside an object or string */
        cout << "Error: unmatched {" << endl;
    }
    yyterminate();
}

%%

/* ============================================================
 * MAIN: File I/O and setup
 * ============================================================ */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Please provide input file name and try again\n");
        return 1;
    }

    FILE* in = fopen(argv[1], "r");
    if (!in) {
        printf("Cannot open specified file\n");
        return 1;
    }

    yyin = in;

    string studentId = "2105ABC";
    logFile.open(studentId + "_log.txt");
    if (!logFile.is_open()) {
        printf("Cannot create log file\n");
        return 1;
    }

    /* Redirect cout to the log file */
    streambuf* oldCout = cout.rdbuf();
    cout.rdbuf(logFile.rdbuf());

    yylex();

    /* Restore cout and cleanup */
    cout.rdbuf(oldCout);
    logFile.close();
    fclose(in);

    return 0;
}
