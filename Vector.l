//Tokenizes vector declarations

%option noyywrap
%option yylineno

/* Start conditions */
%x IN_VECTOR   /* Inside 'vector' after keyword */
%x IN_ANGLES   /* Inside angle brackets < ... > */
%x AFTER_TYPE  /* After reading a base type or nested vector, waiting for '>' */

%{
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// Buffer to accumulate the current vector declaration
string strbuf = "";

// Track nesting depth of '<' and '>'
int angleDepth = 0;

// Line number where current vector started
int startLine = 0;

// Flag to indicate if current token is valid so far
bool validToken = true;

// Output log stream (will be redirected in main)
extern ofstream logFile;
%}

%%

/* Rule: match 'vector' (case-sensitive) possibly preceded/followed by whitespace */
"vector"        {
                    // Start of a potential vector type
                    strbuf = "vector";
                    startLine = yylineno;
                    angleDepth = 0;
                    validToken = true;
                    BEGIN(IN_VECTOR);
                }

/* Inside IN_VECTOR: expect '<', ignoring whitespace */
<IN_VECTOR>[ \t\r]*"<"[ \t\r]*    {
                    // Append cleaned version (no extra spaces) for consistent output
                    strbuf += "<";
                    angleDepth++;
                    BEGIN(IN_ANGLES);
                }

/* Inside angle brackets: allow base types */
<IN_ANGLES>[ \t\r]*"int"[ \t\r]*      { strbuf += "int"; BEGIN(AFTER_TYPE); }
<IN_ANGLES>[ \t\r]*"float"[ \t\r]*    { strbuf += "float"; BEGIN(AFTER_TYPE); }
<IN_ANGLES>[ \t\r]*"double"[ \t\r]*   { strbuf += "double"; BEGIN(AFTER_TYPE); }
<IN_ANGLES>[ \t\r]*"char"[ \t\r]*     { strbuf += "char"; BEGIN(AFTER_TYPE); }

/* Or another 'vector' inside */
<IN_ANGLES>[ \t\r]*"vector"[ \t\r]*   {
                    strbuf += "vector";
                    // Now expect '<' next — stay in logic by transitioning properly
                    BEGIN(IN_VECTOR); // Reuse IN_VECTOR to handle '<'
                }

/* After a type (base or nested vector), we may see more '>'s */
<AFTER_TYPE>[ \t\r]*">"[ \t\r]*       {
                    strbuf += ">";
                    angleDepth--;
                    if (angleDepth == 0) {
                        // Complete vector type!
                        cout << "Line No. " << startLine << ": Token<VECTOR_TYPE> Lexeme " << strbuf << " found" << endl;
                        BEGIN(INITIAL);
                    } else {
                        // Still inside outer vector, go back to waiting for next part
                        BEGIN(AFTER_TYPE); // Could also go to IN_ANGLES if more content expected, but for valid input, only '>' follows
                    }
                }

/* Handle consecutive closing '>'s (e.g., >> or >>>) */
<AFTER_TYPE>[ \t\r]*">"[ \t\r]*       {
                    // This rule is same as above; Flex will match longest, but we handle one '>' at a time
                    // Already covered above — no need duplicate
                }

/* If we see unexpected text inside angles, mark as invalid */
<IN_ANGLES>.|\n    {
                    validToken = false;
                    // Skip rest of line or continue? For simplicity, mark error and reset.
                    // But we must consume until end of token — hard in Flex.
                    // Instead, we'll rely on final check.
                }

/* Newline: if we're not in a vector, and buffer is empty, check for invalid input */
\n                {
                    if (strbuf.empty()) {
                        // Line had no vector start — could be blank or invalid
                        // But we only report "Invalid input" if there was non-whitespace
                        // However, Flex skips whitespace, so if we reach \n in INITIAL,
                        // and nothing was matched, it means line was empty or garbage.
                        // We'll handle invalid input via fallback rule below.
                    } else if (angleDepth != 0 || !validToken) {
                        cout << "Line No. " << startLine << ": Error in <VECTOR_TYPE> declaration" << endl;
                        BEGIN(INITIAL);
                        strbuf = "";
                        angleDepth = 0;
                    }
                    // Reset for next line if needed
                }

/* Whitespace outside vectors: ignore */
[ \t\r]+          { /* ignore */ }

/* Fallback: any other character not part of vector syntax → invalid input */
.                 {
                    // Only report if we're not inside a vector (i.e., in INITIAL)
                    if (YY_START == INITIAL) {
                        cout << "Line No. " << yylineno << ": Invalid input" << endl;
                        // Skip rest of line? Not required per spec — assume one token per line
                        // But to avoid multiple errors, consume rest?
                        // For simplicity, just report once per line.
                    } else {
                        // Inside vector but got unexpected char → malformed
                        validToken = false;
                        // We'll report error at newline or end
                    }
                }

%%

// Global log file (to avoid redirecting cout globally)
ofstream logFile;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Please provide input file name and try again" << endl;
        return 1;
    }

    FILE* in = fopen(argv[1], "r");
    if (!in) {
        cerr << "Cannot open specified file" << endl;
        return 1;
    }

    yyin = in;

    string studentId = "2205029";
    logFile.open(studentId + "_log.txt");
    if (!logFile.is_open()) {
        cerr << "Cannot create log file" << endl;
        return 1;
    }

    // Redirect cout to log file
    streambuf* oldCout = cout.rdbuf();
    cout.rdbuf(logFile.rdbuf());

    yylex();

    // Flush and restore
    cout.rdbuf(oldCout);
    logFile.close();
    fclose(in);
    return 0;
}
