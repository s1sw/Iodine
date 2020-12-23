#pragma once
#include <string.h>
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace iodine {
    enum class DataType {
        Int32,
        F32,
        F64,
        Ref,
        Null,
        Boolean,
        ConstStr,
        Count
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
        If,
        OpenBrace,
        CloseBrace,
        NewLine,
        True,
        False,
        StringContents,
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
        FunctionCall,
        If,
        Comparison,
        Count
    };

    enum class ComparisonType {
        Equal,
        GreaterThan,
        LessThan,
        GreaterThanOrEqual,
        LessThanOrEqual
    };


    template <typename T>
    class EnumNames {
    public:
        EnumNames(const char* names[(int)T::Count])
            : names(names) { }
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
    extern EnumNames<DataType> dataTypeNames;

    inline bool isNumberType(DataType type) {
        return type == DataType::F32 || type == DataType::Int32 || type == DataType::F64;
    }

    inline bool isDecimalType(DataType type) {
        return type == DataType::F32 || type == DataType::F64;
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

        if (isDecimalType(a) || isDecimalType(b))
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
        Value() : type(DataType::Null) { }
        Value(int i)
            : type(DataType::Int32)
            , intVal(i) { }
        Value(float f)
            : type(DataType::F32)
            , floatVal(f) { }
        Value(double d)
            : type(DataType::F64)
            , doubleVal(d) { }
        Value(bool b)
            : type(DataType::Boolean)
            , boolVal(b) { }
        Value(const char* constStr)
            : type(DataType::ConstStr)
            , constStrVal(constStr) {}

        DataType type;
        union {
            int intVal;
            float floatVal;
            double doubleVal;
            bool boolVal;
            const char* constStrVal;
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
            case DataType::Boolean:
                return (T)boolVal;
            default:
                throw std::runtime_error("Invalid cast");
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
                assert(false && (bool)"Flipping sign doesn't make sense for this dataType!");
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
                return Value { 0 };
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
                return Value { 0 };
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
                return Value { 0 };
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
                return Value { 0 };
            }
        }

        Value isEqual(const Value& other) {
            if (type != other.type)
                throw std::runtime_error("Trying to compare values of different types");

            switch (type) {
                case DataType::Int32:
                    return Value(intVal == other.intVal);
                case DataType::Boolean:
                    return Value(boolVal == other.boolVal);
                case DataType::F32:
                    return Value(floatVal == other.floatVal);
                case DataType::F64:
                    return Value(doubleVal == other.doubleVal);
                case DataType::ConstStr:
                    return Value(strcmp(constStrVal, other.constStrVal));
                default:
                    throw std::runtime_error(std::string("No comparison for ") + dataTypeNames[type]);
            }
        }
    };

    struct Variable {
        std::string name;
        Value val;
        DataType type;
    };


    struct Token {
        TokenType type;
        std::string val;
        int numberVal;
    };

    class ASTNode {
    protected:
        ASTNode(ASTNodeType type)
            : type(type) { }

        virtual ~ASTNode() {}
    public:
        ASTNodeType type;
    };

    class ScriptNode : public ASTNode {
    public:
        ScriptNode()
            : ASTNode(ASTNodeType::Script) { }
        std::vector<ASTNode*> children;

        virtual ~ScriptNode() {}
    };

    class ProducesValueNode : public ASTNode {
    protected:
        ProducesValueNode(ASTNodeType nodeType)
            : ASTNode(nodeType) { }

    public:
        virtual Value getValue() = 0;
        virtual ~ProducesValueNode() {}
    };

    class ConstValNode : public ProducesValueNode {
    public:
        ConstValNode(Value val = 0)
            : ProducesValueNode(ASTNodeType::ConstVal)
            , val(val) { }
        Value val;
        Value getValue() override { return val; }
        
        virtual ~ConstValNode() {}
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
        ArithmeticNode()
            : ProducesValueNode(ASTNodeType::Arithmetic) { }

        virtual ~ArithmeticNode() {}
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
        UnaryOpNode()
            : ProducesValueNode(ASTNodeType::UnaryOp) { }
        virtual ~UnaryOpNode() {}
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
        VarAssignmentNode()
            : ASTNode(ASTNodeType::VarAssignment) { }
        virtual ~VarAssignmentNode() {}
        std::shared_ptr<ProducesValueNode> valNode;
        bool createNew;
        std::string varName;
        DataType type;
    };

    extern std::unordered_map<std::string, Variable> variables;

    class VariableReferenceNode : public ProducesValueNode {
    public:
        VariableReferenceNode()
            : ProducesValueNode(ASTNodeType::VariableReference) { }
        virtual ~VariableReferenceNode() {}
        std::string varName;

        Value getValue() override {
            auto varIt = variables.find(varName);
            if (varIt == variables.end())
                throw std::runtime_error("Nonexistent variable " + varName + " referenced");

            return varIt->second.val;
        }
    };

    typedef std::vector<std::shared_ptr<ProducesValueNode>> FuncArgs;

    typedef std::function<Value(FuncArgs)> NativeFunction;

    struct Function {
        bool isBuiltin;
        std::string name;
        NativeFunction nativeFunc;
    };

    extern std::unordered_map<std::string, Function> functions;

    class FunctionCallNode : public ProducesValueNode {
    public:
        FunctionCallNode() : ProducesValueNode(ASTNodeType::FunctionCall) {}

        virtual ~FunctionCallNode() {}

        std::string functionName;
        std::vector<std::shared_ptr<ProducesValueNode>> args;

        Value getValue() override {
            auto iter = functions.find(functionName);

            if (iter == functions.end())
               throw std::runtime_error("Tried to call nonexistent function"); 

            assert(iter->second.isBuiltin);

            return iter->second.nativeFunc(args);
        }
    };


    class ComparisonNode : public ProducesValueNode {
    public:
        ComparisonNode() : ProducesValueNode(ASTNodeType::Comparison) {}

        virtual ~ComparisonNode() {}
        std::shared_ptr<ProducesValueNode> lhs;
        std::shared_ptr<ProducesValueNode> rhs;
        ComparisonType compType;

        Value getValue() override {
            switch (compType) {
                case ComparisonType::Equal:
                    if (lhs->getValue().type != rhs->getValue().type)
                        throw std::runtime_error("Trying to compare values of different types");
                    return lhs->getValue().isEqual(rhs->getValue());
                default:
                    return false;
            }
        }
    };

    class IfNode : public ASTNode {
    public:
        IfNode() : ASTNode(ASTNodeType::If) {}

        virtual ~IfNode() {}

        std::vector<std::shared_ptr<ASTNode>> nodes;
        std::shared_ptr<ProducesValueNode> condition;
    };

    std::vector<Token> parseTokens(std::string str);
    std::shared_ptr<ASTNode> parseExpression(std::vector<Token> tokens);
    std::vector<std::shared_ptr<ASTNode>> parseScript(std::vector<Token> tokens);
}
