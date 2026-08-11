#include "ID2205029_Visitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace antlr4;
using namespace std;

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
    errorFile << "Line no " << line << ": " << message << "\n";
}

// ---- start ----
any ID2205029_Visitor::visitStart(ID2205029_CSubsetParser::StartContext *ctx) {
    string progText = any_cast<string>(visit(ctx->program()));
    //logRule(ctx->getStart()->getLine(), "start : program", progText);
    logFile << "Line "<< ctx->getStart()->getLine() << ": start : program\n\n\n" ;
    symbolTable.PrintAll(logFile);
    if (ctx->getStop()) {
        lineCount = ctx->getStop()->getLine();
    }
    return progText;
}

// ---- program ----
any ID2205029_Visitor::visitProgramUnit(ID2205029_CSubsetParser::ProgramUnitContext *ctx) {
    string program = any_cast<string>(visit(ctx->program()));
    string unit = any_cast<string>(visit(ctx->unit()));
    string text = program + "\n" + unit;
    
    logLine();
    int line = ctx->unit()->getStart()->getLine();
    logRule(line, "program : program unit", text);
    logLine();
    return text;
}

any ID2205029_Visitor::visitUnitOnly(ID2205029_CSubsetParser::UnitOnlyContext *ctx) {
    string text = any_cast<string>(visit(ctx->unit()));
    logLine();
    logRule(ctx->getStart()->getLine(), "program : unit", text);
    logLine();
    return text;
}

// ---- unit ----
any ID2205029_Visitor::visitUnitVarDec(ID2205029_CSubsetParser::UnitVarDecContext *ctx) {
    string text = any_cast<string>(visit(ctx->var_declaration()));
    logRule(ctx->getStart()->getLine(), "unit : var_declaration", text);
    return text;
}

any ID2205029_Visitor::visitFuncDec(ID2205029_CSubsetParser::FuncDecContext *ctx) {
    string text = any_cast<string>(visit(ctx->func_declaration()));
    logRule(ctx->getStart()->getLine(), "unit : func_declaration", text);
    return text;
}

any ID2205029_Visitor::visitFuncDef(ID2205029_CSubsetParser::FuncDefContext *ctx) {
    string text = any_cast<string>(visit(ctx->func_definition()));
    logRule(ctx->getStart()->getLine(), "unit : func_definition", text);
    return text;
}

/// ---- func_declaration ----
any ID2205029_Visitor::visitFuncDecWithParam(ID2205029_CSubsetParser::FuncDecWithParamContext *ctx) {
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();
    string params = any_cast<string>(visit(ctx->parameter_list()));

    symbolTable.Insert(funcName, "ID");
    symbolTable.EnterScope(30);

    string text = type + " " + funcName + "(" + params + ");";
    logRule(ctx->getStart()->getLine(), "func_declaration : type_specifier ID LPAREN parameter_list RPAREN SEMICOLON", text);
    symbolTable.ExitScope();
    return text;
}

any ID2205029_Visitor::visitFuncDecNoParam(ID2205029_CSubsetParser::FuncDecNoParamContext *ctx) {
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();
    
    symbolTable.Insert(funcName, "ID");
    symbolTable.EnterScope(30);
    string text = type + " " + funcName + "();";
    logRule(ctx->getStart()->getLine(), "func_declaration : type_specifier ID LPAREN RPAREN SEMICOLON", text);
    symbolTable.ExitScope();
    return text;
}

// ---- func_definition ----
any ID2205029_Visitor::visitFuncDefWithParam(ID2205029_CSubsetParser::FuncDefWithParamContext *ctx) {
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();
    symbolTable.Insert(funcName, "ID");

    isFunctionDef = true;
    symbolTable.EnterScope(30);

    string params = "";
    if (ctx->parameter_list()) {
        params = any_cast<string>(visit(ctx->parameter_list()));
    }
    
    for (auto &p : pendingParams) {
        symbolTable.Insert(p.first, "ID");
    }
    pendingParams.clear();

    string body = any_cast<string>(visit(ctx->compound_statement()));

    // symbolTable.PrintAll(logFile); 
    // symbolTable.ExitScope();
    isFunctionDef = false;

    string text = type + " " + funcName + "(" + params + ")" + body;
    logRule(ctx->getStart()->getLine(), "func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement", text);
    return text;
}

any ID2205029_Visitor::visitFuncDefNoParam(ID2205029_CSubsetParser::FuncDefNoParamContext *ctx) {
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string funcName = ctx->ID()->getText();
    symbolTable.Insert(funcName, "ID");

    isFunctionDef = true;
    symbolTable.EnterScope(30);

    string body = any_cast<string>(visit(ctx->compound_statement()));

    // symbolTable.PrintAll(logFile);
    // symbolTable.ExitScope();
    isFunctionDef = false;

    string text = type + " " + funcName + "()" + body;
    logRule(ctx->getStart()->getLine(), "func_definition : type_specifier ID LPAREN RPAREN compound_statement", text);
    return text;
}

// ---- parameter_list ----
any ID2205029_Visitor::visitParamListMultipleID(ID2205029_CSubsetParser::ParamListMultipleIDContext *ctx) {
    string paramList = any_cast<string>(visit(ctx->parameter_list()));
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string ID = ctx->ID()->getText();
    pendingParams.push_back({ID, type});
    string full = paramList + "," + type + " " + ID;
    logRule(ctx->getStart()->getLine(), "parameter_list : parameter_list COMMA type_specifier ID", full);
    return full;
}
any ID2205029_Visitor::visitParamListMultipleWithNoIDLast(ID2205029_CSubsetParser::ParamListMultipleWithNoIDLastContext *ctx) {
    string paramList = any_cast<string>(visit(ctx->parameter_list()));
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string full = paramList + "," + type;
    logRule(ctx->getStart()->getLine(), "parameter_list : parameter_list COMMA type_specifier", full);
    return full;
}
any ID2205029_Visitor::visitParamListSingleID(ID2205029_CSubsetParser::ParamListSingleIDContext *ctx) {
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string ID = ctx->ID()->getText();
    pendingParams.push_back({ID, type});
    string full = type + " " + ID;
    logRule(ctx->getStart()->getLine(), "parameter_list : type_specifier ID", full);
    return full;
}
any ID2205029_Visitor::visitParamListNoID(ID2205029_CSubsetParser::ParamListNoIDContext *ctx) {
    string type = any_cast<string>(visit(ctx->type_specifier()));
    logRule(ctx->getStart()->getLine(), "parameter_list : type_specifier", type);
    return type;
}

// ---- compound_statement ----
any ID2205029_Visitor::visitCodeBlock(ID2205029_CSubsetParser::CodeBlockContext *ctx) {
    bool newlyOpenedScope = false;
    
    if (!isFunctionDef) {
        symbolTable.EnterScope(30);
        newlyOpenedScope = true;
    }
    isFunctionDef = false;

    string stmts = "";
    if (ctx->statements()) {
        stmts = any_cast<string>(visit(ctx->statements()));
    }

    string full = "{\n" + stmts + "\n}";
    logRule(ctx->getStart()->getLine(), "compound_statement : LCURL statements RCURL", full);

    // Print all active scope 
    symbolTable.PrintAll(logFile);
    symbolTable.ExitScope();

    return full;
}
any ID2205029_Visitor::visitBlankBlock(ID2205029_CSubsetParser::BlankBlockContext *ctx) {
    string text = "{ }";
    logRule(ctx->getStart()->getLine(), "compound_statement : LCURL RCURL", text);
    symbolTable.PrintAll(logFile);
    return text;
}

// ---- var_declaration ----
any ID2205029_Visitor::visitTypeSpecifiedVarDeclarationList(ID2205029_CSubsetParser::TypeSpecifiedVarDeclarationListContext *ctx) {
    string type = any_cast<string>(visit(ctx->type_specifier()));
    string decList = any_cast<string>(visit(ctx->declaration_list()));
    string full = type + " " + decList + ";";
    logRule(ctx->getStart()->getLine(), "var_declaration : type_specifier declaration_list SEMICOLON", full);
    return full;
}

// ---- type_specifier ----
any ID2205029_Visitor::visitTypeInt(ID2205029_CSubsetParser::TypeIntContext *ctx) { 
    string type = "int";
    logRule(ctx->getStart()->getLine(), "type_specifier : INT", type);
    currentType = "INT";
    return type;
}

any ID2205029_Visitor::visitTypeFloat(ID2205029_CSubsetParser::TypeFloatContext *ctx) { 
    string type = "float";
    logRule(ctx->getStart()->getLine(), "type_specifier : FLOAT", type);
    currentType = "FLOAT";
    return type;
}

any ID2205029_Visitor::visitTypeVoid(ID2205029_CSubsetParser::TypeVoidContext *ctx) { 
    string type = "void";
    logRule(ctx->getStart()->getLine(), "type_specifier : VOID", type);
    currentType = "VOID";
    return type;
}   

// ---- declaration_list ----
any ID2205029_Visitor::visitMultipleIDDeclaration(ID2205029_CSubsetParser::MultipleIDDeclarationContext *ctx) { 
    string list = any_cast<string>(visit(ctx->declaration_list()));
    string ID = ctx->ID()->getText();
    symbolTable.Insert(ID, "ID");
    string full = list + "," + ID;
    logRule(ctx->getStart()->getLine(), "declaration_list : declaration_list COMMA ID", full);
    return full;
}

any ID2205029_Visitor::visitMutlipleIDDeclarationWithArray(ID2205029_CSubsetParser::MutlipleIDDeclarationWithArrayContext *ctx) { 
    string list = any_cast<string>(visit(ctx->declaration_list()));
    string ID = ctx->ID()->getText();
    string CONST_INT = ctx->CONST_INT()->getText();
    symbolTable.Insert(ID, "ID");
    string full = list + "," + ID + "[" + CONST_INT + "]";
    logRule(ctx->getStart()->getLine(), "declaration_list : declaration_list COMMA ID LTHIRD CONST_INT RTHIRD", full);
    return full;
}

any ID2205029_Visitor::visitSingleIDDeclaration(ID2205029_CSubsetParser::SingleIDDeclarationContext *ctx) { 
    string varName = ctx->ID()->getText();
    symbolTable.Insert(varName, "ID");
    logRule(ctx->getStart()->getLine(), "declaration_list : ID", varName);
    return varName; 
}

any ID2205029_Visitor::visitSignleIDArrayDeclaration(ID2205029_CSubsetParser::SignleIDArrayDeclarationContext *ctx) { 
    string varName = ctx->ID()->getText();
    string size = ctx->CONST_INT()->getText();
    symbolTable.Insert(varName, "ID");
    string full = varName + "[" + size + "]";
    logRule(ctx->getStart()->getLine(), "declaration_list : ID LTHIRD CONST_INT RTHIRD", full);
    return full; 
}

// ---- statements ----
any ID2205029_Visitor::visitSingleStatement(ID2205029_CSubsetParser::SingleStatementContext *ctx) { 
    string text = any_cast<string>(visit(ctx->statement()));
    logLine();
    logRule(ctx->statement()->getStart()->getLine(), "statements : statement", text);
    logLine();
    return text;
}

any ID2205029_Visitor::visitMultipleStatement(ID2205029_CSubsetParser::MultipleStatementContext *ctx) { 
    string statements = any_cast<string>(visit(ctx->statements()));
    string statement = any_cast<string>(visit(ctx->statement()));
    string full = statements + "\n" + statement;
    logRule(ctx->statement()->getStart()->getLine(), "statements : statements statement", full);
    return full;
}

// ---- statement ----
any ID2205029_Visitor::visitStatementVarDec(ID2205029_CSubsetParser::StatementVarDecContext *ctx) { 
    string text = any_cast<string>(visit(ctx->var_declaration()));
    logRule(ctx->getStart()->getLine(), "statement : var_declaration", text);
    return text;
}

any ID2205029_Visitor::visitSingleExpressionStatement(ID2205029_CSubsetParser::SingleExpressionStatementContext *ctx) { 
    string text = any_cast<string>(visit(ctx->expression_statement()));
    logRule(ctx->getStart()->getLine(), "statement : expression_statement", text);
    return text;
}

any ID2205029_Visitor::visitCompoundStatement(ID2205029_CSubsetParser::CompoundStatementContext *ctx) { 
    string text = any_cast<string>(visit(ctx->compound_statement()));
    logRule(ctx->getStart()->getLine(), "statement : compound_statement", text);
    return text;
}

any ID2205029_Visitor::visitForLoopStatement(ID2205029_CSubsetParser::ForLoopStatementContext *ctx) { 
    string expr1 = any_cast<string>(visit(ctx->expression_statement(0)));
    string expr2 = any_cast<string>(visit(ctx->expression_statement(1)));
    string expr3 = any_cast<string>(visit(ctx->expression()));
    string statement = any_cast<string>(visit(ctx->statement()));
    string full = "for(" + expr1 + expr2 + expr3 + ")" + statement;
    logRule(ctx->getStart()->getLine(), "statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement", full);
    return full;
}

any ID2205029_Visitor::visitIfStatement(ID2205029_CSubsetParser::IfStatementContext *ctx) {
    string expr = any_cast<string>(visit(ctx->expression()));
    string statement = any_cast<string>(visit(ctx->statement()));
    string full = "if (" + expr + ")" + statement;
    logRule(ctx->getStart()->getLine(), "statement : IF LPAREN expression RPAREN statement", full);
    return full;
}

any ID2205029_Visitor::visitIfElseStatement(ID2205029_CSubsetParser::IfElseStatementContext *ctx) { 
    string expr = any_cast<string>(visit(ctx->expression()));
    string statement1 = any_cast<string>(visit(ctx->statement(0)));
    string statement2 = any_cast<string>(visit(ctx->statement(1)));
    string full = "if (" + expr + ")" + statement1 + "\nelse\n" + statement2;
    logRule(ctx->getStart()->getLine(), "statement : IF LPAREN expression RPAREN statement ELSE statement", full);
    return full;
}

any ID2205029_Visitor::visitWhileLoopStatement(ID2205029_CSubsetParser::WhileLoopStatementContext *ctx) { 
    string expr = any_cast<string>(visit(ctx->expression()));
    string statement = any_cast<string>(visit(ctx->statement()));
    string full = "while (" + expr + ")" + statement;
    logRule(ctx->getStart()->getLine(), "statement : WHILE LPAREN expression RPAREN statement", full);
    return full;
}

any ID2205029_Visitor::visitPrintLineStatement(ID2205029_CSubsetParser::PrintLineStatementContext *ctx) {
    string id = ctx->ID()->getText();
    string full = "printf(" + id + ");";
    logRule(ctx->getStart()->getLine(), "statement : PRINTLN LPAREN ID RPAREN SEMICOLON", full);
    return full;
}

any ID2205029_Visitor::visitReturnExpressionStatement(ID2205029_CSubsetParser::ReturnExpressionStatementContext *ctx) {
    string expr = any_cast<string>(visit(ctx->expression()));
    string full = "return " + expr + ";";
    logRule(ctx->getStart()->getLine(), "statement : RETURN expression SEMICOLON", full);
    return full;
}

// ---- expression_statement ----
any ID2205029_Visitor::visitNoExpression(ID2205029_CSubsetParser::NoExpressionContext *ctx) { 
    string text = ";";
    logRule(ctx->getStart()->getLine(), "expression_statement : SEMICOLON", text);
    return text;
}

any ID2205029_Visitor::visitExpressionStatement(ID2205029_CSubsetParser::ExpressionStatementContext *ctx) { 
    string expr = any_cast<string>(visit(ctx->expression()));
    string text = expr + ";";
    logRule(ctx->getStart()->getLine(), "expression_statement : expression SEMICOLON", text);
    return text;
}

// ---- variable ----
any ID2205029_Visitor::visitAnID(ID2205029_CSubsetParser::AnIDContext *ctx) {
    string varName = ctx->ID()->getText();
    logRule(ctx->getStart()->getLine(), "variable : ID", varName);
    return varName;
}

any ID2205029_Visitor::visitAnArrayIndex(ID2205029_CSubsetParser::AnArrayIndexContext *ctx) {
    string varName = ctx->ID()->getText();
    string indexExpr = any_cast<string>(visit(ctx->expression()));  
    string full = varName + "[" + indexExpr + "]";
    logRule(ctx->getStart()->getLine(), "variable : ID LTHIRD expression RTHIRD", full);
    return full;
}

// ---- expression ----
any ID2205029_Visitor::visitLogicalExpression(ID2205029_CSubsetParser::LogicalExpressionContext *ctx) { 
    string text = any_cast<string>(visit(ctx->logic_expression()));
    logRule(ctx->getStart()->getLine(), "expression : logic expression", text);
    return text;
}

any ID2205029_Visitor::visitAssignExpression(ID2205029_CSubsetParser::AssignExpressionContext *ctx) {
    string varName = any_cast<string>(visit(ctx->variable()));
    string expr = any_cast<string>(visit(ctx->logic_expression()));
    string full = varName + "=" + expr;
    logRule(ctx->getStart()->getLine(), "expression : variable ASSIGNOP logic_expression", full);
    return full;
}

// ---- logic_expression ----
any ID2205029_Visitor::visitRelationalExpression(ID2205029_CSubsetParser::RelationalExpressionContext *ctx) { 
    string text = any_cast<string>(visit(ctx->rel_expression()));
    logRule(ctx->getStart()->getLine(), "logic_expression : rel_expression", text);
    return text;
}

any ID2205029_Visitor::visitMultipleRelationalExpression(ID2205029_CSubsetParser::MultipleRelationalExpressionContext *ctx) {
    string leftExpr = any_cast<string>(visit(ctx->rel_expression(0)));
    string rightExpr = any_cast<string>(visit(ctx->rel_expression(1)));
    string op = ctx->LOGICOP()->getText();
    string full = leftExpr+ op + rightExpr;
    logRule(ctx->getStart()->getLine(), "logic_expression : rel_expression LOGICOP rel_expression", full);
    return full;    
}

// ---- rel_expression ----
any ID2205029_Visitor::visitSimpleExpression(ID2205029_CSubsetParser::SimpleExpressionContext *ctx) { 
    string text = any_cast<string>(visit(ctx->simple_expression()));
    logRule(ctx->getStart()->getLine(), "rel_expression : simple_expression", text);
    return text;
}

any ID2205029_Visitor::visitSimpleCompareSimple(ID2205029_CSubsetParser::SimpleCompareSimpleContext *ctx) {
    string leftExpr = any_cast<string>(visit(ctx->simple_expression(0)));
    string rightExpr = any_cast<string>(visit(ctx->simple_expression(1)));
    string op = ctx->RELOP()->getText();
    string full = leftExpr + op + rightExpr;
    logRule(ctx->getStart()->getLine(), "rel_expression : simple_expression RELOP simple_expression", full);
    return full;
}

any ID2205029_Visitor::visitSimpleTerm(ID2205029_CSubsetParser::SimpleTermContext *ctx) { 
    string text = any_cast<string>(visit(ctx->term()));
    logRule(ctx->getStart()->getLine(), "simple_expression : term", text);
    return text;
}

any ID2205029_Visitor::visitSimpleExpressionAddTerm(ID2205029_CSubsetParser::SimpleExpressionAddTermContext *ctx) { 
    string leftExpr = any_cast<string>(visit(ctx->simple_expression()));
    string rightTerm = any_cast<string>(visit(ctx->term()));
    string op = ctx->ADDOP()->getText();
    string full = leftExpr + op + rightTerm;
    logRule(ctx->getStart()->getLine(), "simple_expression : simple_expression ADDOP term", full);
    return full;
}

// ---- term ----
any ID2205029_Visitor::visitUnaryExpression(ID2205029_CSubsetParser::UnaryExpressionContext *ctx) { 
    string text = any_cast<string>(visit(ctx->unary_expression())); 
    logRule(ctx->getStart()->getLine(), "term : unary_expression", text);
    return text;
}

any ID2205029_Visitor::visitTermMultipliedUnaryExpression(ID2205029_CSubsetParser::TermMultipliedUnaryExpressionContext *ctx) {
    string leftTerm = any_cast<string>(visit(ctx->term()));
    string rightUnaryExpr = any_cast<string>(visit(ctx->unary_expression()));
    string op = ctx->MULOP()->getText();
    string full = leftTerm + op + rightUnaryExpr;
    logRule(ctx->getStart()->getLine(), "term : term MULOP unary_expression", full);
    return full;
}

// ---- unary_expression ----
any ID2205029_Visitor::visitAddUnaryExpression(ID2205029_CSubsetParser::AddUnaryExpressionContext *ctx) { 
    string unary = any_cast<string>(visit(ctx->unary_expression()));
    string plus = ctx->ADDOP()->getText();
    string text = plus + unary;
    logRule(ctx->getStart()->getLine(), "unary_expression : ADDOP unary_expression", text);
    return text;
}

any ID2205029_Visitor::visitNotUnaryExpression(ID2205029_CSubsetParser::NotUnaryExpressionContext *ctx) { 
    string notnot = ctx->NOT()->getText();
    string unary = any_cast<string>(visit(ctx->unary_expression()));
    string text = notnot + unary;
    logRule(ctx->getStart()->getLine(), "unary_expression : NOT unary_expression", text);
    return text;
}

any ID2205029_Visitor::visitSingleFactor(ID2205029_CSubsetParser::SingleFactorContext *ctx) { 
    string text = any_cast<string>(visit(ctx->factor()));
    logRule(ctx->getStart()->getLine(), "unary_expression : factor", text);
    return text;
}

// ---- factor ----
any ID2205029_Visitor::visitFactorVariable(ID2205029_CSubsetParser::FactorVariableContext *ctx) { 
    string text = any_cast<string>(visit(ctx->variable()));
    logRule(ctx->getStart()->getLine(), "factor : variable", text);
    return text;
}

any ID2205029_Visitor::visitIDOfArgList(ID2205029_CSubsetParser::IDOfArgListContext *ctx) {
    string funcName = ctx->ID()->getText();
    string args = any_cast<string>(visit(ctx->argument_list()));
    string full = funcName + "(" + args + ")";
    logRule(ctx->getStart()->getLine(), "factor : ID LPAREN argument_list RPAREN", full);
    return full;
}

any ID2205029_Visitor::visitBracketedExpression(ID2205029_CSubsetParser::BracketedExpressionContext *ctx) { 
    string expr = any_cast<string>(visit(ctx->expression()));
    string text = "(" + expr + ")";
    logRule(ctx->getStart()->getLine(), "factor : LPAREN expression RPAREN", text);
    return text;
}

any ID2205029_Visitor::visitConstInt(ID2205029_CSubsetParser::ConstIntContext *ctx) { 
    string text = ctx->CONST_INT()->getText();
    logRule(ctx->getStart()->getLine(), "factor : CONST_INT", text);
    return text;
}

any ID2205029_Visitor::visitConstFloat(ID2205029_CSubsetParser::ConstFloatContext *ctx) { 
    float fVal = stof(ctx->CONST_FLOAT()->getText());
    ostringstream stream;
    stream << fixed << setprecision(2) << fVal;
    string text = stream.str();
    logRule(ctx->getStart()->getLine(), "factor : CONST_FLOAT", text);
    return text;
}

any ID2205029_Visitor::visitVariableIncrement(ID2205029_CSubsetParser::VariableIncrementContext *ctx) { 
    string text = any_cast<string>(visit(ctx->variable()));
    string full = text + "++";
    logRule(ctx->getStart()->getLine(), "factor : variable INCOP", full);
    return full;
}

any ID2205029_Visitor::visitVariableDecrement(ID2205029_CSubsetParser::VariableDecrementContext *ctx) { 
    string text = any_cast<string>(visit(ctx->variable()));
    string full = text + "--";
    logRule(ctx->getStart()->getLine(), "factor : variable DECOP", full);
    return full;
}

// ---- argument_list ----
any ID2205029_Visitor::visitNonEmptyArguments(ID2205029_CSubsetParser::NonEmptyArgumentsContext *ctx) { 
    string text = any_cast<string>(visit(ctx->arguments()));
    logRule(ctx->getStart()->getLine(), "argument_list : arguments", text);
    return text;
}

any ID2205029_Visitor::visitBlankArgument(ID2205029_CSubsetParser::BlankArgumentContext *ctx) { 
    logRule(ctx->getStart()->getLine(), "argument_list : ", "");
    return string("");
}

// ---- arguments ----
any ID2205029_Visitor::visitMultipleArguments(ID2205029_CSubsetParser::MultipleArgumentsContext *ctx) { 
    string argList = any_cast<string>(visit(ctx->arguments()));
    string expr = any_cast<string>(visit(ctx->logic_expression()));
    string full = argList + "," + expr;
    logRule(ctx->getStart()->getLine(), "arguments : arguments COMMA logic_expression", full);
    return full;
}

any ID2205029_Visitor::visitSingleArgumentLogic(ID2205029_CSubsetParser::SingleArgumentLogicContext *ctx) { 
    string text = any_cast<string>(visit(ctx->logic_expression()));
    logRule(ctx->getStart()->getLine(), "arguments : logic_expression", text);
    return text;
}