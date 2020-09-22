#pragma once
#include <cassert>
#include <initializer_list>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>

namespace iodine {
    enum class DataType {
        Int32,
        F32,
        F64,
        Ref
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
        Equals,
        Count
    };

    enum class ASTNodeType {
        Script,
        ConstVal,
        Arithmetic,
        Expression,
        VarAssignment,
        UnaryOp,
        VariableReference,
        Count
    };

    inline bool isNumberType(DataType type) {
      return type == DataType::F32 || type == DataType::Int32 || type == DataType::F64;
    }

    inline int getTypeBits(DataType type) {
        assert(isNumberType(type));
        switch (type) {
            case DataType::Int32:
                return 32;
            case DataType::F32:
                return 32;
            case DataType::F64:
                return 64;
            default:
                return -1;
        }
    }

    inline DataType getHighestPrecisionType(DataType a, DataType b) {
        int bits = std::max(getTypeBits(a), getTypeBits(b));

        if (isNumberType(a) || isNumberType(b))
          return bits == 32 ? DataType::F32 : DataType::F64;
        else
            return DataType::Int32;
    }

    struct TypeInfo {
        std::string name;
    };

    struct Ref {
        std::shared_ptr<TypeInfo> typeInfo;
        void* data;
    };

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

        // Declaring outside the union since it has a non-trivial
        // destructor. Wastes some memory, but oh well!
        Ref ref;

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

        void flipSign() {
            assert(isNumberType(type));
            switch (type) {
                case DataType::Int32:
                    intVal = -intVal;
                    break;
                case DataType::F32:
                    floatVal = -floatVal;
                    break;
                case DataType::F64:
                    doubleVal = -doubleVal;
                    break;
                default:
                    assert(true ^ (bool)"Flipping sign doesn't make sense for this dataType!");;
                    break;
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
                default:
                    return Value{0};
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
                default:
                    return Value{0};
            }
        }

        Value operator-(const Value& other) {
            if (other.type != type) {
                DataType newType = getHighestPrecisionType(other.type, type);
                return other.as(newType) - as(newType);
            }

            switch (type) {
                case DataType::Int32:
                    return intVal - other.intVal;
                case DataType::F32:
                    return floatVal - other.floatVal;
                case DataType::F64:
                    return doubleVal - other.doubleVal; 
                default:
                    return Value{0};
            }
        }

        Value operator/(const Value& other) {
            if (other.type != type) {
                DataType newType = getHighestPrecisionType(other.type, type);
                return other.as(newType) / as(newType);
            }

            switch (type) {
                case DataType::Int32:
                    return intVal / other.intVal; 
                case DataType::F32:
                    return floatVal / other.floatVal;
                case DataType::F64:
                    return doubleVal / other.doubleVal;
                default:
                    return Value{0};
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

    enum class UnaryOperation {
        Plus,
        Minus,
        Count
    };

    extern EnumNames<UnaryOperation> unaryOperationNames;

    class UnaryOpNode : public ProducesValueNode {
        public:
            UnaryOpNode() : ProducesValueNode(ASTNodeType::UnaryOp) {}
            std::shared_ptr<ProducesValueNode> valNode;
            UnaryOperation operation;

            Value getValue() override {
                Value val = valNode->getValue();

                if (operation == UnaryOperation::Minus) {
                    val.flipSign();
                }

                return val;
            }
    };

    class VarAssignmentNode : public ASTNode {
        public:
            VarAssignmentNode() : ASTNode(ASTNodeType::VarAssignment) {}
            std::shared_ptr<ProducesValueNode> valNode;
            std::string varName;
    };

    extern std::unordered_map<std::string, Value> variables;

    class VariableReferenceNode : public ProducesValueNode {
        public:
           VariableReferenceNode() : ProducesValueNode(ASTNodeType::VariableReference) {} 
           std::string varName;

           Value getValue() override {
               return variables[varName];
           }
    };

    std::vector<Token> parseTokens(std::string str); 
    std::shared_ptr<ASTNode> parseScript(std::vector<Token> tokens);
}
