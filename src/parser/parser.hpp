#pragma once
#include <vector>
#include <string>

namespace iodine {
    enum class TokenType {
        Name,
        Semicolon,
        OpenParenthesis,
        CloseParenthesis,
        Comma,
        DoubleQuote,
        Number,
        Operator
    };

    struct Token {
        TokenType type;
        std::string val;
        int numberVal;
    };

    enum class ASTNodeType {
        Script,
        IntegerVal,
        Arithmetic,
        Expression,
        VarAssignment
    };

    class ASTNode {
        protected:
            ASTNode(ASTNodeType type) : type(type) {}
        public:
            ASTNodeType type;
    };

    class ScriptNode : public ASTNode {
        public:
            ScriptNode() : ASTNode(ASTNodeType::Script) {}
            std::vector<ASTNode*> children;
    };

    class IntegerValueNode : public ASTNode {
        public:
            IntegerValueNode(int val = 0) : ASTNode(ASTNodeType::IntegerVal), val(val) {}
            int val;
    };

    enum class ArithmeticOperation {
        Add,
        Subtract,
        Divide,
        Multiply
    };

    class ArithmeticNode : public ASTNode {
        public:
            ArithmeticNode() : ASTNode(ASTNodeType::Arithmetic) {}
            ASTNode* a;
            ASTNode* b;
            ArithmeticOperation operation;
    };

    class VarAssignmentNode : public ASTNode {
        public:
            ASTNode* valueNode;
    };

    std::vector<Token> parseTokens(std::string str); 
    ASTNode* parseScript(std::vector<Token> tokens);
}
