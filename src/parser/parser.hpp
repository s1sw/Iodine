#pragma once
#include <initializer_list>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace iodine {
    enum class DataType {
        Int32,
        F32,
        F64
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
        ConstVal,
        Arithmetic,
        Expression,
        VarAssignment,
        Count
    };

    inline bool isDecimalType(DataType type) {
        return type == DataType::F32 || type == DataType::Int32;
    }

    inline int getTypeBits(DataType type) {
        switch (type) {
            case DataType::Int32:
                return 32;
            case DataType::F32:
                return 32;
            case DataType::F64:
                return 64;
        }
    }

    inline DataType getHighestPrecisionType(DataType a, DataType b) {
        int bits = std::max(getTypeBits(a), getTypeBits(b));
        
        if (isDecimalType(a) || isDecimalType(b))
            return bits == 32 ? DataType::F32 : DataType::F64;
        else
            return DataType::Int32;
    }

    struct Value {
        Value() {}
        Value(int i) : type(DataType::Int32), intVal(i) {}
        Value(float f) : type(DataType::F32), floatVal(f) {}
        Value(double d) : type(DataType::F64), doubleVal(d) {}
        DataType type;
        union {
            int intVal;
            float floatVal;
            double doubleVal;
        };

        template <typename T>
        T as() const {
            switch (type) {
                case DataType::Int32:
                    return (T)intVal;
                case DataType::F32:
                    return (T)floatVal;
                case DataType::F64:
                    return (T)doubleVal;
                default:
                    return T{};
            }
        }

        Value as(DataType type) const {
            switch (type) {
                case DataType::Int32:
                    return Value(as<int>());
                case DataType::F32:
                    return Value(as<float>());
                case DataType::F64:
                    return Value(as<double>());
                default:
                    return Value(0);
            }
        }

        Value operator*(const Value& other) {
            if (other.type != type) {
                DataType newType = getHighestPrecisionType(other.type, type);
                return other.as(newType) * as(newType);
            }

            switch (type) {
                case DataType::Int32:
                    return other.intVal * intVal;
                case DataType::F32:
                    return other.floatVal * floatVal;
                case DataType::F64:
                    return other.doubleVal * doubleVal;
            }
        }

        Value operator+(const Value& other) {
            if (other.type != type) {
                DataType newType = getHighestPrecisionType(other.type, type);
                return other.as(newType) + as(newType);
            }

            switch (type) {
                case DataType::Int32:
                    return other.intVal + intVal;
                case DataType::F32:
                    return other.floatVal + floatVal;
                case DataType::F64:
                    return other.doubleVal + doubleVal;
            }
        }

        Value operator-(const Value& other) {
            if (other.type != type) {
                DataType newType = getHighestPrecisionType(other.type, type);
                return other.as(newType) - as(newType);
            }

            switch (type) {
                case DataType::Int32:
                    return other.intVal - intVal;
                case DataType::F32:
                    return other.floatVal - floatVal;
                case DataType::F64:
                    return other.doubleVal - doubleVal;
            }
        }

        Value operator/(const Value& other) {
            if (other.type != type) {
                DataType newType = getHighestPrecisionType(other.type, type);
                return other.as(newType) / as(newType);
            }

            switch (type) {
                case DataType::Int32:
                    return other.intVal / intVal;
                case DataType::F32:
                    return other.floatVal / floatVal;
                case DataType::F64:
                    return other.doubleVal / doubleVal;
            }
        }
    };

    struct Variable {
        std::string name;
        Value val;
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

    class ProducesValueNode : public ASTNode {
    protected:
        ProducesValueNode(ASTNodeType nodeType) : ASTNode(nodeType) {}
    public:
        virtual Value getValue() = 0;
    };

    class ConstValNode : public ProducesValueNode {
        public:
            ConstValNode(Value val = 0) : ProducesValueNode(ASTNodeType::ConstVal), val(val) {}
            Value val;
            Value getValue() override { return val; }
    };

    enum class ArithmeticOperation {
        Add,
        Subtract,
        Divide,
        Multiply,
        Count
    };

    extern EnumNames<ArithmeticOperation> arithOperationNames;

    class ArithmeticNode : public ProducesValueNode {
        public:
            ArithmeticNode() : ProducesValueNode(ASTNodeType::Arithmetic) {}
            std::shared_ptr<ProducesValueNode> a;
            std::shared_ptr<ProducesValueNode> b;
            ArithmeticOperation operation;

            Value calculateValue() {
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

            Value getValue() override {
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
