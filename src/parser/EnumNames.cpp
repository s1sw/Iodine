#include "parser.hpp"

namespace iodine {
    EnumNames<ASTNodeType> nodeTypeNames {
        "Script",
        "ConstVal",
        "Arithmetic",
        "Expression",
        "VarAssignment",
        "UnaryOp",
        "VariableReference"
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
        "Equals"
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
}
