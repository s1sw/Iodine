#include "parser.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
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
            } else if (token.type == TokenType::True) {
                return std::make_shared<ConstValNode>(true);
            } else if (token.type == TokenType::False) {
                return std::make_shared<ConstValNode>(false);
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

        std::shared_ptr<ASTNode> parsePartialExpression(std::shared_ptr<ProducesValueNode> val, std::vector<Token>::iterator begin, std::vector<Token>::iterator end) {
            auto tokenIt = begin;
            if (tokenIt->type == TokenType::Semicolon) return val;

            if (tokenIt->type == TokenType::Equals) {
                safeAdvance(tokenIt, end);
                expect(tokenIt, TokenType::Equals);

                auto comp = std::make_shared<ComparisonNode>();
                comp->compType = ComparisonType::Equal;
                comp->lhs = val;
                safeAdvance(tokenIt, end);
                comp->rhs = std::static_pointer_cast<ProducesValueNode>(parseExpression(tokenIt, end));
                return comp;
            }

            if (tokenIt->type != TokenType::Operator)
                throw std::runtime_error(std::string("Expected operator, found ") + tokenNames[tokenIt->type]);

            // After this, there are actually a few different tokens.
            // This is a whole other expression. Recurse!
            auto subExpr = parseExpression(tokenIt + 1, end);

            auto a = val;
            auto b = std::static_pointer_cast<ProducesValueNode>(subExpr);
            if (b == nullptr)
                throw std::runtime_error("Expected expression");

            return makeArithmeticNode(*tokenIt, a, b);
        }

        using TokenIter = std::vector<Token>::iterator;

        void expect(TokenIter it, TokenType type) {
            if (it->type != type) {
                throw std::runtime_error(std::string("Expected ") + tokenNames[type] + ", got " + tokenNames[it->type]);
            }
        }

        TokenIter find(TokenIter begin, TokenIter end, TokenType type) {
            for (auto i = begin; i < end; i++) {
                if (i->type == type)
                    return i;
            }

            return end;
        }

        TokenIter findAssured(TokenIter begin, TokenIter end, TokenType type) {
            auto ret = find(begin, end, type);

            if (ret == end)
                throw std::runtime_error(std::string("Could not find ") + tokenNames[type]);

            return ret;
        }

        TokenIter findMultiple(TokenIter begin, TokenIter end, TokenType a, TokenType b) {
            for (auto i = begin; i < end; i++) {
                if (i->type == a || i ->type == b)
                    return i;
            }

            return end;
        }
    public:
        Parser() {
        }

        std::shared_ptr<ASTNode> parseExpression(std::vector<Token>::iterator begin, std::vector<Token>::iterator end) {
            std::shared_ptr<ASTNode> currNode = nullptr;
            for (auto tokenIt = begin; tokenIt < end; tokenIt++) {
                if (tokenIt->type == TokenType::Semicolon || tokenIt->type == TokenType::NewLine) continue;
                if (tokenIt->type == TokenType::StringContents) {
                    return std::make_shared<ConstValNode>(Value{strdup(tokenIt->val.c_str())}); 
                } else if (tokenIt->type == TokenType::Number || tokenIt->type == TokenType::DecimalNumber || tokenIt->type == TokenType::True || tokenIt->type == TokenType::False) {
                    // Tokens that can follow this:
                    // Operator

                    if (tokenIt == end - 1) {
                        return tokenToVal(*tokenIt);
                    }

                    auto val = tokenToVal(*tokenIt);

                    tokenIt++;
                    return parsePartialExpression(val, tokenIt, end);
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
                    if (parenthesisNode == nullptr)
                        throw std::runtime_error("Expected expression in parenthesis");

                    if (closeParenthesisPos == end - 1)
                        return parenthesisNode;

                    // look for an operator
                    tokenIt = closeParenthesisPos + 1;

                    assert(tokenIt->type == TokenType::Operator);

                    auto b = parseExpression(tokenIt + 1, end);

                    if (b == nullptr)
                        throw std::runtime_error("Expected expression");

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
                } else if (tokenIt->type == TokenType::Name) {
                    // Either a type name or a variable name
                    auto typeIt = builtinTypes.find(tokenIt->val);

                    if (typeIt == builtinTypes.end()) {
                        // is this the end?
                        if (tokenIt == (end - 1)) {
                            auto varRef = std::make_shared<VariableReferenceNode>();
                            varRef->varName = tokenIt->val;
                            return varRef;
                        }

                        // nope!
                        // check the next token
                        // if it's an open parenthesis we know this is a function call

                        if ((tokenIt + 1)->type == TokenType::OpenParenthesis) {
                            auto openParenToken = tokenIt + 1;

                            // find close parenthesis
                            auto closeParenToken = end;
                            for (auto it = openParenToken; it < end; it++) {
                                if (it->type == TokenType::CloseParenthesis)
                                    closeParenToken = it;
                            }

                            if (closeParenToken == end)
                                throw std::runtime_error("Could not find close parenthesis for function call");

                            // function call!
                            auto fCall = std::make_shared<FunctionCallNode>();

                            fCall->functionName = tokenIt->val; 
                            auto currentArgStart = openParenToken + 1;
                            for (auto it = openParenToken +1; it < closeParenToken; it++) {
                                if (it->type == TokenType::Comma) {
                                    fCall->args.emplace_back(std::static_pointer_cast<ProducesValueNode>(parseExpression(currentArgStart, it))); 
                                    currentArgStart = it + 1;
                                }
                            }

                            if (std::distance(openParenToken, closeParenToken) > 1) {
                                fCall->args.emplace_back(std::static_pointer_cast<ProducesValueNode>(parseExpression(openParenToken + 1, closeParenToken)));
                            }
                            
                            return fCall;
                        }

                        // Cross our fingers and hope it's a variable reference
                        std::string name = tokenIt->val;

                        tokenIt++;
                        if (tokenIt->type == TokenType::Equals) {
                            auto varAssign = std::make_shared<VarAssignmentNode>();
                            varAssign->varName = name;
                            varAssign->createNew = false;
                            varAssign->valNode = std::static_pointer_cast<ProducesValueNode>(parseExpression(tokenIt + 1, end));
                            return varAssign;
                        }

                        auto varRef = std::make_shared<VariableReferenceNode>();
                        varRef->varName = name;

                        return parsePartialExpression(varRef, tokenIt, end);
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

                    if (expr == nullptr)
                        throw std::runtime_error("Expected expression after equals");

                    auto varAssignment = std::make_shared<VarAssignmentNode>();
                    varAssignment->varName = name;
                    varAssignment->createNew = true;
                    varAssignment->type = typeIt->second;
                    varAssignment->valNode = std::static_pointer_cast<ProducesValueNode>(expr);

                    return varAssignment;
                } else if (tokenIt->type == TokenType::If) {
                    // Condition in parenthesis followed by further statements
                    // Get expression in parenthesis
                    safeAdvance(tokenIt, end);
                    expect(tokenIt, TokenType::OpenParenthesis);
                    safeAdvance(tokenIt, end);

                    auto closeParen = findAssured(tokenIt, end, TokenType::CloseParenthesis);

                    auto expr = parseExpression(tokenIt, closeParen);

                    if (expr == nullptr)
                        throw std::runtime_error("Expected expression");

                    auto ifNode = std::make_shared<IfNode>();
                    ifNode->condition = std::static_pointer_cast<ProducesValueNode>(expr);

                    tokenIt = closeParen;
                    safeAdvance(tokenIt, end);
                    expect(tokenIt, TokenType::OpenBrace);
                    safeAdvance(tokenIt, end);

                    auto cIt = tokenIt;

                    while (true) {
                        // get the next statement to the semicolon
                        auto nextSemicolon = find(cIt, end, TokenType::Semicolon);
                        auto expr = parseExpression(cIt, nextSemicolon);

                        if (expr == nullptr)
                            break;

                        ifNode->nodes.push_back(parseExpression(cIt, nextSemicolon));
                        cIt = nextSemicolon;
                    }

                    return ifNode;
                } else {
                    throw std::runtime_error("Unexpected token " + tokenIt->val + " (" + tokenNames[tokenIt->type] + ")");
                }
            }

            return nullptr;
        }

        std::vector<std::shared_ptr<ASTNode>> parseScript(std::vector<Token> tokens) {
            std::vector<std::shared_ptr<ASTNode>> nodes;
            auto cIt = tokens.begin();
            while (cIt < tokens.end()) {
                auto next = find(cIt, tokens.end(), TokenType::Semicolon);
                auto expr = parseExpression(cIt, next);
                if (expr == nullptr)
                    break;

                nodes.push_back(expr);
                cIt = next + 1;
            }
            
            return nodes;
        }
    };

    std::shared_ptr<ASTNode> parseExpression(std::vector<Token> tokens) {
        return Parser().parseExpression(tokens.begin(), tokens.end());
    }

    std::vector<std::shared_ptr<ASTNode>> parseScript(std::vector<Token> tokens) {
        return Parser().parseScript(tokens);
    }
}
