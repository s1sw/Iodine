#include <memory>
#include <math.h>
#include <string.h>
#include <parser.hpp>
#include <iostream>
#include <unordered_map>

using namespace iodine;

void printIndents(int indents) {
    for (int i = 0; i < indents; i++) {
        std::cout << "  ";
    }
}

std::string valueToStr(const Value& val) {
    switch (val.type) {
        case DataType::F32:
            return std::to_string(val.floatVal);
            break;
        case DataType::F64:
            return std::to_string(val.doubleVal);
        case DataType::Int32:
            return std::to_string(val.intVal);
            break;
        case DataType::Null:
            return "(null)";
            break;
        case DataType::Boolean:
            return std::to_string(val.boolVal);
            break;
        case DataType::ConstStr:
            return std::string(val.constStrVal);
            break;
        default:
            return "";
    }
}

std::unordered_map<std::string, Variable> iodine::variables;
std::unordered_map<std::string, Function> iodine::functions;

void printASTNode(std::shared_ptr<ASTNode> node, int indentDepth = 0) {
    printIndents(indentDepth);
    std::cout << "Node type: " << nodeTypeNames[node->type] << "\n";
    switch (node->type) {
        case ASTNodeType::ConstVal:
            printIndents(indentDepth);
            std::cout << "val: " << valueToStr(std::static_pointer_cast<ConstValNode>(node)->val) << "\n";
            break;
        case ASTNodeType::Arithmetic:
        {
            auto aNode = std::static_pointer_cast<ArithmeticNode>(node);
            printIndents(indentDepth);
            std::cout << "a: \n";
            printASTNode(aNode->a, indentDepth + 1);

            printIndents(indentDepth);
            std::cout << "b: \n";
            printASTNode(aNode->b, indentDepth + 1);

            printIndents(indentDepth);
            std::cout << "operation: " << arithOperationNames[aNode->operation] << "\n";
            break;
        }
        case ASTNodeType::UnaryOp:
        {
            printIndents(indentDepth);
            auto uNode = std::static_pointer_cast<UnaryOpNode>(node);
            std::cout << "operation: " << unaryOperationNames[uNode->operation] << "\n";
            printIndents(indentDepth);
            std::cout << "child:\n";
            printASTNode(uNode->valNode, indentDepth + 1);
            break;
        }
        case ASTNodeType::VarAssignment:
        {
            printIndents(indentDepth);
            auto assignmentNode = std::static_pointer_cast<VarAssignmentNode>(node);

            std::cout << "variable name: " << assignmentNode->varName << "\n";

            printIndents(indentDepth);
            std::cout << "value:\n";
            printASTNode(assignmentNode->valNode, indentDepth + 1);
            break;
        }
        case ASTNodeType::VariableReference:
        {
            printIndents(indentDepth);
            auto refNode = std::static_pointer_cast<VariableReferenceNode>(node);
            std::cout << "varname: " << refNode->varName << "\n";
            printIndents(indentDepth);
            auto it = variables.find(refNode->varName);

            if (it != variables.end()) {
                std::cout << "varinfo:" << (int)variables.at(refNode->varName).type << "\n";
            }
            break;
        }
        case ASTNodeType::FunctionCall:
        {
            printIndents(indentDepth);
            auto functionCall = std::static_pointer_cast<FunctionCallNode>(node);
            std::cout << "func name: " << functionCall->functionName << "\n";

            for (size_t i = 0; i < functionCall->args.size(); i++) {
                printIndents(indentDepth);
                std::cout << "arg " << i << ":\n";

                printASTNode(functionCall->args[i], indentDepth + 1);
            }
            break;
        }
        case ASTNodeType::If:
        {
            auto in = std::static_pointer_cast<IfNode>(node);
            printIndents(indentDepth);
            std::cout << "condition:\n";
            printIndents(indentDepth);
            printASTNode(in->condition, indentDepth + 1);

            printIndents(indentDepth);
            std::cout << "children:\n";
            for (auto& n : in->nodes) {
                printIndents(indentDepth);
                printASTNode(n, indentDepth + 1);
            }
            break;
        }
        default:
            break;
    }
}

Value evalAST(std::shared_ptr<ASTNode> exprRoot) {
    if (exprRoot->type == ASTNodeType::VarAssignment) {
        auto assignNode = std::static_pointer_cast<VarAssignmentNode>(exprRoot);

        auto val = assignNode->valNode->getValue();
        if (assignNode->createNew) {
            Variable var;
            var.name = assignNode->varName;
            var.type = assignNode->type;
            var.val = val.as(var.type);
            variables[assignNode->varName] = var;
        } else {
            auto varIt = variables.find(assignNode->varName);
            if (varIt == variables.end()) 
                throw std::runtime_error("Reference to undefined variable " + assignNode->varName);
            if (val.type != varIt->second.type) {
                std::string msg = "Assignment to variable "
                    + assignNode->varName + " (" + dataTypeNames[varIt->second.type] + ")"
                    + " with wrong type " + dataTypeNames[val.type];
                throw std::runtime_error(msg);
            }
            variables.at(assignNode->varName).val = val;
        }
        return Value{};
    }

    if (exprRoot->type == ASTNodeType::If) {
        auto ifNode = std::static_pointer_cast<IfNode>(exprRoot);

        if (ifNode->condition->getValue().as<bool>()) {
            for (auto& n : ifNode->nodes) {
                evalAST(n);
            }
        }
        return Value{};
    }
    return std::static_pointer_cast<ProducesValueNode>(exprRoot)->getValue();
}

void printTokens(std::vector<Token>& tokens) {
    for (auto& token : tokens) {
        std::cout << "Token: " << tokenNames[token.type];
        std::cout << " (" << token.val << ")";
        std::cout << "\n";
    }
}

int main(int argc, char** argv) {
    bool doPrintTokens = false;
    bool printAST = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-tokens") == 0) {
            doPrintTokens = true;
        }

        if (strcmp(argv[i], "--print-ast") == 0) {
            printAST = true;
        }
    }

    Function sqrt {true, "sqrt", [](std::vector<std::shared_ptr<ProducesValueNode>> args) {
        if (args.size() != 1) {
            throw std::runtime_error("incorrect num args");
        }

        auto arg = args[0];

        if (arg->getValue().type == DataType::F32) {
            return Value(sqrtf(arg->getValue().as<float>()));
        } else if (arg->getValue().type == DataType::F64) {
            return Value(::sqrt(arg->getValue().as<double>()));
        } else if (arg->getValue().type == DataType::Int32) {
            return Value(round(::sqrt(arg->getValue().as<double>())));
        } else {
            throw std::runtime_error("invalid type!");
        }
    }};

    Function println {true, "println", [](FuncArgs args) {
        if (args.size() != 1) throw std::runtime_error("Incorrect number of arguments");

        std::cout << valueToStr(args[0]->getValue()) << "\n"; 
        return Value();
    }};

    functions.insert({ "sqrt", sqrt });
    functions.insert({ "println", println });

    while (true) {
        std::string line;

        std::cout << ">";

        std::getline(std::cin, line);

        if (std::cin.eof())
            break;

        try {
            if (*(line.end() - 1) == '\n') {
                line = line.substr(0, line.size() - 1);
            }
            std::vector<Token> tokens = parseTokens(line);

            if (doPrintTokens)
                printTokens(tokens);

            std::shared_ptr<ASTNode> n = parseExpression(tokens);

            if (n == nullptr) {
                std::cout << "AST is empty\n";
            } else {

                if (printAST)
                    printASTNode(n);

                Value val = evalAST(n);

                if (val.type != DataType::Null) {
                    std::cout << valueToStr(val) << "\n";
                }
            }
        } catch (std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    } 
    return 0;
}
