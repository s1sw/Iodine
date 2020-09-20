#pragma once
#include <vector>
#include <string>
#include <memory>

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
            virtual void doNothing() {}
    };

    class ScriptNode : public ASTNode {
        public:
            ScriptNode() : ASTNode(ASTNodeType::Script) {}
            std::vector<ASTNode*> children;
    };

    class ConstValNode {

    };

    class ProducesIntValueNode : public ASTNode {
    protected:
        ProducesIntValueNode(ASTNodeType nodeType) : ASTNode(nodeType) {}
    public:
        virtual int getValue() = 0;
    };

    class IntegerValueNode : public ProducesIntValueNode {
        public:
            IntegerValueNode(int val = 0) : ProducesIntValueNode(ASTNodeType::IntegerVal), val(val) {}
            int val;
            int getValue() override { return val; }
    };

    enum class ArithmeticOperation {
        Add,
        Subtract,
        Divide,
        Multiply
    };

    class ArithmeticNode : public ProducesIntValueNode {
        public:
            ArithmeticNode() : ProducesIntValueNode(ASTNodeType::Arithmetic) {}
            std::shared_ptr<ProducesIntValueNode> a;
            std::shared_ptr<ProducesIntValueNode> b;
            ArithmeticOperation operation;

            int calculateValue() {
                switch (operation) {
                case ArithmeticOperation::Add:
                    return a->getValue() + b->getValue();
                case ArithmeticOperation::Subtract:
                    return a->getValue() - b->getValue();
                case ArithmeticOperation::Multiply:
                    return a->getValue() * b->getValue();
                case ArithmeticOperation::Divide:
                    return a->getValue() / b->getValue();
                }
            }

            int getValue() override {
                return calculateValue();
            }
    };

    class VarAssignmentNode : public ASTNode {
        public:
            ASTNode* valueNode;
    };

    std::vector<Token> parseTokens(std::string str); 
    std::shared_ptr<ASTNode> parseScript(std::vector<Token> tokens);
}
