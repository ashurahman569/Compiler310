/*
 * 2205029.l
 * CSE 310 Online: Pyramid Pattern Validator
 * 
 * Objective: Validate a pyramid pattern of stars (*) and plus signs (+).
 *            - Line 1: n stars, 1 plus, n stars
 *            - Line k: n-k+1 stars, 2k-1 pluses, n-k+1 stars
 *            - Final line: 0 stars, 2n+1 pluses
 * 
 * Approach: Use start conditions to track the parsing state (left stars, pluses, right stars).
 *           Validate counts at the end of each line. Report specific errors as defined.
 */

%option noyywrap
%option yylineno

/* Start conditions for parsing different parts of a line */
%x LEFT_STARS
%x PLUSES
%x RIGHT_STARS

%{
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

int line_num = 1;
int expected_n = -1;      // Number of stars on each side for line 1. -1 means not set yet.
int stars_left = 0;
int stars_right = 0;
int pluses = 0;
bool error_reported = false;
bool pyramid_complete = false;

/* 
 * validate_line: Checks if the current line matches the expected pyramid structure.
 * Called at the end of each line (\n) or at EOF.
 */
void validate_line() {
    if (error_reported) return;

    // If this is the first line, determine 'n' from the left stars
    if (expected_n == -1) {
        expected_n = stars_left;
    }

    int expected_stars = expected_n - (line_num - 1);
    int expected_pluses = 2 * line_num - 1;

    if (expected_stars == 0) {
        // --- LAST LINE ---
        if (stars_right > 0) {
            cout << "Error at line " << line_num << ": Last line must have no stars" << endl;
            error_reported = true;
            return;
        }
        if (pluses != expected_pluses) {
            cout << "Error at line " << line_num << ": Number of pluses incorrect" << endl;
            error_reported = true;
            return;
        }
        pyramid_complete = true;
    } else {
        // --- NOT LAST LINE ---
        if (stars_left != expected_stars) {
            cout << "Error at line " << line_num << ": Invalid character or format" << endl;
            error_reported = true;
            return;
        }
        if (stars_left != stars_right) {
            cout << "Error at line " << line_num << ": Stars on left and right not equal" << endl;
            error_reported = true;
            return;
        }
        if (pluses != expected_pluses) {
            cout << "Error at line " << line_num << ": Number of pluses incorrect" << endl;
            error_reported = true;
            return;
        }
    }
}
%}

%%

/* ============================================================
 * INITIAL STATE: Start of a new line
 * ============================================================ */
<INITIAL>{
    "*"     { stars_left = 1; BEGIN(LEFT_STARS); }
    "+"     {
                // Edge case: n=0, so the first line is just "+"
                if (expected_n == -1) {
                    expected_n = 0;
                    pluses = 1;
                    BEGIN(PLUSES);
                } else {
                    cout << "Error at line " << line_num << ": Invalid character or format" << endl;
                    error_reported = true;
                }
            }
    "\n"    {
                // Empty line
                if (expected_n == -1) {
                    cout << "Error at line " << line_num << ": Invalid character or format" << endl;
                    error_reported = true;
                }
                line_num++;
            }
    .       {
                cout << "Error at line " << line_num << ": Invalid character or format" << endl;
                error_reported = true;
            }
}

/* ============================================================
 * LEFT_STARS STATE: Reading stars on the left side
 * ============================================================ */
<LEFT_STARS>{
    "*"     { stars_left++; }
    "+"     { pluses = 1; BEGIN(PLUSES); }
    "\n"    {
                cout << "Error at line " << line_num << ": Invalid character or format" << endl;
                error_reported = true;
                line_num++;
                BEGIN(INITIAL);
            }
    .       {
                cout << "Error at line " << line_num << ": Invalid character or format" << endl;
                error_reported = true;
            }
}

/* ============================================================
 * PLUSES STATE: Reading pluses in the middle
 * ============================================================ */
<PLUSES>{
    "+"     { pluses++; }
    "*"     {
                int expected_stars = (expected_n == -1) ? stars_left : expected_n - (line_num - 1);
                if (expected_stars == 0) {
                    // Trying to read right stars on the last line
                    cout << "Error at line " << line_num << ": Last line must have no stars" << endl;
                    error_reported = true;
                } else {
                    stars_right = 1;
                    BEGIN(RIGHT_STARS);
                }
            }
    "\n"    {
                validate_line();
                line_num++;
                BEGIN(INITIAL);
                stars_left = stars_right = pluses = 0;
            }
    .       {
                cout << "Error at line " << line_num << ": Invalid character or format" << endl;
                error_reported = true;
            }
}

/* ============================================================
 * RIGHT_STARS STATE: Reading stars on the right side
 * ============================================================ */
<RIGHT_STARS>{
    "*"     { stars_right++; }
    "\n"    {
                validate_line();
                line_num++;
                BEGIN(INITIAL);
                stars_left = stars_right = pluses = 0;
            }
    .       {
                cout << "Error at line " << line_num << ": Invalid character or format" << endl;
                error_reported = true;
            }
}

/* ============================================================
 * EOF HANDLING: Check for incomplete pyramids or confirm success
 * ============================================================ */
<<EOF>> {
    if (YY_START != INITIAL) {
        // File ended in the middle of a line
        if (!error_reported) {
            cout << "Error at line " << line_num << ": Unexpected end of file; incomplete pyramid" << endl;
        }
    } else {
        // File ended after a newline
        if (!error_reported && !pyramid_complete) {
            int err_line = (line_num == 1) ? 1 : line_num - 1;
            cout << "Error at line " << err_line << ": Unexpected end of file; incomplete pyramid" << endl;
        } else if (!error_reported && pyramid_complete) {
            cout << "Pattern matched successfully!" << endl;
        }
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

    string studentId = "2205029";
    ofstream logFile(studentId + "_log.txt");
    if (!logFile.is_open()) {
        printf("Cannot create log file\n");
        return 1;
    }

    // Redirect cout to the log file
    streambuf* oldCout = cout.rdbuf();
    cout.rdbuf(logFile.rdbuf());

    yylex();

    // Restore cout and cleanup
    cout.rdbuf(oldCout);
    logFile.close();
    fclose(in);

    return 0;
}
