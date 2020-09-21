#include <parser.hpp>
#include <iostream>
#include <unordered_map>

using namespace iodine;

void printIndents(int indents) {
    for (int i = 0; i < indents; i++) {
        std::cout << "  ";
    }
}

void printASTNode(std::shared_ptr<ASTNode> node, int indentDepth = 0) {
    printIndents(indentDepth);
        std::cout << "Node type: " << nodeTypeNames[node->type] << "\n";
    switch (node->type) {
        case ASTNodeType::IntegerVal:
            printIndents(indentDepth);
            std::cout << "integer val: " << std::static_pointer_cast<IntegerValueNode>(node)->val << "\n";
            break;
        case ASTNodeType::Arithmetic:
        {
            auto aNode = std::static_pointer_cast<ArithmeticNode>(node);
            printIndents(indentDepth);
            std::cout << "a: \n";
            printASTNode(aNode->a, indentDepth + 1);

            printIndents(indentDepth);
            std::cout << "b: \n";
            printASTNode(aNode->b, indentDepth + 1);

            printIndents(indentDepth);
            std::cout << "operation: " << arithOperationNames[aNode->operation] << "\n";
            break;
        }
        default:
            break;
    }
}

int evalAST(std::shared_ptr<ASTNode> exprRoot) {
    return std::static_pointer_cast<ProducesIntValueNode>(exprRoot)->getValue();
}

void printTokens(std::vector<Token>& tokens) {
    for (auto& token : tokens) {
        std::cout << "Token: " << tokenNames[token.type];
        std::cout << " (" << token.val << ")";
        std::cout << "\n";
    }
}

int main(int argc, char** argv) {
    while (true) {
        std::string line;

        std::cout << ">";

        std::getline(std::cin, line);

        try {
            std::vector<Token> tokens = parseTokens(line);

            std::shared_ptr<ASTNode> n = parseScript(tokens);

            if (n == nullptr) {
                std::cout << "AST is empty\n";
            } else {
                printTokens(tokens);
                printASTNode(n);
                std::cout << evalAST(n) << "\n";
            }
        } catch (std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    } 
    return 0;
}
