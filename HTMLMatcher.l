/*
 * 2105ABC.l
 * CSE 310 Online C1: HTML Tag Matching
 * 
 * Objective: Validate proper nesting of HTML-style tags (<html>, <head>, <body>, <p>).
 *            Ignore text content between tags. Report mismatches or missing closing tags.
 * 
 * Approach: Use a stack to track opening tags. 
 *           - On valid opening tag -> push to stack.
 *           - On valid closing tag -> pop from stack and verify it matches.
 *           - On invalid tag -> report error and stop parsing.
 *           - At EOF -> if stack is empty, structure is valid; else, report missing closing tag.
 */

%option noyywrap

%{
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

/* --- Stack Implementation (Provided in prompt) --- */
/* Renamed 'stack' to 'tag_stack' to avoid potential conflicts with C++ std::stack */
#define MAX_STACK 1000
char *tag_stack[MAX_STACK];
int top = -1;
 
void push(const char *tag) {
    if (top >= MAX_STACK - 1) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
    tag_stack[++top] = strdup(tag);
}
 
char* pop() {
    if (top < 0) return NULL;
    return tag_stack[top--];
}

/* Output file stream for the log */
ofstream logFile;
%}

%%

/* ============================================================
 * VALID OPENING TAGS: Push the tag name onto the stack
 * ============================================================ */

"<html>"    { push("html"); }
"<head>"    { push("head"); }
"<body>"    { push("body"); }
"<p>"       { push("p"); }

/* ============================================================
 * VALID CLOSING TAGS: Pop from stack and verify match
 * If mismatch or stack is empty, report error and STOP PARSING.
 * ============================================================ */

"</html>"   {
                char* t = pop();
                if (t == NULL || strcmp(t, "html") != 0) {
                    cout << "Invalid HTML: tag mismatch</html>" << endl;
                    exit(0); /* Stop parsing on first mismatch */
                }
                free(t); /* Free memory allocated by strdup in push() */
            }

"</head>"   {
                char* t = pop();
                if (t == NULL || strcmp(t, "head") != 0) {
                    cout << "Invalid HTML: tag mismatch</head>" << endl;
                    exit(0);
                }
                free(t);
            }

"</body>"   {
                char* t = pop();
                if (t == NULL || strcmp(t, "body") != 0) {
                    cout << "Invalid HTML: tag mismatch</body>" << endl;
                    exit(0);
                }
                free(t);
            }

"</p>"      {
                char* t = pop();
                if (t == NULL || strcmp(t, "p") != 0) {
                    cout << "Invalid HTML: tag mismatch</p>" << endl;
                    exit(0);
                }
                free(t);
            }

/* ============================================================
 * TEXT CONTENT: Ignore anything that is not a tag
 * ============================================================ */

[^<]+       { 
                /* Any sequence of characters without '<' is treated as text content.
                   We simply ignore it by providing an empty action. */
            }

/* ============================================================
 * INVALID TAGS: Any other tag format is invalid input
 * ============================================================ */

<[^>]*>     { 
                /* Matches things like <div>, <span>, or malformed tags like <html 
                   that have a closing '>' but aren't in our valid list. */
                cout << "Invalid input" << endl; 
                exit(0); 
            }

"<"         {
                /* Handles edge case where '<' appears at the end of file without '>' */
                cout << "Invalid input" << endl;
                exit(0);
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
    /* Using 2105ABC as per the assignment instructions. Change to your actual ID if needed. */
    string studentId = "2105ABC"; 
    logFile.open(studentId + "_log.txt");
    if (!logFile.is_open()) {
        printf("Cannot create log file\n");
        return 1;
    }

    /* Redirect cout to the log file so all output goes there */
    streambuf* oldCout = cout.rdbuf();
    cout.rdbuf(logFile.rdbuf());

    /* --- Step 5: Run the lexical analyzer --- */
    yylex();

    /* --- Step 6: After scanning, determine final result --- */
    /* If we reach here, no mismatch occurred during parsing. 
       We just need to check if all opening tags were closed. */
    if (top == -1) {
        cout << "Valid HTML structure" << endl;
    } else {
        /* Stack is not empty -> report the topmost unclosed tag */
        cout << "Invalid HTML: missing closing tag for<" << tag_stack[top] << ">" << endl;
    }

    /* --- Step 7: Cleanup --- */
    cout.rdbuf(oldCout);   /* Restore original cout */
    logFile.close();       /* Close log file */
    fclose(in);            /* Close input file */

    return 0;
}
