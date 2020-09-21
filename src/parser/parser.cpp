#include "parser.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cassert>

namespace iodine {
    class Parser {
    private:
        std::shared_ptr<ArithmeticNode> makeArithmeticNode(Token& opToken, std::shared_ptr<ProducesValueNode> a, std::shared_ptr<ProducesValueNode> b) {
            assert(opToken.type == TokenType::Operator);

            auto node = std::make_shared<ArithmeticNode>();

            if (opToken.val == "-")
                node->operation = ArithmeticOperation::Subtract;
            else if (opToken.val == "+")
                node->operation = ArithmeticOperation::Add;
            else if (opToken.val == "*")
                node->operation = ArithmeticOperation::Multiply;
            else if (opToken.val == "/")
                node->operation = ArithmeticOperation::Divide;

            node->a = a;
            node->b = b;
            return node;
        }

        std::shared_ptr<ConstValNode> tokenToVal(const Token& token) {
            if (token.type == TokenType::DecimalNumber) {
                return std::make_shared<ConstValNode>((float)std::atof(token.val.c_str()));
            } else if (token.type == TokenType::Number) {
                return std::make_shared<ConstValNode>(token.numberVal);
            } else {
                std::cerr << "Warning: Can't deduce value type of token " << tokenNames[token.type] << "\n";
                return nullptr;
            }
        }
    public:
        Parser() {
        }

        std::shared_ptr<ASTNode> parseExpression(std::vector<Token>::iterator begin, std::vector<Token>::iterator end) {
            std::shared_ptr<ASTNode> currNode = nullptr;
            for (auto tokenIt = begin; tokenIt < end; tokenIt++) {
                if (tokenIt->type == TokenType::Number || tokenIt->type == TokenType::DecimalNumber) {
                    // Tokens that can follow this:
                    // Operator

                    if (tokenIt == end - 1) {
                        return tokenToVal(*tokenIt);
                    }

                    auto valToken = tokenIt;

                    tokenIt++;
                    if (tokenIt->type != TokenType::Operator) throw std::runtime_error(std::string("Expected operator, found ") + tokenNames[tokenIt->type]);

                    // After this, there are actually a few different tokens.
                    // This is a whole other expression. Recurse!
                    auto subExpr = parseExpression(tokenIt + 1, end);

                    auto a = tokenToVal(*valToken);
                    auto b = std::static_pointer_cast<ProducesValueNode>(subExpr);
                    if (b == nullptr) throw std::runtime_error("Expected expression");

                    return makeArithmeticNode(*tokenIt, a, b);
                } else if (tokenIt->type == TokenType::OpenParenthesis) {
                    // search forward for close parenthesis

                    std::vector<Token>::iterator closeParenthesisPos = end;
                    for (auto it2 = tokenIt; it2 < end; it2++) {
                        if (it2->type == TokenType::CloseParenthesis) {
                            closeParenthesisPos = it2;
                        }
                    } 

                    if (closeParenthesisPos == end)
                        throw std::runtime_error("Couldn't find close parenthesis");

                    auto parenthesisNode = parseExpression(tokenIt + 1, closeParenthesisPos);
                    if (parenthesisNode == nullptr) throw std::runtime_error("Expected expression in parenthesis");

                    if (closeParenthesisPos == end - 1)
                        return parenthesisNode;

                    // look for an operator
                    tokenIt = closeParenthesisPos + 1;

                    assert(tokenIt->type == TokenType::Operator);

                    auto b = parseExpression(tokenIt + 1, end);

                    if (b == nullptr) throw std::runtime_error("Expected expression");

                    return makeArithmeticNode(*tokenIt, std::static_pointer_cast<ProducesValueNode>(parenthesisNode), std::static_pointer_cast<ProducesValueNode>(b));
                } else if (tokenIt->type == TokenType::Operator) {
                    if (tokenIt->val == "+") {
                        // Unary plus

                    } else if (tokenIt->val == "-") {

                    }
                } else {
                    throw std::runtime_error("Unexpected token " + tokenIt->val + " (" + tokenNames[tokenIt->type] + ")");
                }
            }

            return nullptr;
        }
    };

    std::shared_ptr<ASTNode> parseScript(std::vector<Token> tokens) {
        return Parser().parseExpression(tokens.begin(), tokens.end());
    }
}
