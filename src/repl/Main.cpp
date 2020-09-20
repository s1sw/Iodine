#include <parser.hpp>
#include <iostream>
#include <unordered_map>

using namespace iodine;

void printIndents(int indents) {
    for (int i = 0; i < indents; i++) {
        std::cout << "  ";
    }
}

const std::unordered_map<ASTNodeType, std::string> typeNames = {
    { ASTNodeType::IntegerVal, "IntegerVal" },
    { ASTNodeType::Arithmetic, "Arithmetic" }
};

void printASTNode(std::shared_ptr<ASTNode> node, int indentDepth = 0) {
    printIndents(indentDepth);
    std::cout << "Node type: " << typeNames.at(node->type) << "\n";
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
            break;
        }
        default:
            break;
    }
}

int evalAST(std::shared_ptr<ASTNode> exprRoot) {
    return std::static_pointer_cast<ProducesIntValueNode>(exprRoot)->getValue();
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

        std::shared_ptr<ASTNode> n = parseScript(tokens);

        printASTNode(n);
        std::cout << "FINAL VALUE: " << evalAST(n) << "\n";
    } 
    return 0;
}
