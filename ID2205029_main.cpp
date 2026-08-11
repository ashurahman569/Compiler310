#include <iostream>
#include <fstream>
#include "antlr4-runtime.h"
#include "ID2205029_CSubsetLexer.h"
#include "ID2205029_CSubsetParser.h"
#include "ID2205029_Visitor.h"
using namespace antlr4;
using namespace std;
using namespace tree;

ofstream lexLogFile;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    if (!inputFile) {
        cerr << "Error opening input file " << argv[1] << endl;
        return 1;
    }

    ANTLRInputStream input(inputFile);
    ID2205029_CSubsetLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    ID2205029_CSubsetParser parser(&tokens);
    parser.removeErrorListeners(); 
    ParseTree *tree = parser.start();

    ID2205029_Visitor visitor("log.txt", "error.txt");
    
    visitor.visit(tree);

    return 0;
}