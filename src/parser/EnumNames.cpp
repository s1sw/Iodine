#include "parser.hpp"

namespace iodine {
    EnumNames<ASTNodeType> nodeTypeNames {
        "Script",
        "ConstVal",
        "Arithmetic",
        "Expression",
        "VarAssignment",
        "UnaryOp",
        "VariableReference",
        "FunctionCall",
        "If",
        "Comparison"
    };

    EnumNames<TokenType> tokenNames {
        "Name",
        "Semicolon",
        "OpenParenthesis",
        "CloseParenthesis",
        "Comma",
        "DoubleQuote",
        "Number",
        "DecimalNumber",
        "Operator",
        "Equals",
        "If",
        "OpenBrace",
        "CloseBrace",
        "NewLine",
        "True",
        "False",
        "StringContents"
    };

    EnumNames<ArithmeticOperation> arithOperationNames {
        "Add",
        "Subtract",
        "Divide",
        "Multiply"
    };

    EnumNames<UnaryOperation> unaryOperationNames {
        "Plus",
        "Minus"
    };

    EnumNames<DataType> dataTypeNames {
        "Int32",
        "F32",
        "F64",
        "Ref",
        "Boolean",
        "ConstStr",
        "Null"
    };
}
