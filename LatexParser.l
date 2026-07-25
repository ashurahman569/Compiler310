/*
 * 2205029.l
 * CSE 310 Online B2: Simplified LaTeX Syntax Parser
 * 
 * Objective: Validate simplified LaTeX commands (\textbf, \textit, \section) 
 *            and ensure their arguments are properly enclosed in matched {}.
 * 
 * Approach: 
 *   - Use states to track if we are expecting a brace block after a command.
 *   - Use a stack to track nested {} blocks.
 *   - Ignore text content inside and outside braces.
 */

%option noyywrap

%{
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

/* --- Stack Implementation for tracking nested braces --- */
#define MAX_STACK 1000
char *stack[MAX_STACK];
int top = -1;
 
void push(const char *tag) {
    if (top >= MAX_STACK - 1) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
    stack[++top] = strdup(tag);
}
 
char* pop() {
    if (top < 0) return NULL;
    return stack[top--];
}

ofstream logFile;
%}

/* Start conditions */
%x EXPECT_BRACE  /* Just saw a valid command, expecting '{' */
%x IN_BRACE      /* Inside a '{...}' block */

%%

/* ============================================================
 * INITIAL STATE: Outside any braces, looking for commands or text
 * ============================================================ */
<INITIAL>{
    "\\textbf" | "\\textit" | "\\section" {
        /* Valid command found. Next character MUST be '{' */
        BEGIN(EXPECT_BRACE);
    }
    "\\"[a-zA-Z]+ {
        /* Backslash followed by letters, but not a supported command */
        cout << "Error: unsupported command" << endl;
        exit(0);
    }
    "{" {
        /* Standalone '{' (not following a command). Push to stack and enter brace block. */
        push("{");
        BEGIN(IN_BRACE);
    }
    "}" {
        /* Closing brace without any opening brace in INITIAL state */
        cout << "Error: unmatched }" << endl;
        exit(0);
    }
    [^\\\{\}\n]+ {
        /* Ignore any text that is not a command or brace */
    }
    \n {
        /* Ignore newlines */
    }
    "\\" {
        /* Ignore lone backslash not followed by letters */
    }
}

/* ============================================================
 * EXPECT_BRACE STATE: Immediately after a valid command
 * ============================================================ */
<EXPECT_BRACE>{
    "{" {
        /* Correctly followed by '{'. Push to stack and enter brace block. */
        push("{");
        BEGIN(IN_BRACE);
    }
    .|\n {
        /* Any other character (including space) means the command is not followed by '{' */
        cout << "Error: command not followed by brace block" << endl;
        exit(0);
    }
}

/* ============================================================
 * IN_BRACE STATE: Inside a '{...}' block
 * ============================================================ */
<IN_BRACE>{
    "{" {
        /* Nested opening brace. Push to stack. */
        push("{");
    }
    "}" {
        /* Closing brace. Pop from stack. */
        char* t = pop();
        if (t == NULL) {
            /* Should not happen due to state logic, but safe guard */
            cout << "Error: unmatched }" << endl;
            exit(0);
        }
        free(t);
        
        /* If stack is empty, we've closed the outermost brace. Return to INITIAL. */
        if (top == -1) {
            BEGIN(INITIAL);
        }
    }
    "\\textbf" | "\\textit" | "\\section" {
        /* Valid command inside braces. Expect '{' next. */
        BEGIN(EXPECT_BRACE);
    }
    "\\"[a-zA-Z]+ {
        /* Unsupported command inside braces */
        cout << "Error: unsupported command" << endl;
        exit(0);
    }
    [^\\\{\}\n]+ {
        /* Ignore text content inside braces */
    }
    \n {
        /* Ignore newlines inside braces */
    }
    "\\" {
        /* Ignore lone backslash inside braces */
    }
}

/* ============================================================
 * EOF HANDLING: Check for incomplete structures at end of file
 * ============================================================ */
<INITIAL><<EOF>> {
    if (top == -1) {
        cout << "Valid LaTeX syntax" << endl;
    }
    yyterminate();
}

<EXPECT_BRACE><<EOF>> {
    cout << "Error: command not followed by brace block" << endl;
    yyterminate();
}

<IN_BRACE><<EOF>> {
    cout << "Error: unmatched {" << endl;
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

    string studentId = "2205029";
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
