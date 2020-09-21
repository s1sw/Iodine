#pragma once
#include <initializer_list>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

namespace iodine {
    enum class DataType {
        Integer,
        Float
    };

    enum class TokenType {
        Name,
        Semicolon,
        OpenParenthesis,
        CloseParenthesis,
        Comma,
        DoubleQuote,
        Number,
        DecimalNumber,
        Operator,
        Count
    };

    enum class ASTNodeType {
        Script,
        IntegerVal,
        Arithmetic,
        Expression,
        VarAssignment,
        Count
    };

    struct Variable {
        std::string name;
        DataType type;
    };

    template<typename T>
    class EnumNames {
    public:
        EnumNames(const char* names[(int)T::Count]) : names(names) {}
        EnumNames(std::initializer_list<const char*> iList) {
            int i = 0;
            for (auto& val : iList) {
                names[i] = val;
                i++;
            }
        }

        const char* operator[](T t) {
            return names[(int)t];
        }
    private:
        const char* names[(int)T::Count];
    };

    extern EnumNames<TokenType> tokenNames;
    extern EnumNames<ASTNodeType> nodeTypeNames;

    struct Token {
        TokenType type;
        std::string val;
        int numberVal;
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
        Multiply,
        Count
    };

    extern EnumNames<ArithmeticOperation> arithOperationNames;

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
                default:
                    return 0;
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
