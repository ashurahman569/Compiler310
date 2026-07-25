%option noyywrap
%option yylineno

%x COMMENT
%x LCOMMENT
%x STR
%x CHAR

%{
#include "SymbolTable.hpp"
using namespace std;
string unescape(const string& s) {
    // s is the raw text without quotes, may contain backslash escapes
    string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char c = s[i + 1];
            switch (c) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case '\\': out += '\\'; break;
                case '\'': out += '\''; break;
                case '"': out += '"'; break;
                case 'a': out += '\a'; break;
                case 'f': out += '\f'; break;
                case 'r': out += '\r'; break;
                case 'b': out += '\b'; break;
                case 'v': out += '\v'; break;
                case '0': out += '\0'; break;
                default: out += c; break; // fallback: keep literal char
            }
            i++; // skip the escaped char
        } else {
            out += s[i];
        }
    }
    return out;
}
ofstream tokenOut;  
SymbolTable* symTab;
int errorCount = 0;

string strbuf, strbuf2, combuf;
int startLine = 0;

void logFound(const string& token, const string& lexeme) {
    if(token == "CONST_CHAR"){
        string lex= lexeme.substr(1, lexeme.size() - 2);
        cout << "Line no " << yylineno << ": Token <" << token<< "> Lexeme " << lexeme << " found --> <"<<token<<", "<<unescape(lex) <<">"<< endl<<endl;
        return;
    }
    cout << "Line no " << yylineno << ": Token <" << token
         << "> Lexeme " << lexeme << " found" << endl<<endl;
}
void logFound(const string& token, const string& lexeme, const string& lexeme2) {
    string lex= lexeme.substr(1, lexeme.size() - 2);
    cout << "Line no " << yylineno << ": Token <" << token<< "> Lexeme " << lexeme2 << " found --> <"<<token<<", "<<unescape(lex) <<">"<< endl<<endl;
    return;
}
void logError(const string& msg) {
    errorCount++;
    cout << "Error at line no " << yylineno << ": " << msg << endl;
}
void emitToken(const string& type) {
    tokenOut << "<" << type << "> ";
    logFound(type, yytext);
}
void emitToken(const string& type, const string& lexeme) {
    if(type=="CONST_CHAR"){
        string lex=lexeme.substr(1,lexeme.size()-2);
        tokenOut << "<" << type << ", " << unescape(lex) << "> ";
        logFound(type, lexeme);
        return;
    }
    tokenOut << "<" << type << ", " << lexeme << "> ";
    logFound(type, lexeme);
}

void emitToken(const string& type, const string& lexeme, const string& lexeme2) {

    string lex=lexeme.substr(1,lexeme.size()-2);
    tokenOut << "<" << type << ", " << unescape(lex) << "> ";
    logFound(type, lexeme, lexeme2);
    return;

}

void handleNumber(const string& raw) {
    if (regex_match(raw, regex("^[0-9]+$"))) {
        emitToken("CONST_INT", raw);
        if (!symTab->Insert(raw, "CONST_INT")) {
            return;// already exists inside Insert
        }
        symTab->PrintAll();
        return;
    }
    // valid float
    if (regex_match(raw, regex("^([0-9]+\\.[0-9]+|\\.[0-9]+)([eE][+-]?[0-9]+)?$")) ||
        regex_match(raw, regex("^[0-9]+[eE][+-]?[0-9]+$"))) {
        emitToken("CONST_FLOAT", raw);
        if (!symTab->Insert(raw, "CONST_FLOAT")) {
            return;// already exists inside Insert
        }
        symTab->PrintAll();
        return;
    }
    int dotCount = count(raw.begin(), raw.end(), '.');
    size_t ePos = raw.find_first_of("eE");
    bool hasTrailingAlpha = regex_search(raw, regex("[a-zA-Z_][a-zA-Z0-9_]*$")) &&
                             !regex_search(raw, regex("[eE][+-]?[0-9]+$"));

    if (dotCount >= 2) {
        logError("Too many decimal points " + raw+"\n\n");
    } else if (ePos != string::npos && raw.find('.', ePos) != string::npos) {
        // dot appears AFTER the exponent marker -> e.g. 1E10.7
        logError("Ill formed number " + raw+"\n\n");
    } else if (hasTrailingAlpha) {
        logError("Invalid prefix on ID or invalid suffix on Number " + raw+"\n");
    } else {
        logError("Ill formed number " + raw+"\n\n");
    }
}

%}

DIGIT[0-9]
ALPHA[a-zA-Z_]
ALNUM[a-zA-Z0-9_]
NUMLIKE(\.)?{DIGIT}+([.eE][0-9eE+\-]*)*{ALPHA}*{ALNUM}*

%%

"/*" { 
    BEGIN(COMMENT); 
    combuf = yytext; 
    startLine = yylineno; 
}
<COMMENT>"*/" { 
    combuf += yytext; 
    BEGIN(INITIAL);
    logFound("COMMENT", combuf); 
}
<COMMENT>\n { 
    combuf += '\n'; 
}
<COMMENT><<EOF>> { 
    cout<< "Error at line no "<<startLine<<": Unterminated comment "<<combuf<<endl<<endl;
    errorCount++; 
    BEGIN(INITIAL);
}
<COMMENT>. { 
    combuf += yytext[0]; 
}


"//" { 
    BEGIN(LCOMMENT); 
    combuf = yytext; 
    startLine = yylineno; 
}
<LCOMMENT>\\\n { 
    combuf += "\\\n"; 
} 
<LCOMMENT>\n { 
    BEGIN(INITIAL); 
    int line = yylineno - 1;
    cout << "Line no " << line << ": Token <COMMENT> Lexeme " << combuf << " found" << endl << endl;
}
<LCOMMENT><<EOF>> { 
    BEGIN(INITIAL); 
    logFound("COMMENT", combuf); 
}
<LCOMMENT>. { 
    combuf += yytext[0]; 
}


\" { 
    BEGIN(STR); 
    strbuf = ""; 
    strbuf2 = "";
    startLine = yylineno; 
}
<STR>\\\n { 
    strbuf2 += string(yytext);
    strbuf += "\\"; 
}
<STR>\\. { 
    string str = string(yytext);
    strbuf += str; 
    strbuf2 += str;
}
<STR>\" { 
    BEGIN(INITIAL);
    string str = "\""+strbuf+"\"";
    string str2 = "\""+strbuf2+"\"";
    emitToken("STRING", str, str2); 
}
<STR>\n { 
    cout<<"Error at line no "<< startLine <<": Unterminated string \"" << strbuf2<<"\n"<<endl; 
    errorCount++;
    BEGIN(INITIAL); 
}
<STR><<EOF>> { 
    cout<<"Error at line no "<< startLine<<": Unterminated string \"" << strbuf2<<"\n"<<endl; 
    errorCount++;
    BEGIN(INITIAL); 
}
<STR>. { 
    char str = yytext[0];
    strbuf += str;
    strbuf2 += str;
}


' {
    BEGIN(CHAR);
    strbuf = "";
    startLine = yylineno;
}
<CHAR>' { 
    BEGIN(INITIAL);                                    
    logError("Empty character constant error ''\n\n"); 
}

<CHAR>([^'\\\n]|\\[ntrab0fv\\'"])' {
    string inner(yytext , yyleng - 1);
    string val = "\'"+inner+"\'" ;
    BEGIN(INITIAL);                                 
    emitToken("CONST_CHAR", val);
    if (!symTab->Insert(val, "CONST_CHAR")) {
    }
    else symTab->PrintAll();
}
<CHAR>\\' {
    BEGIN(INITIAL);
    logError("Unterminated character '" + string(yytext) + "\n\n");
}
<CHAR>([^'\n]|\\.)*' {
    BEGIN(INITIAL);                                  
    string inner(yytext + 1, yyleng - 2);
    logError("Multi character constant error \'" + string(yytext) + "\n\n");                                      
}

<CHAR>[^'\n]* { 
    BEGIN(INITIAL);                                   
    logError("Unterminated character \'" + string(yytext) + "\n\n");                                  
}

<CHAR>\n {
    int line = yylineno - 1;
    cout<<"Error at line no "<<line<<": Unterminated character \'" <<strbuf<< "\n\n"<<endl;
    errorCount++;
    BEGIN(INITIAL);                                    
}

<CHAR><<EOF>> {
    logError("Unterminated character \'" + strbuf + "\n\n");
    BEGIN(INITIAL);                                    
}


"if"       { emitToken("IF"); }
"else"     { emitToken("ELSE"); }
"for"      { emitToken("FOR"); }
"while"    { emitToken("WHILE"); }
"do"       { emitToken("DO"); }
"break"    { emitToken("BREAK"); }
"int"      { emitToken("INT"); }
"char"     { emitToken("CHAR"); }
"float"    { emitToken("FLOAT"); }
"double"   { emitToken("DOUBLE"); }
"void"     { emitToken("VOID"); }
"return"   { emitToken("RETURN"); }
"switch"   { emitToken("SWITCH"); }
"case"     { emitToken("CASE"); }
"default"  { emitToken("DEFAULT"); }
"continue" { emitToken("CONTINUE"); }
"goto"     { emitToken("GOTO"); }
"long"     { emitToken("LONG"); }
"short"    { emitToken("SHORT"); }
"static"   { emitToken("STATIC"); }
"unsigned" { emitToken("UNSIGNED"); }


{NUMLIKE}  { handleNumber(yytext); }


{ALPHA}{ALNUM}* {
    emitToken("ID", yytext);
    if (!symTab->Insert(yytext, "ID")) {
        // already logged "already exists" inside Insert
    }
    else symTab->PrintAll();
}


"++"   { emitToken("INCOP", yytext); }
"--"   { emitToken("INCOP", yytext); }
"+"    { emitToken("ADDOP", yytext); }
"-"    { emitToken("ADDOP", yytext); }
"*"    { emitToken("MULOP", yytext); }
"/"    { emitToken("MULOP", yytext); }
"%"    { emitToken("MULOP", yytext); }
"<="   { emitToken("RELOP", yytext); }
">="   { emitToken("RELOP", yytext); }
"=="   { emitToken("RELOP", yytext); }
"!="   { emitToken("RELOP", yytext); }
"<"    { emitToken("RELOP", yytext); }
">"    { emitToken("RELOP", yytext); }
"="    { emitToken("ASSIGNOP", yytext); }
"&&"   { emitToken("LOGICOP", yytext); }
"||"   { emitToken("LOGICOP", yytext); }
"!"    { emitToken("NOT", yytext); }
"("    { emitToken("LPAREN", yytext); }
")"    { emitToken("RPAREN", yytext); }
"{"    { emitToken("LCURL", yytext); symTab->EnterScope(); }
"}"    { emitToken("RCURL", yytext); symTab->ExitScope(); }
"["    { emitToken("LTHIRD", yytext); }
"]"    { emitToken("RTHIRD", yytext); }
","    { emitToken("COMMA", yytext); }
";"    { emitToken("SEMICOLON", yytext); }


\n { }
[ \t\r]+ { }

. { logError(string("Unrecognized character ") + yytext +"\n"); }

%%

int main(int argc,char *argv[]){

    if (argc != 2) {
        printf("Please provide input file name and try again\n");
        return 1;
    }
    FILE* in = fopen(argv[1], "r");
    if (!in) { 
        printf("Cannot open specified file\n");
        return 0;
    }

    yyin = in;

    string studentId = "2205029";
    tokenOut.open(studentId + "_token.txt");
    ofstream logOut(studentId + "_log.txt");
    streambuf* oldCout = cout.rdbuf(logOut.rdbuf());

    symTab = new SymbolTable();
    yylex();
    symTab->PrintAll();
    cout << "Total lines: " << yylineno << endl;
    cout << "Total errors: " << errorCount << endl;

    cout.rdbuf(oldCout);
    tokenOut.close();
    fclose(in);
    return 0;
}