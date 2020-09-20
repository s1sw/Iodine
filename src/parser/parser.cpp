#include "parser.hpp"
#include <stdexcept>

namespace iodine {
    ASTNode* parseScript(std::vector<Token> tokens) {
        // So, first we're just going to try parsing a basic addition.
        // This will be a simple set of tokens:
        // 1. A number
        // 2. An operation (+, -, *, *)
        // 3. Another number
        
        if (tokens[0].type != TokenType::Number) throw std::runtime_error("Expected number");
        if (tokens[1].type != TokenType::Operator) throw std::runtime_error("Expected operator");
        if (tokens[2].type != TokenType::Number) throw std::runtime_error("Expected number");

        ArithmeticNode* aNode = new ArithmeticNode;
        aNode->operation = ArithmeticOperation::Add;  

        aNode->a = new IntegerValueNode(tokens[0].numberVal);
        aNode->b = new IntegerValueNode(tokens[2].numberVal);

        return aNode;
    }
}
