#include "parser.hpp"

namespace iodine {
    EnumNames<ASTNodeType> nodeTypeNames {
        "Script",
        "IntegerVal",
        "Arithmetic",
        "Expression",
        "VarAssignment"
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
        "Operator"
    };

    EnumNames<ArithmeticOperation> arithOperationNames {
        "Add",
        "Subtract",
        "Divide",
        "Multiply"
    };
}
