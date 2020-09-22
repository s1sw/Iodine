#include "parser.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <unordered_map>

namespace iodine {
    const std::unordered_map<std::string, DataType> builtinTypes = {
        { "f64", DataType::F64 },
        { "f32", DataType::F32 },
        { "i32", DataType::Int32 }
    };

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

        void safeAdvance(std::vector<Token>::iterator& it, const std::vector<Token>::iterator& end) {
            it++;
            if (it >= end)
                throw std::runtime_error("Unexpected end of token stream");
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
                    // Bit of a problem with unary ops
                    // If we do something like -5 + 2, we're expecting it to parse as (-5) + (2) = -3
                    // However, Iodine currently parses it as -(5 + 2) = -7 which is just wrong!
                    // I'm not entirely sure how to fix this...
                    // Order of operations seems to be a significant problem with how the parser is designed right now.
                    auto unaryOpNode = std::make_shared<UnaryOpNode>();
                    if (tokenIt->val == "+") {
                        // Unary plus
                        unaryOpNode->operation = UnaryOperation::Plus;
                    } else if (tokenIt->val == "-") {
                        unaryOpNode->operation = UnaryOperation::Minus;
                    }

                    auto nextNode = std::static_pointer_cast<ProducesValueNode>(parseExpression(tokenIt + 1, end));
                    // Special case: if the next node is an arithmetic node, swap the leftmost value for our unary op.
                    if (nextNode->type == ASTNodeType::Arithmetic) {
                        auto aNode = std::static_pointer_cast<ArithmeticNode>(nextNode);

                        auto tmp = aNode->a;
                        aNode->a = unaryOpNode;
                        unaryOpNode->valNode = tmp;
                            return aNode;
                    } else {
                        unaryOpNode->valNode = nextNode;
                    }
                    return unaryOpNode;
                } else if(tokenIt->type == TokenType::Name) {
                    // Either a type name or a variable name
                    auto typeIt = builtinTypes.find(tokenIt->val);

                    if (typeIt == builtinTypes.end()) {
                        // Cross our fingers and hope it's a variable reference
                        auto varRef = std::make_shared<VariableReferenceNode>();
                        varRef->varName = tokenIt->val;

                        // is this is the end?
                        if (tokenIt == (end - 1))
                            return varRef;

                        auto valToken = tokenIt;

                        tokenIt++;
                        if (tokenIt->type != TokenType::Operator) throw std::runtime_error(std::string("Expected operator, found ") + tokenNames[tokenIt->type]);

                        // After this, there are actually a few different tokens.
                        // This is a whole other expression. Recurse!
                        auto subExpr = parseExpression(tokenIt + 1, end);

                        auto a = varRef;
                        auto b = std::static_pointer_cast<ProducesValueNode>(subExpr);
                        if (b == nullptr) throw std::runtime_error("Expected expression");

                        return makeArithmeticNode(*tokenIt, a, b);
                    }

                    // Next token should be another name (the variable name).
                    safeAdvance(tokenIt, end);

                    if (tokenIt->type != TokenType::Name) {
                        throw std::runtime_error("Expected variable name");
                    }

                    std::string name = tokenIt->val;

                    // Next is an equals sign...
                    safeAdvance(tokenIt, end);

                    if (tokenIt->type != TokenType::Equals) {
                        throw std::runtime_error("Expected equals symbol");
                    }
                    
                    auto expr = parseExpression(tokenIt + 1, end);

                    if (expr == nullptr) throw std::runtime_error("Expected expression after equals");

                    auto varAssignment = std::make_shared<VarAssignmentNode>();
                    varAssignment->varName = name;
                    varAssignment->valNode = std::static_pointer_cast<ProducesValueNode>(expr);

                    return varAssignment;
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
