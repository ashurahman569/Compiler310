#include "ID2205029_Visitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace antlr4;
using namespace std;

string safeCastString(const any &result) {
    if (result.has_value()) {
        try {
            return any_cast<string>(result);
        } catch (const bad_any_cast &) {
            return "";
        }
    }
    return "";
}
string varType;
bool isVoid = false;
bool returningVoid = false;
vector<string> typeStack;
vector<string> ArrayMismatch;
int paramOrdinal = 0;
string currentFuncNameForParams;
string nextsyminparamlist = "";

ID2205029_Visitor::ID2205029_Visitor(const string &logPath, const string &errorPath) {
    logFile.open(logPath);
    errorFile.open(errorPath);
}

ID2205029_Visitor::~ID2205029_Visitor() {
    if (logFile.is_open()) {
        logFile << "Total lines: " << lineCount << "\n";
        logFile << "Total errors: " << errorCount << "\n";
        logFile.close();
    }
    if (errorFile.is_open()) errorFile.close();
}

void ID2205029_Visitor::logLine() {
    logFile << endl;
}

bool ID2205029_Visitor::isInt(const string& str) {
    if (str.empty()) return false;
    SymbolInfo* info = symbolTable.Lookup(str);
    if (info && info->varType == "int") {
        return true;    
    }
    try {
        size_t idx;
        stoi(str, &idx);
        return idx == str.length();
    } 
    catch (const invalid_argument&) {
        return false;
    } 
    catch (const out_of_range&) {
        return false;
    }
}

bool ID2205029_Visitor::isArrayMiss(const string& str) {
    for (const auto& name : ArrayMismatch) {
        if (name == str) {
            return true;
        }
    }
    return false;
}

bool ID2205029_Visitor::isFloat(const string& str) {
    if (str.empty()) return false;
    SymbolInfo* info = symbolTable.Lookup(str);
    if (info && info->varType == "float") {
        return true;    
    }
    try {
        size_t idx;
        stof(str, &idx);
        return idx == str.length();
    } 
    catch (const invalid_argument&) {
        return false;
    } 
    catch (const out_of_range&) {
        return false;
    }
}

string ID2205029_Visitor::getFullText(tree::ParseTree *tree) {
    if (!tree) return "";
    return tree->getText();
}

void ID2205029_Visitor::logRule(int line, const string &ruleLabel, const string &matchedText) {
    if (line > lineCount) {
        lineCount = line;
    }
    logFile << "Line " << line << ": " << ruleLabel << "\n\n" << matchedText << "\n\n";
}

void ID2205029_Visitor::logError(int line, const string &message) {
    errorCount++;
    logFile << "Error at line " << line << ": " << message << "\n\n";
    errorFile << "Error at line " << line << ": " << message << "\n\n";
}

// ---- start ----
any ID2205029_Visitor::visitStart(ID2205029_CSubsetParser::StartContext *ctx) {
    string progText = safeCastString(visit(ctx->program()));
    //logRule(ctx->getStart()->getLine(), "start : program", progText);
    logFile << "Line "<< ctx->getStart()->getLine() << ": start : program\n" ;
    symbolTable.PrintAll(logFile);
    if (ctx->getStop()) {
        lineCount = ctx->getStop()->getLine();
    }
    return progText;
}

// ---- program ----
any ID2205029_Visitor::visitProgramUnit(ID2205029_CSubsetParser::ProgramUnitContext *ctx) {
    string program = safeCastString(visit(ctx->program()));
    string unit = safeCastString(visit(ctx->unit()));
    string text = program + "\n" + unit;
    
    logLine();
    int line = ctx->unit()->getStart()->getLine();
    logRule(line, "program : program unit", text);
    logLine();
    return text;
}

any ID2205029_Visitor::visitUnitOnly(ID2205029_CSubsetParser::UnitOnlyContext *ctx) {
    string text = safeCastString(visit(ctx->unit()));
    logLine();
    logRule(ctx->getStart()->getLine(), "program : unit", text);
    logLine();
    return text;
}

// ---- unit ----
any ID2205029_Visitor::visitUnitVarDec(ID2205029_CSubsetParser::UnitVarDecContext *ctx) {
    string text = safeCastString(visit(ctx->var_declaration()));
    logRule(ctx->getStart()->getLine(), "unit : var_declaration", text);
    return text;
}

any ID2205029_Visitor::visitFuncDec(ID2205029_CSubsetParser::FuncDecContext *ctx) {
    string text = safeCastString(visit(ctx->func_declaration()));
    logRule(ctx->getStart()->getLine(), "unit : func_declaration", text);
    return text;
}

any ID2205029_Visitor::visitFuncDef(ID2205029_CSubsetParser::FuncDefContext *ctx) {
    string text = safeCastString(visit(ctx->func_definition()));
    logRule(ctx->getStart()->getLine(), "unit : func_definition", text);
    logLine();
    return text+"\n";
}

/// ---- func_declaration ----
any ID2205029_Visitor::visitFuncDecWithParam(ID2205029_CSubsetParser::FuncDecWithParamContext *ctx) {
    string type = safeCastString(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();

    symbolTable.EnterScope(30);
    string params = safeCastString(visit(ctx->parameter_list()));
    pendingParams.clear();
    string text = type + " " + funcName + "(" + params + ");";
    bool success = symbolTable.InsertParent(funcName, "ID " + type + " " + params, "function", true);
    if (!success) {
        logError(ctx->getStart()->getLine(), "Multiple declaration of " + funcName);
        symbolTable.ExitScope();
        return text;
    }
    logRule(ctx->getStart()->getLine(), "func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON", text+"\n");
    symbolTable.ExitScope();
    return text;
}

any ID2205029_Visitor::visitFuncDecNoParam(ID2205029_CSubsetParser::FuncDecNoParamContext *ctx) {
    string type = safeCastString(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();

    symbolTable.Insert(funcName, "ID "+type, "function", true);
    symbolTable.EnterScope(30);
    string text = type + " " + funcName + "();";
    logRule(ctx->getStart()->getLine(), "func_declaration : type_specifier ID LPAREN RPAREN SEMICOLON", text+"\n");
    symbolTable.ExitScope();
    return text;
}

// ---- func_definition ----
any ID2205029_Visitor::visitFuncDefWithParam(ID2205029_CSubsetParser::FuncDefWithParamContext *ctx) {
    string type = safeCastString(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();
    paramOrdinal = 0;
    currentFuncNameForParams = funcName;
    isFunctionDef = true;
    symbolTable.EnterScope(30);
    nextsyminparamlist = ")";
    string params = "";
    if (ctx->parameter_list()) {
        params = safeCastString(visit(ctx->parameter_list()));
    }
    bool success = symbolTable.InsertParent(funcName, "ID " + type + " " + params, "function", true, true);
    if (!success) {
        SymbolInfo* existing = symbolTable.Lookup(funcName);
        if (!existing->isFunction){
            logError(ctx->getStart()->getLine(), "Multiple declaration of " + funcName);
        }
        else if (existing->isDefined) {
            logError(ctx->getStart()->getLine(), "Multiple definition of " + funcName);
        } 
        else if (existing->retType != type) {
                logError(ctx->getStart()->getLine(), "Return type mismatch with function declaration in function " + funcName);
        } 
        else {
            stringstream iss(params);
            int argcount = 0;
            string paramToken;
            while (getline(iss, paramToken, ',')) {
                argcount++;
            }
            if (existing->numArgs != argcount) {
                logError(ctx->getStart()->getLine(), "Total number of arguments mismatch with declaration in function " + funcName);
            }
            else{
                existing->isDefined = true;
            }
        }
        
    }

    pendingParams.clear();

    string body = safeCastString(visit(ctx->compound_statement()));

    // symbolTable.PrintAll(logFile); 
    symbolTable.ExitScope();
    isFunctionDef = false;

    string text = type + " " + funcName + "(" + params + ")" + body;
    logRule(ctx->getStart()->getLine(), "func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement", text+"\n");
    return text;
}

any ID2205029_Visitor::visitFuncDefNoParam(ID2205029_CSubsetParser::FuncDefNoParamContext *ctx) {
    string type = safeCastString(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();
    paramOrdinal = 0;
    currentFuncNameForParams = funcName;
    bool success = symbolTable.Insert(funcName, "ID " + type, "function", true, true);
    if (!success) {
        SymbolInfo* existing = symbolTable.Lookup(funcName);
        if (!existing->isFunction){
            logError(ctx->getStart()->getLine(), "Multiple declaration of " + funcName);
        }
        else if (existing->isDefined) {
            logError(ctx->getStart()->getLine(), "Multiple definition of " + funcName);
        } else {
            if (existing->retType != type) {
                logError(ctx->getStart()->getLine(), "Return type mismatch with function declaration in function " + funcName);
            }
            else{
                existing->isDefined = true;
            }
        }
    }

    isFunctionDef = true;
    symbolTable.EnterScope(30);

    string body = safeCastString(visit(ctx->compound_statement()));

    // symbolTable.PrintAll(logFile);
    symbolTable.ExitScope();
    isFunctionDef = false;

    string text = type + " " + funcName + "()" + body;
    logRule(ctx->getStart()->getLine(), "func_definition : type_specifier ID LPAREN RPAREN compound_statement", text+"\n");
    return text;
}

// ---- parameter_list ----
any ID2205029_Visitor::visitParamListMultipleID(ID2205029_CSubsetParser::ParamListMultipleIDContext *ctx) {
    nextsyminparamlist = ","; 
    string paramList = safeCastString(visit(ctx->parameter_list()));
    string type = safeCastString(visit(ctx->type_specifier()));
    string ID = ctx->ID()->getText();
    paramOrdinal++;
    bool success = symbolTable.Insert(ID, "ID", type);
    if (!success) {
        logError(ctx->getStart()->getLine(), "Multiple declaration of " + ID+" in parameter");
    }
    string full = paramList + "," + type + " " + ID;
    logRule(ctx->getStart()->getLine(), "parameter_list : parameter_list COMMA type_specifier ID", full);
    return full;
}
any ID2205029_Visitor::visitParamListMultipleWithNoIDLast(ID2205029_CSubsetParser::ParamListMultipleWithNoIDLastContext *ctx) {
    nextsyminparamlist = ")"; 
    string paramList = safeCastString(visit(ctx->parameter_list()));
    string type = safeCastString(visit(ctx->type_specifier()));
    paramOrdinal++;
    string full = paramList + "," + type;
    logRule(ctx->getStart()->getLine(), "parameter_list : parameter_list COMMA type_specifier", full);
    return full;
}
any ID2205029_Visitor::visitParamListSingleID(ID2205029_CSubsetParser::ParamListSingleIDContext *ctx) {
    string type = safeCastString(visit(ctx->type_specifier()));
    paramOrdinal++;
    string ID = ctx->ID()->getText();
    bool success = symbolTable.Insert(ID, "ID", type);
    if (!success) {
        logError(ctx->getStart()->getLine(), "Multiple declaration of " + ID+" in parameter");
    }
    string full = type + " " + ID;
    logRule(ctx->getStart()->getLine(), "parameter_list : type_specifier ID", full);
    return full;
}
any ID2205029_Visitor::visitParamListNoID(ID2205029_CSubsetParser::ParamListNoIDContext *ctx) { 
    string type = safeCastString(visit(ctx->type_specifier()));
    paramOrdinal++;
    if (isFunctionDef) {
        logError(ctx->getStart()->getLine(), to_string(paramOrdinal) + "th parameter's name not given in function definition of " + currentFuncNameForParams);
    }
    logRule(ctx->getStart()->getLine(), "parameter_list : type_specifier", type);
    return type;
}
any ID2205029_Visitor::visitParamListGarbage(ID2205029_CSubsetParser::ParamListGarbageContext *ctx) {
    string type = safeCastString(visit(ctx->type_specifier()));
    paramOrdinal++;
    string bad = ctx->invalid()->getText();

    logRule(ctx->getStart()->getLine(), "parameter_list : type_specifier", type);   // FIRST, matches ParamListNoID's exact label

    logError(ctx->getStart()->getLine(), "syntax error, unexpected token(s) '" + bad + "' before '" + nextsyminparamlist + "'");
    if (isFunctionDef) {
        logError(ctx->getStart()->getLine(), to_string(paramOrdinal) + "th parameter's name not given in function definition of " + currentFuncNameForParams);
    }
    return type;
}
any ID2205029_Visitor::visitParamListMultipleGarbage(ID2205029_CSubsetParser::ParamListMultipleGarbageContext *ctx) {
    string prevList = safeCastString(visit(ctx->parameter_list()));
    string type = safeCastString(visit(ctx->type_specifier()));
    paramOrdinal++;
    string bad = ctx->invalid()->getText();
    string full = prevList + "," + type;

    logRule(ctx->getStart()->getLine(), "parameter_list : parameter_list COMMA type_specifier", full);

    logError(ctx->getStart()->getLine(), "syntax error, unexpected token(s) '" + bad + "' before '" + nextsyminparamlist + "'");
    if (isFunctionDef) {
        logError(ctx->getStart()->getLine(), to_string(paramOrdinal) + "th parameter's name not given in function definition of " + currentFuncNameForParams);
    }
    return full;
}

// ---- compound_statement ----
any ID2205029_Visitor::visitCodeBlock(ID2205029_CSubsetParser::CodeBlockContext *ctx) {
    bool newlyOpenedScope = false;
    
    if (!isFunctionDef) {
        symbolTable.EnterScope(30);
        newlyOpenedScope = true;
        isFunctionDef = false;
    
        string stmts = "";
        if (ctx->statements()) {
            stmts = safeCastString(visit(ctx->statements()));
        }

        string full = "{\n" + stmts + "\n}";
        logRule(ctx->getStart()->getLine(), "compound_statement : LCURL statements RCURL", full);

        // Print all active scope 
        symbolTable.PrintAll(logFile);
        symbolTable.ExitScope();

        return full;
    
    }
    isFunctionDef = false;
    
    string stmts = "";
    if (ctx->statements()) {
        stmts = safeCastString(visit(ctx->statements()));
    }

    string full = "{\n" + stmts + "\n}";
    logRule(ctx->getStart()->getLine(), "compound_statement : LCURL statements RCURL", full);
    symbolTable.PrintAll(logFile);
    return full;
}
any ID2205029_Visitor::visitBlankBlock(ID2205029_CSubsetParser::BlankBlockContext *ctx) {
    string text = "{}";
    logRule(ctx->getStart()->getLine(), "compound_statement : LCURL RCURL", text);
    symbolTable.PrintAll(logFile);
    return text;
}

// ---- var_declaration ----
any ID2205029_Visitor::visitTypeSpecifiedVarDeclarationList(ID2205029_CSubsetParser::TypeSpecifiedVarDeclarationListContext *ctx) {
    string type = safeCastString(visit(ctx->type_specifier()));
    if (type == "void") {
        isVoid = true;
    }
    string decList = safeCastString(visit(ctx->declaration_list()));
    if (type == "void") {
        logError(ctx->getStart()->getLine(), "Variable type cannot be void");
    }
    isVoid = false;
    string full = type + " " + decList + ";";
    logRule(ctx->getStart()->getLine(), "var_declaration : type_specifier declaration_list SEMICOLON", full);
    currentType="";
    return full;
}

// ---- type_specifier ----
any ID2205029_Visitor::visitTypeInt(ID2205029_CSubsetParser::TypeIntContext *ctx) { 
    string type = "int";
    logRule(ctx->getStart()->getLine(), "type_specifier : INT", type);
    currentType = "int";
    return type;
}

any ID2205029_Visitor::visitTypeFloat(ID2205029_CSubsetParser::TypeFloatContext *ctx) { 
    string type = "float";
    logRule(ctx->getStart()->getLine(), "type_specifier : FLOAT", type);
    currentType = "float";
    return type;
}

any ID2205029_Visitor::visitTypeVoid(ID2205029_CSubsetParser::TypeVoidContext *ctx) { 
    string type = "void";
    logRule(ctx->getStart()->getLine(), "type_specifier : VOID", type);
    currentType = "void";
    return type;
}   

// ---- declaration_list ----
any ID2205029_Visitor::visitMultipleIDDeclaration(ID2205029_CSubsetParser::MultipleIDDeclarationContext *ctx) { 
    string list = safeCastString(visit(ctx->declaration_list()));
    string ID = ctx->ID()->getText();
    string full = list + "," + ID;
    if(!isVoid) {
        bool success = symbolTable.Insert(ID, "ID", currentType, false, false, false);
        if (!success) {
            logError(ctx->getStart()->getLine(), "Multiple declaration of " + ID);
        }
    }
    logRule(ctx->getStart()->getLine(), "declaration_list : declaration_list COMMA ID", full);
    return full;
}

any ID2205029_Visitor::visitMutlipleIDDeclarationWithArray(ID2205029_CSubsetParser::MutlipleIDDeclarationWithArrayContext *ctx) { 
    string list = safeCastString(visit(ctx->declaration_list()));
    string ID = ctx->ID()->getText();
    string CONST_INT = ctx->CONST_INT()->getText();
    string full = list + "," + ID + "[" + CONST_INT + "]";   
    if(!isVoid) {
        bool success = symbolTable.Insert(ID, "ID", currentType, false, false, true);
        if (!success) {
            logError(ctx->getStart()->getLine(), "Multiple declaration of " + ID);
        }
    }
    logRule(ctx->getStart()->getLine(), "declaration_list : declaration_list COMMA ID LTHIRD CONST_INT RTHIRD", full);
    return full;
}

any ID2205029_Visitor::visitSingleIDDeclaration(ID2205029_CSubsetParser::SingleIDDeclarationContext *ctx) { 
    string varName = ctx->ID()->getText();
    if(!isVoid) {
        bool success = symbolTable.Insert(varName, "ID", currentType, false, false, false);
        if (!success) {
            logError(ctx->getStart()->getLine(), "Multiple declaration of " + varName);
        }
    }
    logRule(ctx->getStart()->getLine(), "declaration_list : ID", varName);
    return varName; 
}

any ID2205029_Visitor::visitSignleIDArrayDeclaration(ID2205029_CSubsetParser::SignleIDArrayDeclarationContext *ctx) { 
    string varName = ctx->ID()->getText();
    string size = ctx->CONST_INT()->getText();
    string full = varName + "[" + size + "]";
    if(!isVoid) {
        bool success = symbolTable.Insert(varName, "ID", currentType, false, false, true);
        if (!success) {
            logError(ctx->getStart()->getLine(), "Multiple declaration of " + varName);
        }
    }

    logRule(ctx->getStart()->getLine(), "declaration_list : ID LTHIRD CONST_INT RTHIRD", full);
    return full; 
}
any ID2205029_Visitor::visitDeclListGarbage(ID2205029_CSubsetParser::DeclListGarbageContext *ctx) {
    string varName = ctx->ID(0)->getText();
    symbolTable.Insert(varName, "ID", currentType, false, false, false);
    string bad = ctx->invalid()->getText() + " " + ctx->ID(1)->getText();

    logRule(ctx->getStart()->getLine(), "declaration_list : ID", varName);   // FIRST, no suffix

    logError(ctx->getStart()->getLine(), "syntax error, unexpected token(s) '" + bad + "' in declaration list");
    return varName;
}
any ID2205029_Visitor::visitMultipleDeclListGarbage(ID2205029_CSubsetParser::MultipleDeclListGarbageContext *ctx) {
    string prevList = safeCastString(visit(ctx->declaration_list()));
    string ID = ctx->ID(0)->getText();   // the valid ID right after COMMA (e.g. "q" in "p, q-r")
    symbolTable.Insert(ID, "ID", currentType, false, false, false);
    string bad = ctx->invalid()->getText() + " " + ctx->ID(1)->getText();
    string full = prevList + "," + ID;

    logRule(ctx->getStart()->getLine(), "declaration_list : declaration_list COMMA ID", full);
    logError(ctx->getStart()->getLine(), "syntax error, unexpected token(s) '" + bad + "' in declaration list");
    return full;
}

// ---- statements ----
any ID2205029_Visitor::visitSingleStatement(ID2205029_CSubsetParser::SingleStatementContext *ctx) { 
    string text = safeCastString(visit(ctx->statement()));
    logLine();
    logRule(ctx->statement()->getStart()->getLine(), "statements : statement", text);
    logLine();
    return text;
}

any ID2205029_Visitor::visitMultipleStatement(ID2205029_CSubsetParser::MultipleStatementContext *ctx) { 
    string statements = safeCastString(visit(ctx->statements()));
    string statement = safeCastString(visit(ctx->statement()));
    string full = statements + "\n" + statement;
    logLine();
    logRule(ctx->statement()->getStart()->getLine(), "statements : statements statement", full);
    logLine();
    return full;
}

// ---- statement ----
any ID2205029_Visitor::visitStatementVarDec(ID2205029_CSubsetParser::StatementVarDecContext *ctx) { 
    string text = safeCastString(visit(ctx->var_declaration()));
    logRule(ctx->getStart()->getLine(), "statement : var_declaration", text);
    returningVoid = false;
    return text;
}

any ID2205029_Visitor::visitSingleExpressionStatement(ID2205029_CSubsetParser::SingleExpressionStatementContext *ctx) { 
    string text = safeCastString(visit(ctx->expression_statement()));
    logRule(ctx->getStart()->getLine(), "statement : expression_statement", text);
    returningVoid = false;
    return text;
}

any ID2205029_Visitor::visitCompoundStatement(ID2205029_CSubsetParser::CompoundStatementContext *ctx) { 
    string text = safeCastString(visit(ctx->compound_statement()));
    logRule(ctx->getStart()->getLine(), "statement : compound_statement", text);
    logLine();
    returningVoid = false;
    return text;
}

any ID2205029_Visitor::visitForLoopStatement(ID2205029_CSubsetParser::ForLoopStatementContext *ctx) { 
    string expr1 = safeCastString(visit(ctx->expression_statement(0)));
    string expr2 = safeCastString(visit(ctx->expression_statement(1)));
    string expr3 = safeCastString(visit(ctx->expression()));
    string statement = safeCastString(visit(ctx->statement()));
    string full = "for(" + expr1 + expr2 + expr3 + ")" + statement;
    logRule(ctx->getStart()->getLine(), "statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement", full);
    returningVoid = false;
    return full;
}

any ID2205029_Visitor::visitIfStatement(ID2205029_CSubsetParser::IfStatementContext *ctx) {
    string expr = safeCastString(visit(ctx->expression()));
    string statement = safeCastString(visit(ctx->statement()));
    string full = "if (" + expr + ")" + statement;
    logRule(ctx->getStart()->getLine(), "statement : IF LPAREN expression RPAREN statement", full);
    returningVoid = false;
    return full;
}

any ID2205029_Visitor::visitIfElseStatement(ID2205029_CSubsetParser::IfElseStatementContext *ctx) { 
    string expr = safeCastString(visit(ctx->expression()));
    string statement1 = safeCastString(visit(ctx->statement(0)));
    string statement2 = safeCastString(visit(ctx->statement(1)));
    string full = "if (" + expr + ")" + statement1 + "\nelse\n" + statement2;
    logRule(ctx->getStart()->getLine(), "statement : IF LPAREN expression RPAREN statement ELSE statement", full);
    returningVoid = false;
    return full;
}

any ID2205029_Visitor::visitWhileLoopStatement(ID2205029_CSubsetParser::WhileLoopStatementContext *ctx) { 
    string expr = safeCastString(visit(ctx->expression()));
    string statement = safeCastString(visit(ctx->statement()));
    string full = "while (" + expr + ")" + statement;
    logRule(ctx->getStart()->getLine(), "statement : WHILE LPAREN expression RPAREN statement", full);
    returningVoid = false;
    return full;
}

any ID2205029_Visitor::visitPrintLineStatement(ID2205029_CSubsetParser::PrintLineStatementContext *ctx) {
    string id = ctx->ID()->getText();
    if (symbolTable.Lookup(id) == nullptr) {
        logError(ctx->getStart()->getLine(), "Undeclared variable " + id);
    }
    string full = "printf(" + id + ");";
    logRule(ctx->getStart()->getLine(), "statement : PRINTLN LPAREN ID RPAREN SEMICOLON", full);
    returningVoid = false;
    return full;
}

any ID2205029_Visitor::visitReturnExpressionStatement(ID2205029_CSubsetParser::ReturnExpressionStatementContext *ctx) {
    string expr = safeCastString(visit(ctx->expression()));
    string full = "return " + expr + ";";
    logRule(ctx->getStart()->getLine(), "statement : RETURN expression SEMICOLON", full);
    returningVoid = false;
    return full;
}

// ---- expression_statement ----
any ID2205029_Visitor::visitNoExpression(ID2205029_CSubsetParser::NoExpressionContext *ctx) { 
    string text = ";";
    logRule(ctx->getStart()->getLine(), "expression_statement : SEMICOLON", text);
    return text;
}

any ID2205029_Visitor::visitExpressionStatement(ID2205029_CSubsetParser::ExpressionStatementContext *ctx) { 
    string expr = safeCastString(visit(ctx->expression()));
    string sc = ctx->SEMICOLON() ? ctx->SEMICOLON()->getText() : "";
    if (sc != ";") {
        logError(ctx->getStart()->getLine(), "syntax error, missing ';' after expression '" + expr + "'");
        logRule(ctx->getStart()->getLine(), "expression_statement : expression (missing SEMICOLON)", expr);
        return expr;
    }
    string text = expr + sc;
    logRule(ctx->getStart()->getLine(), "expression_statement : expression SEMICOLON", text);
    return text;
}

// ---- variable ----
any ID2205029_Visitor::visitAnID(ID2205029_CSubsetParser::AnIDContext *ctx) {
    string varName = ctx->ID()->getText();
    SymbolInfo* exist = symbolTable.Lookup(varName);
    if (exist == nullptr) {
        logError(ctx->getStart()->getLine(), "Undeclared variable " + varName);
    } else if (exist->isArray) {
        logError(ctx->getStart()->getLine(), "Type mismatch, " + varName + " is an array");
        ArrayMismatch.push_back(varName);
    } 
    varType = exist ? exist->varType : "";
    logRule(ctx->getStart()->getLine(), "variable : ID", varName);
    return varName;
}

any ID2205029_Visitor::visitAnArrayIndex(ID2205029_CSubsetParser::AnArrayIndexContext *ctx) {
    string varName = ctx->ID()->getText();
    SymbolInfo* exist = symbolTable.Lookup(varName);

    string indexExpr = safeCastString(visit(ctx->expression()));
    if (exist == nullptr) {
        logError(ctx->getStart()->getLine(), "Undeclared variable " + varName);
    } else if (!exist->isArray) {
        logError(ctx->getStart()->getLine(), varName + " not an array");
        ArrayMismatch.push_back(varName);
    }  else if (!isInt(indexExpr)) {
        logError(ctx->getStart()->getLine(), "Expression inside third brackets not an integer");
    }  
    varType = exist ? exist->varType : "";
    string full = varName + "[" + indexExpr + "]";
    logRule(ctx->getStart()->getLine(), "variable : ID LTHIRD expression RTHIRD", full);
    return full;
}

// ---- expression ----
any ID2205029_Visitor::visitLogicalExpression(ID2205029_CSubsetParser::LogicalExpressionContext *ctx) { 
    string text = safeCastString(visit(ctx->logic_expression()));
    logRule(ctx->getStart()->getLine(), "expression : logic expression", text);
    return text;
}

any ID2205029_Visitor::visitAssignExpression(ID2205029_CSubsetParser::AssignExpressionContext *ctx) {
    string varName = safeCastString(visit(ctx->variable()));
    string expr = safeCastString(visit(ctx->logic_expression()));
    string rhsType = typeStack.empty() ? "" : typeStack.back();  
    if (!typeStack.empty()) typeStack.pop_back();  
    if(returningVoid) {
        logError(ctx->getStart()->getLine(), "Void function used in expression");
        returningVoid = false;
    } else if (varType != "" && rhsType != "" && varType != rhsType && varType != "float") {
        logError(ctx->getStart()->getLine(), "Type Mismatch");
    }
    string full = varName + "=" + expr;
    logRule(ctx->getStart()->getLine(), "expression : variable ASSIGNOP logic_expression", full);
    return full;
}

// ---- logic_expression ----
any ID2205029_Visitor::visitRelationalExpression(ID2205029_CSubsetParser::RelationalExpressionContext *ctx) { 
    string text = safeCastString(visit(ctx->rel_expression()));
    logRule(ctx->getStart()->getLine(), "logic_expression : rel_expression", text);
    return text;
}

any ID2205029_Visitor::visitMultipleRelationalExpression(ID2205029_CSubsetParser::MultipleRelationalExpressionContext *ctx) {
    string leftExpr = safeCastString(visit(ctx->rel_expression(0)));
    string rightExpr = safeCastString(visit(ctx->rel_expression(1)));
    string op = ctx->LOGICOP()->getText();
    string full = leftExpr+ op + rightExpr;
    if (!typeStack.empty()) typeStack.pop_back();
    if (!typeStack.empty()) typeStack.pop_back();
    typeStack.push_back("int"); // Logical expressions result in an int (0 or 1)
    logRule(ctx->getStart()->getLine(), "logic_expression : rel_expression LOGICOP rel_expression", full);
    return full;    
}

// ---- rel_expression ----
any ID2205029_Visitor::visitSimpleExpression(ID2205029_CSubsetParser::SimpleExpressionContext *ctx) { 
    string text = safeCastString(visit(ctx->simple_expression()));
    logRule(ctx->getStart()->getLine(), "rel_expression : simple_expression", text);
    return text;
}

any ID2205029_Visitor::visitSimpleCompareSimple(ID2205029_CSubsetParser::SimpleCompareSimpleContext *ctx) {
    string leftExpr = safeCastString(visit(ctx->simple_expression(0)));
    string rightExpr = safeCastString(visit(ctx->simple_expression(1)));
    string op = ctx->RELOP()->getText();
    string full = leftExpr + op + rightExpr;
    if (!typeStack.empty()) typeStack.pop_back();
    if (!typeStack.empty()) typeStack.pop_back();
    typeStack.push_back("int"); // Relational expressions result in an int (0 or 1)
    logRule(ctx->getStart()->getLine(), "rel_expression : simple_expression RELOP simple_expression", full);
    return full;
}

any ID2205029_Visitor::visitSimpleTerm(ID2205029_CSubsetParser::SimpleTermContext *ctx) { 
    string text = safeCastString(visit(ctx->term()));
    logRule(ctx->getStart()->getLine(), "simple_expression : term", text);
    return text;
}

any ID2205029_Visitor::visitSimpleExpressionAddTerm(ID2205029_CSubsetParser::SimpleExpressionAddTermContext *ctx) { 
    string leftExpr = safeCastString(visit(ctx->simple_expression()));
    string rightTerm = safeCastString(visit(ctx->term()));
    string rightType = typeStack.empty() ? "" : typeStack.back(); 
    if (!typeStack.empty()) typeStack.pop_back();
    string leftType  = typeStack.empty() ? "" : typeStack.back(); 
    if (!typeStack.empty()) typeStack.pop_back();
    string resultType = (leftType == "float" || rightType == "float") ? "float" : "int";
    typeStack.push_back(resultType);
    if (returningVoid) {
        logError(ctx->getStart()->getLine(), "Void function used in expression");
        returningVoid = false;
    }

    string op = ctx->ADDOP()->getText();
    string full = leftExpr + op + rightTerm;
    logRule(ctx->getStart()->getLine(), "simple_expression : simple_expression ADDOP term", full);
    return full;
}
any ID2205029_Visitor::visitSimpleAddInvalidTerm(ID2205029_CSubsetParser::SimpleAddInvalidTermContext *ctx) { 
    string left = safeCastString(visit(ctx->simple_expression()));
    string bad = ctx->invalid()->getText();
    logError(ctx->getStart()->getLine(), "syntax error, invalid operand '" + bad + "' after '+'");
    return left;
}
// ---- term ----
any ID2205029_Visitor::visitUnaryExpression(ID2205029_CSubsetParser::UnaryExpressionContext *ctx) { 
    string text = safeCastString(visit(ctx->unary_expression())); 
    logRule(ctx->getStart()->getLine(), "term : unary_expression", text);
    return text;
}

any ID2205029_Visitor::visitTermMultipliedUnaryExpression(ID2205029_CSubsetParser::TermMultipliedUnaryExpressionContext *ctx) {
    string leftTerm = safeCastString(visit(ctx->term()));
    string rightUnaryExpr = safeCastString(visit(ctx->unary_expression()));
    string rightType = typeStack.empty() ? "" : typeStack.back(); 
    if (!typeStack.empty()) typeStack.pop_back();
    string leftType  = typeStack.empty() ? "" : typeStack.back(); 
    if (!typeStack.empty()) typeStack.pop_back();

    string op = ctx->MULOP()->getText();

    string resultType;
    if (op == "%") {
        resultType = "int";
    } else {
        resultType = (leftType == "float" || rightType == "float") ? "float" : "int";
    }
    typeStack.push_back(resultType);
    // if (logicType != ""){
    //     if (isInt(leftTerm)) logicType = "int";
    //     else logicType = "float";
    // }
    if (returningVoid) {
        logError(ctx->getStart()->getLine(), "Void function used in expression");
        returningVoid = false;
    } else if (op == "%" && rightUnaryExpr == "0") {
        logError(ctx->getStart()->getLine(), "Modulus by Zero");
    }
    else if (op == "%" && !isInt(rightUnaryExpr)) {
        logError(ctx->getStart()->getLine(), "Non-Integer operand on modulus operator");
    }

    string full = leftTerm + op + rightUnaryExpr;
    logRule(ctx->getStart()->getLine(), "term : term MULOP unary_expression", full);
    return full;
}

// ---- unary_expression ----
any ID2205029_Visitor::visitAddUnaryExpression(ID2205029_CSubsetParser::AddUnaryExpressionContext *ctx) { 
    string unary = safeCastString(visit(ctx->unary_expression()));
    if (returningVoid) {
        logError(ctx->getStart()->getLine(), "Void function used in expression");
        returningVoid = false;
    }
    string plus = ctx->ADDOP()->getText();
    string text = plus + unary;

    logRule(ctx->getStart()->getLine(), "unary_expression : ADDOP unary_expression", text);
    return text;
}

any ID2205029_Visitor::visitNotUnaryExpression(ID2205029_CSubsetParser::NotUnaryExpressionContext *ctx) { 
    string notnot = ctx->NOT()->getText();
    string unary = safeCastString(visit(ctx->unary_expression()));
    if (returningVoid) {
        logError(ctx->getStart()->getLine(), "Void function used in expression");
        returningVoid = false;
    }
    string text = notnot + unary;
    logRule(ctx->getStart()->getLine(), "unary_expression : NOT unary expression", text);
    return text;
}

any ID2205029_Visitor::visitSingleFactor(ID2205029_CSubsetParser::SingleFactorContext *ctx) { 
    string text = safeCastString(visit(ctx->factor()));
    logRule(ctx->getStart()->getLine(), "unary_expression : factor", text);
    return text;
}

// ---- factor ----
any ID2205029_Visitor::visitFactorVariable(ID2205029_CSubsetParser::FactorVariableContext *ctx) { 
    string text = safeCastString(visit(ctx->variable()));
    SymbolInfo* full = symbolTable.Lookup(text);
    typeStack.push_back(full ? full->varType : "");
    logRule(ctx->getStart()->getLine(), "factor : variable", text);
    return text;
}

any ID2205029_Visitor::visitIDOfArgList(ID2205029_CSubsetParser::IDOfArgListContext *ctx) {
    string funcName = ctx->ID()->getText();
    SymbolInfo* exist = symbolTable.Lookup(funcName);
    string args = safeCastString(visit(ctx->argument_list()));

    if (exist == nullptr || !exist->isFunction) {
        logError(ctx->getStart()->getLine(), "Undeclared function " + funcName);
        typeStack.push_back("");
    }     
    else {
        // Collect evaluated types of the arguments from typeStack
        stringstream iss(args);
        int argcount = 0;
        string paramToken;
        while (getline(iss, paramToken, ',')) {
            argcount++;
        }

        if (exist->numArgs != argcount) {
            logError(ctx->getStart()->getLine(), "Total number of arguments mismatch in function " + funcName);
            typeStack.push_back(exist->retType);
        }
        else if (exist->retType == "void") {
            typeStack.push_back("");
            returningVoid = true;
        } else {
            // Retrieve the types of the arguments from typeStack
            vector<string> argTypes(argcount);
            for (int i = argcount - 1; i >= 0; i--) {
                if (!typeStack.empty()) {
                    argTypes[i] = typeStack.back();
                    typeStack.pop_back();
                }
            }

            // Perform type checking against expected parameter types
            for (int i = 0; i < argcount; i++) {
                string expectedType = exist->args[i];
                string actualType = argTypes[i];

                if (expectedType == "int" && actualType != "int") {
                    logError(ctx->getStart()->getLine(), to_string(i+1) + "th argument mismatch in function " + funcName);
                    break;
                } 
                else if (expectedType == "float" && actualType != "float" && actualType != "int") {
                    logError(ctx->getStart()->getLine(), to_string(i+1) + "th argument mismatch in function " + funcName);
                    break;
                }
            }
            typeStack.push_back(exist->retType);
        }
    }
    string full = funcName + "(" + args + ")";
    logRule(ctx->getStart()->getLine(), "factor : ID LPAREN argument_list RPAREN", full);
    return full;
}

any ID2205029_Visitor::visitBracketedExpression(ID2205029_CSubsetParser::BracketedExpressionContext *ctx) { 
    string expr = safeCastString(visit(ctx->expression()));
    string text = "(" + expr + ")";
    logRule(ctx->getStart()->getLine(), "factor : LPAREN expression RPAREN", text);
    return text;
}

any ID2205029_Visitor::visitConstInt(ID2205029_CSubsetParser::ConstIntContext *ctx) { 
    string text = ctx->CONST_INT()->getText();
    typeStack.push_back("int");
    logRule(ctx->getStart()->getLine(), "factor : CONST_INT", text);
    return text;
}

any ID2205029_Visitor::visitConstFloat(ID2205029_CSubsetParser::ConstFloatContext *ctx) { 
    float fVal = stof(ctx->CONST_FLOAT()->getText());
    ostringstream stream;
    stream << fixed << setprecision(2) << fVal;
    string text = stream.str();
    typeStack.push_back("float");
    logRule(ctx->getStart()->getLine(), "factor : CONST_FLOAT", text);
    return text;
}

any ID2205029_Visitor::visitVariableIncrement(ID2205029_CSubsetParser::VariableIncrementContext *ctx) { 
    string text = safeCastString(visit(ctx->variable()));
    string full = text + "++";
    logRule(ctx->getStart()->getLine(), "factor : variable INCOP", full);
    return full;
}

any ID2205029_Visitor::visitVariableDecrement(ID2205029_CSubsetParser::VariableDecrementContext *ctx) { 
    string text = safeCastString(visit(ctx->variable()));
    string full = text + "--";
    logRule(ctx->getStart()->getLine(), "factor : variable DECOP", full);
    return full;
}

// ---- argument_list ----
any ID2205029_Visitor::visitNonEmptyArguments(ID2205029_CSubsetParser::NonEmptyArgumentsContext *ctx) { 
    string text = safeCastString(visit(ctx->arguments()));
    logRule(ctx->getStart()->getLine(), "argument_list : arguments", text);
    return text;
}

any ID2205029_Visitor::visitBlankArgument(ID2205029_CSubsetParser::BlankArgumentContext *ctx) { 
    logRule(ctx->getStart()->getLine(), "argument_list : ", "");
    return string("");
}

// ---- arguments ----
any ID2205029_Visitor::visitMultipleArguments(ID2205029_CSubsetParser::MultipleArgumentsContext *ctx) { 
    string argList = safeCastString(visit(ctx->arguments()));
    string expr = safeCastString(visit(ctx->logic_expression()));
    if (returningVoid) {                                                     // ADD
        logError(ctx->getStart()->getLine(), "Void function used in expression");
        returningVoid = false;
    }
    string full = argList + "," + expr;
    logRule(ctx->getStart()->getLine(), "arguments : arguments COMMA logic_expression", full);
    return full;
}

any ID2205029_Visitor::visitSingleArgumentLogic(ID2205029_CSubsetParser::SingleArgumentLogicContext *ctx) { 
    string text = safeCastString(visit(ctx->logic_expression()));
    if (returningVoid) {                                                     // ADD
        logError(ctx->getStart()->getLine(), "Void function used in expression");
        returningVoid = false;
    }
    logRule(ctx->getStart()->getLine(), "arguments : logic_expression", text);
    return text;
}

any ID2205029_Visitor::visitInvalid(ID2205029_CSubsetParser::InvalidContext *ctx) { 
    return ctx->getText();
}
