# 🦊 Ultimate Lex / Flex Cheatsheet

A comprehensive guide to Lexical Analysis using Lex/Flex, tailored for compiler design courses (like CSE 310).

---

## 📑 Table of Contents
1. [Structure of a `.l` File](#1-structure-of-a-l-file)
2. [Regular Expressions (The Patterns)](#2-regular-expressions-the-patterns)
3. [Context & Anchors (Advanced Matching)](#3-context--anchors-advanced-matching)
4. [Start Conditions (State Machine)](#4-start-conditions-state-machine)
5. [Built-in Variables & Functions](#5-built-in-variables--functions)
6. [Lex Directives (Options)](#6-lex-directives-options)
7. [The "Golden Rules" of Disambiguation](#7-the-golden-rules-of-lex-disambiguation)
8. [Common "Cheat" Patterns for Exams](#8-common-cheat-patterns-for-exams)
9. [Pro-Tips for Online Exams](#9-pro-tips-for-cse-310-online-exams)

---

## 1. Structure of a `.l` File
A Lex file is strictly divided into three sections separated by `%%`.

```lex
/* ================= 1. DEFINITIONS SECTION ================= */
%{
#include <stdio.h>
// C/C++ code, variable declarations, helper functions
%}

/* Declare start conditions */
%s INCLUSIVE_STATE  /* Inclusive start condition */
%x EXCLUSIVE_STATE  /* Exclusive start condition */

%%
/* ================= 2. RULES SECTION ================= */
/* Regex patterns followed by C/C++ actions */
pattern1   { /* C code */ }
pattern2   { /* C code */ }

%%
/* ================= 3. USER CODE SECTION ================= */
int main(int argc, char **argv) {
    yyin = fopen(argv[1], "r");
    yylex();   // Starts the lexical analysis
    return 0;
}

int yywrap() { 
    return 1; // Called at EOF. Return 1 to stop. 
}
```

---

## 2. Regular Expressions (The Patterns)

### Basic Matching
| Symbol | Meaning | Example | Matches |
| :--- | :--- | :--- | :--- |
| `c` | Literal character | `a` | `a` |
| `"str"` | Literal string (ignores regex operators inside) | `"vector"` | `vector` |
| `.` | Any character **except** newline (`\n`) | `a.c` | `abc`, `a c`, `a5c` |
| `\c` | Escape character (to match literal `*`, `+`, etc.) | `\*` | `*` |
| `\n`, `\t`, `\r` | Newline, Tab, Carriage Return | `\n` | Newline character |

### Character Classes
| Symbol | Meaning | Example | Matches |
| :--- | :--- | :--- | :--- |
| `[abc]` | Any one character in the set | `[aeiou]` | `a`, `e`, `i`, etc. |
| `[a-z]` | Range of characters | `[0-9]` | Any digit |
| `[^abc]` | **Negation**: Any char *not* in set | `[^0-9]` | Any non-digit |
| `[a-zA-Z_]` | Combined ranges | `[a-zA-Z_]+` | Identifiers |

### Quantifiers (Repetition)
| Symbol | Meaning | Example | Matches |
| :--- | :--- | :--- | :--- |
| `*` | 0 or more times | `a*` | ``, `a`, `aa`, `aaa` |
| `+` | 1 or more times | `a+` | `a`, `aa`, `aaa` |
| `?` | 0 or 1 time (optional) | `a?` | ``, `a` |
| `{n}` | Exactly *n* times | `a{3}` | `aaa` |
| `{n,}` | *n* or more times | `a{2,}` | `aa`, `aaa`, `aaaa` |
| `{n,m}` | Between *n* and *m* times | `a{2,4}` | `aa`, `aaa`, `aaaa` |

### Grouping & Alternation
| Symbol | Meaning | Example | Matches |
| :--- | :--- | :--- | :--- |
| `\|` | OR (Alternation) | `cat\|dog` | `cat` or `dog` |
| `()` | Grouping | `(ab)+` | `ab`, `abab`, `ababab` |

---

## 3. Context & Anchors (Advanced Matching)

| Symbol | Meaning | Example | Explanation |
| :--- | :--- | :--- | :--- |
| `^` | Start of line anchor | `^#` | Matches `#` only if it's the first char on a line. |
| `$` | End of line anchor | `;$` | Matches `;` only if it's the last char on a line. |
| `/` | **Trailing Context** | `A/B` | Matches `A` **only if** followed by `B`. `B` is *not* consumed (stays in input buffer). |
| `<<EOF>>` | End of File | `<<EOF>>` | Special pattern triggered when the file ends. |

*Example of Trailing Context:*
```lex
"real"/[0-9]  { printf("Found 'real' before a number\n"); }
```

---

## 4. Start Conditions (State Machine)
Used to handle nested structures (like C comments, strings, or nested brackets).

### Declaration
```lex
%s STATE_NAME   /* Inclusive: Matches rules in INITIAL and STATE_NAME */
%x STATE_NAME   /* Exclusive: Matches ONLY rules explicitly tagged with <STATE_NAME> */
```

### Usage in Rules
```lex
<STATE_NAME>pattern    { /* Action */ }
<STATE1,STATE2>pattern { /* Action for multiple states */ }
<*>\n                  { /* Action for ALL states */ }
```

### State Control Functions
| Function | Action |
| :--- | :--- |
| `BEGIN(STATE_NAME);` | Switches the lexer to `STATE_NAME`. |
| `BEGIN(INITIAL);` | Switches back to the default starting state. |
| `YY_START` | Variable holding the current state (e.g., `if (YY_START == STATE_NAME)`). |

---

## 5. Built-in Variables & Functions

### Variables
| Variable | Type | Description |
| :--- | :--- | :--- |
| `yytext` | `char *` | The actual string that was matched by the regex. |
| `yyleng` | `int` | The length of `yytext`. |
| `yylineno` | `int` | Current line number (Requires `%option yylineno`). |
| `yyin` | `FILE *` | Input file pointer. Set this in `main()` before calling `yylex()`. |
| `yyout` | `FILE *` | Output file pointer (used by `ECHO`). |

### Functions & Macros
| Function/Macro | Description |
| :--- | :--- |
| `yylex()` | The main function that runs the lexer. Returns 0 at EOF. |
| `ECHO` | Macro that prints `yytext` to `yyout`. (Default action if none is provided). |
| `yywrap()` | Called at EOF. Return `1` to stop, `0` to continue to another file. |
| `yyterminate()` | Immediately stops the lexer and returns 0 from `yylex()`. |
| `REJECT` | Tells Lex: "I don't want this match, go to the next best matching rule." *(Warning: makes lexer slower)*. |
| `input()` | Reads and returns the next character from the input stream. |
| `unput(c)` | Pushes character `c` back into the input stream to be read again. |

---

## 6. Lex Directives (Options)
Placed in the definitions section to change Lex's behavior.

```lex
%option noyywrap         /* Tells Lex not to look for yywrap(). Highly recommended. */
%option yylineno         /* Automatically tracks line numbers in yylineno. */
%option case-insensitive /* Makes all regex matching case-insensitive (A == a). */
%option nodefault        /* Disables the default rule (which echoes unmatched chars). */
%option nounput          /* Disables unput() if you don't use it (removes warnings). */
```

---

## 7. The "Golden Rules" of Lex Disambiguation
When multiple rules could match the input, Lex decides using these strict rules:

1. **Longest Match Wins:** Lex always consumes the maximum number of characters possible.
   * *Example:* Input `vector123`. Rules: `vector` and `[a-z]+`. Lex will match `vector` because `[a-z]+` stops at `1`. But if input is `vectorabc`, `[a-z]+` wins because it matches 9 characters, while `vector` only matches 6.
2. **First Rule Wins (Tie-Breaker):** If two rules match the **exact same length**, the one that appears **first** in the `.l` file wins.
   * *Example:* Rules `int` and `[a-z]+`. Input `int`. Both match 3 characters. `int` wins because it's higher in the file.

---

## 8. Common "Cheat" Patterns for Exams

### 1. C-Style Block Comments `/* ... */`
*Must use start conditions because `.` doesn't match `\n` and we need to ignore everything inside.*
```lex
%x COMMENT
"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>.|\n   { /* Ignore everything inside */ }
```

### 2. Strings with Escape Characters `"..."`
*Handles `\"` inside the string without prematurely ending it.*
```lex
\"([^"\\]|\\.)*\"   { printf("Found string: %s\n", yytext); }
```
*(Breakdown: Starts with `"`. Then matches any non-quote/non-backslash `[^"\\]` OR a backslash followed by any char `\\.`. Repeats 0+ times. Ends with `"`).*

### 3. C-Style Identifiers & Keywords
```lex
"if"|"else"|"while" { printf("Keyword: %s\n", yytext); }
[a-zA-Z_][a-zA-Z0-9_]* { printf("Identifier: %s\n", yytext); }
```
*(Put keywords BEFORE the identifier rule so the "First Rule Wins" tie-breaker applies).*

### 4. Numbers (Integers and Floats)
```lex
[0-9]+                  { printf("Integer: %s\n", yytext); }
[0-9]+\.[0-9]+          { printf("Float: %s\n", yytext); }
```

### 5. Catch-All / Error Handling
*If using `%option nodefault`, you MUST have this, or Lex will crash/warn on unknown chars.*
```lex
.   { printf("Error: Unknown character '%s'\n", yytext); }
```

---

## 9. Pro-Tips for CSE 310 Online Exams

1. **Whitespace is your enemy:** If the problem says "ignore whitespace", add `[ \t\r\n]+ { /* ignore */ }` at the very top of your rules.
2. **EOF Handling:** Always include an `<<EOF>>` rule if the problem requires checking if a structure (like brackets or HTML tags) was closed properly before the file ended.
   ```lex
   <<EOF>> {
       if (top != -1) printf("Error: Unmatched opening bracket\n");
       yyterminate();
   }
   ```
3. **String vs Regex:** If you need to match a literal string that contains regex operators (like `vector<`), wrap it in quotes: `"vector<"` instead of `vector\<`. It's cleaner and less prone to escaping errors.
4. **State Reset:** When you finish a nested structure (like closing a bracket or ending a comment), **always** remember to `BEGIN(INITIAL);` or reset your counters/stacks. Forgetting to reset state is the #1 cause of bugs in these exams.
5. **Memory Leaks:** If you use `strdup()` to push strings onto a stack, remember to `free()` them when you pop them, especially in HTML/LaTeX tag matching problems.
