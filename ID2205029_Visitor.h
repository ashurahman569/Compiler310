#pragma once
#include <fstream>
#include <string>
#include <any>

#include "ID2205029_CSubsetBaseVisitor.h"
#include "ID2205029_CSubsetParser.h"
#include "ID2205029_SymbolTable.cpp"

using namespace antlr4;
using namespace std;

class ID2205029_Visitor : public ID2205029_CSubsetBaseVisitor {
public:
    ID2205029_Visitor(const string &logPath, const string &errorPath);
    ~ID2205029_Visitor();

    any visitStart(ID2205029_CSubsetParser::StartContext *ctx) override;

    any visitProgramUnit(ID2205029_CSubsetParser::ProgramUnitContext *ctx) override;
    any visitUnitOnly(ID2205029_CSubsetParser::UnitOnlyContext *ctx) override;

    any visitUnitVarDec(ID2205029_CSubsetParser::UnitVarDecContext *ctx) override;
    any visitFuncDec(ID2205029_CSubsetParser::FuncDecContext *ctx) override;
    any visitFuncDef(ID2205029_CSubsetParser::FuncDefContext *ctx) override;

    any visitFuncDecWithParam(ID2205029_CSubsetParser::FuncDecWithParamContext *ctx) override;
    any visitFuncDecNoParam(ID2205029_CSubsetParser::FuncDecNoParamContext *ctx) override;

    any visitFuncDefWithParam(ID2205029_CSubsetParser::FuncDefWithParamContext *ctx) override;
    any visitFuncDefNoParam(ID2205029_CSubsetParser::FuncDefNoParamContext *ctx) override;

    any visitParamListMultipleID(ID2205029_CSubsetParser::ParamListMultipleIDContext *ctx) override;
    any visitParamListMultipleWithNoIDLast(ID2205029_CSubsetParser::ParamListMultipleWithNoIDLastContext *ctx) override;
    any visitParamListSingleID(ID2205029_CSubsetParser::ParamListSingleIDContext *ctx) override;
    any visitParamListNoID(ID2205029_CSubsetParser::ParamListNoIDContext *ctx) override;
    any visitParamListGarbage(ID2205029_CSubsetParser::ParamListGarbageContext *ctx) override;
    any visitParamListMultipleGarbage(ID2205029_CSubsetParser::ParamListMultipleGarbageContext *ctx) override;

    any visitCodeBlock(ID2205029_CSubsetParser::CodeBlockContext *ctx) override;
    any visitBlankBlock(ID2205029_CSubsetParser::BlankBlockContext *ctx) override;

    // ---- var_declaration ----
    any visitTypeSpecifiedVarDeclarationList(ID2205029_CSubsetParser::TypeSpecifiedVarDeclarationListContext *ctx) override;

    any visitTypeInt(ID2205029_CSubsetParser::TypeIntContext *ctx) override;
    any visitTypeFloat(ID2205029_CSubsetParser::TypeFloatContext *ctx) override;
    any visitTypeVoid(ID2205029_CSubsetParser::TypeVoidContext *ctx) override;

    any visitMultipleIDDeclaration(ID2205029_CSubsetParser::MultipleIDDeclarationContext *ctx) override;
    any visitMutlipleIDDeclarationWithArray(ID2205029_CSubsetParser::MutlipleIDDeclarationWithArrayContext *ctx) override;
    any visitSingleIDDeclaration(ID2205029_CSubsetParser::SingleIDDeclarationContext *ctx) override;
    any visitSignleIDArrayDeclaration(ID2205029_CSubsetParser::SignleIDArrayDeclarationContext *ctx) override;
    any visitDeclListGarbage(ID2205029_CSubsetParser::DeclListGarbageContext *ctx) override;
    any visitMultipleDeclListGarbage(ID2205029_CSubsetParser::MultipleDeclListGarbageContext *ctx) override;
    
    any visitSingleStatement(ID2205029_CSubsetParser::SingleStatementContext *ctx) override;
    any visitMultipleStatement(ID2205029_CSubsetParser::MultipleStatementContext *ctx) override;

    any visitStatementVarDec(ID2205029_CSubsetParser::StatementVarDecContext *ctx) override;
    any visitSingleExpressionStatement(ID2205029_CSubsetParser::SingleExpressionStatementContext *ctx) override;
    any visitCompoundStatement(ID2205029_CSubsetParser::CompoundStatementContext *ctx) override;
    any visitForLoopStatement(ID2205029_CSubsetParser::ForLoopStatementContext *ctx) override;
    any visitIfStatement(ID2205029_CSubsetParser::IfStatementContext *ctx) override;
    any visitIfElseStatement(ID2205029_CSubsetParser::IfElseStatementContext *ctx) override;
    any visitWhileLoopStatement(ID2205029_CSubsetParser::WhileLoopStatementContext *ctx) override;
    any visitPrintLineStatement(ID2205029_CSubsetParser::PrintLineStatementContext *ctx) override;
    any visitReturnExpressionStatement(ID2205029_CSubsetParser::ReturnExpressionStatementContext *ctx) override;

    any visitNoExpression(ID2205029_CSubsetParser::NoExpressionContext *ctx) override;
    any visitExpressionStatement(ID2205029_CSubsetParser::ExpressionStatementContext *ctx) override;

    any visitAnID(ID2205029_CSubsetParser::AnIDContext *ctx) override;
    any visitAnArrayIndex(ID2205029_CSubsetParser::AnArrayIndexContext *ctx) override;

    any visitLogicalExpression(ID2205029_CSubsetParser::LogicalExpressionContext *ctx) override;
    any visitAssignExpression(ID2205029_CSubsetParser::AssignExpressionContext *ctx) override;

    any visitRelationalExpression(ID2205029_CSubsetParser::RelationalExpressionContext *ctx) override;
    any visitMultipleRelationalExpression(ID2205029_CSubsetParser::MultipleRelationalExpressionContext *ctx) override;

    any visitSimpleExpression(ID2205029_CSubsetParser::SimpleExpressionContext *ctx) override;
    any visitSimpleCompareSimple(ID2205029_CSubsetParser::SimpleCompareSimpleContext *ctx) override;

    any visitSimpleTerm(ID2205029_CSubsetParser::SimpleTermContext *ctx) override;
    any visitSimpleExpressionAddTerm(ID2205029_CSubsetParser::SimpleExpressionAddTermContext *ctx) override;
    any visitSimpleAddInvalidTerm(ID2205029_CSubsetParser::SimpleAddInvalidTermContext *ctx) override;

    any visitUnaryExpression(ID2205029_CSubsetParser::UnaryExpressionContext *ctx) override;
    any visitTermMultipliedUnaryExpression(ID2205029_CSubsetParser::TermMultipliedUnaryExpressionContext *ctx) override;

    any visitAddUnaryExpression(ID2205029_CSubsetParser::AddUnaryExpressionContext *ctx) override;
    any visitNotUnaryExpression(ID2205029_CSubsetParser::NotUnaryExpressionContext *ctx) override;
    any visitSingleFactor(ID2205029_CSubsetParser::SingleFactorContext *ctx) override;

    any visitFactorVariable(ID2205029_CSubsetParser::FactorVariableContext *ctx) override;
    any visitIDOfArgList(ID2205029_CSubsetParser::IDOfArgListContext *ctx) override;
    any visitBracketedExpression(ID2205029_CSubsetParser::BracketedExpressionContext *ctx) override;
    any visitConstInt(ID2205029_CSubsetParser::ConstIntContext *ctx) override;
    any visitConstFloat(ID2205029_CSubsetParser::ConstFloatContext *ctx) override;
    any visitVariableIncrement(ID2205029_CSubsetParser::VariableIncrementContext *ctx) override;
    any visitVariableDecrement(ID2205029_CSubsetParser::VariableDecrementContext *ctx) override;

    any visitNonEmptyArguments(ID2205029_CSubsetParser::NonEmptyArgumentsContext *ctx) override;
    any visitBlankArgument(ID2205029_CSubsetParser::BlankArgumentContext *ctx) override;

    any visitMultipleArguments(ID2205029_CSubsetParser::MultipleArgumentsContext *ctx) override;
    any visitSingleArgumentLogic(ID2205029_CSubsetParser::SingleArgumentLogicContext *ctx) override;
    any visitInvalid(ID2205029_CSubsetParser::InvalidContext *ctx) override;
    int getErrorCount() const { return errorCount; }
    int getLineCount() const { return lineCount; }

private:
    ofstream logFile;
    ofstream errorFile;
    int errorCount = 0;
    int lineCount = 0;
    SymbolTable symbolTable;
    string currentType = "";
    vector<pair<string, string>> pendingParams; // name, type
    bool isFunctionDef= false;

    bool isInt(const string& str);
    bool isFloat(const string& str);
    bool isArrayMiss(const string& str);
    void logRule(int line, const string &ruleLabel, const string &matchedText);
    void logError(int line, const string &message);
    void logLine();
    string getFullText(tree::ParseTree *tree);
};
