#include <parser.hpp>
#include <iostream>

using namespace iodine;

void printIndents(int indents) {
    for (int i = 0; i < indents; i++) {
        std::cout << "  ";
    }
}

void printASTNode(ASTNode* node, int indentDepth = 0) {
    printIndents(indentDepth);
    std::cout << "Node type: " << (int)node->type << "\n";
    switch (node->type) {
        case ASTNodeType::IntegerVal:
            printIndents(indentDepth);
            std::cout << "integer val: " << ((IntegerValueNode*)node)->val << "\n";
            break;
        case ASTNodeType::Arithmetic:
            printIndents(indentDepth);
            std::cout << "a: ";
            printASTNode(((ArithmeticNode*)node)->a, indentDepth + 1);

            printIndents(indentDepth);
            std::cout << "b: ";
            printASTNode(((ArithmeticNode*)node)->b, indentDepth + 1);
            break;
        default:
            break;
    }
}

void evalArithmetic(ArithmeticNode* aNode) {
    IntegerValueNode* aIVal = (IntegerValueNode*)aNode->a;
    IntegerValueNode* bIVal = (IntegerValueNode*)aNode->b;
    switch (aNode->operation) {
        case ArithmeticOperation::Add:
            std::cout << aIVal->val + bIVal->val << "\n"; 
            break;
    }
}

void evalAST(ASTNode* exprRoot) {
    switch (exprRoot->type) {
        case ASTNodeType::Arithmetic:
            evalArithmetic((ArithmeticNode*)exprRoot);
            break;
    }
}

int main(int argc, char** argv) {
    while (true) {
        std::string line;

        std::cout << ">";

        std::getline(std::cin, line);

        std::vector<Token> tokens = parseTokens(line);

        for (auto& token : tokens) {
            std::cout << "Token: " << (int)token.type;
                std::cout << " (" << token.val << ")";
            std::cout << "\n";
        }

        ASTNode* n = parseScript(tokens);

        printASTNode(n);
        evalAST(n);
    } 
    return 0;
}
