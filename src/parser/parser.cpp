#include "parser.hpp"
#include <stdexcept>
#include <cassert>

namespace iodine {
    class Parser {
    public:
        Parser(std::vector<Token>& tokens) : tokens(tokens) {
            currentToken = tokens.begin();
        }
        std::vector<Token>& tokens;
        std::vector<Token>::iterator currentToken;

        std::shared_ptr<ASTNode> parseExpression(std::vector<Token>::iterator begin, std::vector<Token>::iterator end) {
            // So, first we're just going to try parsing a basic addition.
            // This will be a simple set of tokens:
            // 1. A number
            // 2. An operation (+, -, *, *)
            // 3. Another number

            if (begin == end - 1) {
                if (begin->type == TokenType::Number) {
                    return std::make_shared<IntegerValueNode>(begin->numberVal);
                }
            }

            // Assume this is an expression
            for (auto tokenIt = begin; tokenIt < end; tokenIt++) {
                if (tokenIt->type == TokenType::Number) {
                    // Tokens that can follow this:
                    // Operator

                    if (tokenIt >= tokens.end() - 1) return std::make_shared<IntegerValueNode>(tokenIt->numberVal);

                    int numVal = tokenIt->numberVal;

                    tokenIt++;
                    if (tokenIt->type != TokenType::Operator) throw std::runtime_error("Expected operator");

                    // After this, there are actually a few different tokens.
                    // This is a whole other expression. Recurse!
                    auto subExpr = parseExpression(tokenIt + 1, tokens.end());
                    
                    auto aNode = std::make_shared<ArithmeticNode>();

                    if (tokenIt->val == "-")
                        aNode->operation = ArithmeticOperation::Subtract;
                    else if (tokenIt->val == "+")
                        aNode->operation = ArithmeticOperation::Add;
                    else if (tokenIt->val == "*")
                        aNode->operation = ArithmeticOperation::Multiply;
                    else if (tokenIt->val == "/")
                        aNode->operation = ArithmeticOperation::Divide;


                    aNode->a = std::make_shared<IntegerValueNode>(numVal);
                    aNode->b = std::static_pointer_cast<ProducesIntValueNode>(subExpr);
                    return aNode;
                }
            }

            return nullptr;
        }
    };

    std::shared_ptr<ASTNode> parseScript(std::vector<Token> tokens) {
        return Parser(tokens).parseExpression(tokens.begin(), tokens.end());
    }
}
