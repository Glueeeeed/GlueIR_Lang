#include "semantic.h"
#include "../parser/parser.h"
#include "scope.h"
#include <sstream>

std::string SemanticAnalyzer::nodeTypeToString(NodeType t) {
    switch (t) {
        case NodeType::NUMBER: return "'int'";
        case NodeType::NUMBER_DOUBLE: return "'double'";
        case NodeType::STRING: return "'string'";
        case NodeType::NUMBER_FLOAT : return "'float'";
        case NodeType::BOND : return "'bond'";
        case NodeType::BOOLEAN: return "'boolean'";
        default: return "unknown";
    }
}

void SemanticAnalyzer::analyse(const ASTNode *root) {
    visit(root);
}

void SemanticAnalyzer::visit(const ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NodeType::PROGRAM: {
            visitProgram(node);
            break;
        }
        case NodeType::FUNCTION_DECLARATION:
            visitFunctionDeclaration(node);
            break;
        case NodeType::BLOCK: {
            symbolTable.enterScope();
            for (const auto& child : node->children) {
                visit(child.get());
            }
            symbolTable.exitScope();
            break;
        }
        case NodeType::DECLARATION:
            visitDeclaration(node);
            break;
        case NodeType::ASSIGNMENT:
            visitAssignment(node);
            break;
        case NodeType::FUNCTION_CALL:
            visitFunctionCall(node);
            for (const auto& child : node->children) {
                inferType(child.get());
            }
            break;
        case NodeType::RETURN_STATEMENT: {
            visitReturn(node);
            break;
        }
        case NodeType::IF_STATEMENT: {
            visitIf(node);
            break;
        }
        case NodeType::WHILE_STATEMENT: {
            visitWhile(node);
            break;
        }
        default:
            break;
    }
}

void SemanticAnalyzer::visitDeclaration(const ASTNode* node) {
    const ASTNode* idNode   = node->children[0].get();
    const ASTNode* typeNode = node->children[1].get();
    const ASTNode* valNode  = node->children[2].get();

    std::string varName = idNode->value;
    std::string varTypeFormatted = "'" + typeNode->value + "'";
    std::string varType = typeNode->value;
    NodeType valueType = inferType(valNode);

    if (!isCompatible(varType, valueType)) {
        expect("Compile Error: type mismatch; cannot assign " + nodeTypeToString(valueType) + " to variable '" + varName + "' of type " + varTypeFormatted, valNode->line, valNode->column);
    }

    SymbolInfo info;
    info.type = varType;
    info.isConst = idNode->isConst;
    info.isSticky = idNode->isSticky;
    info.stickyUsed = idNode->stickyUsed;

    if (!symbolTable.declare(varName, info)) {
        expect("Compile Error: variable '" + varName + "' is already declared in this scope", idNode->line, idNode->column);
    }
}


NodeType SemanticAnalyzer::inferType(const ASTNode* node) {
    if (!node) return NodeType::BOND;

    switch (node->type) {
        case NodeType::FUNCTION_CALL: {
            visitFunctionCall(node);

            static const std::unordered_map<std::string_view, NodeType> builtins = {
                {"shin", NodeType::STRING},{"toInt", NodeType::NUMBER}, {"random", NodeType::NUMBER},
                {"toDouble", NodeType::NUMBER_DOUBLE},
                {"toFloat", NodeType::NUMBER_FLOAT}
            };

            if (auto it = builtins.find(node->value); it != builtins.end()) {
                return it->second;
            }

            if (SymbolInfo* info = symbolTable.lookup(node->value)) {
                return mapStringToNodeType(info->type);
            }

            expect("Compile Error: undefined function '" + node->value + "'", node->line, node->column);
            return NodeType::BOND;
        }

        case NodeType::BINARY_OPERATION: {
            NodeType leftType = inferType(node->children[0].get());
            NodeType rightType = inferType(node->children[1].get());

            if (leftType == NodeType::STRING || rightType == NodeType::STRING) {
                expect("Compile Error: Operator '" + node->value + "' is not supported for type 'string'", node->line, node->column);
            }

            if (node->value == "/") {
                checkDivisionByZero(node->children[1].get(), node->line, node->column);
            }

            if (leftType == NodeType::NUMBER_DOUBLE || rightType == NodeType::NUMBER_DOUBLE) return NodeType::NUMBER_DOUBLE;
            if (leftType == NodeType::NUMBER_FLOAT || rightType == NodeType::NUMBER_FLOAT) return NodeType::NUMBER_FLOAT;

            return NodeType::NUMBER;
        }

        case NodeType::IDENTIFIER: {
            if (SymbolInfo* info = symbolTable.lookup(node->value)) {
                return mapStringToNodeType(info->type);
            }
            expect("Compile Error: variable '" + node->value + "' is not declared in this scope", node->line, node->column);
            return NodeType::BOND;
        }

        default:
            return node->type;
    }
}


bool SemanticAnalyzer::isCompatible(const std::string& declaredType, NodeType valueType) {

    if (declaredType == "int" && valueType == NodeType::NUMBER) {
        return true;
    }

    if (declaredType == "double" && (valueType == NodeType::NUMBER || valueType == NodeType::NUMBER_DOUBLE)) {
        return true;
    }

    if (declaredType == "string" && valueType == NodeType::STRING) {
        return true;
    }

    if (declaredType == "float" && valueType == NodeType::NUMBER_FLOAT) {
        return true;
    }

    if ((declaredType == "boolean" || declaredType == "bool") && valueType == NodeType::BOOLEAN) {
        return true;
    }

    return false;
}

void SemanticAnalyzer::expect(std::string msg, int line, int column) {
    std::stringstream stream;
    stream << msg;
    if (line > 0) {
        stream << " at " << line << ":" << column;
    }
    stream << std::endl;
    throw ParseError(stream.str());
}

void SemanticAnalyzer::declareSymbol(const ASTNode *node, bool isConst, bool stickyUsed, bool isSticky) {
    std::string idNode = node->children[0]->value;
    std::string typeNode = node->children[1]->value;
    SymbolInfo info;
    info.type = typeNode;
    info.isConst = isConst;
    info.isSticky = isSticky;
    info.stickyUsed = stickyUsed;
    symbols[idNode] = info;

}

void SemanticAnalyzer::visitAssignment(const ASTNode* node) {
    const ASTNode* idNode  = node->children[0].get();
    const ASTNode* valNode = node->children[1].get();
    std::string varName = idNode->value;
    NodeType valueType = inferType(valNode);

    SymbolInfo* info = symbolTable.lookup(varName);
    if (!info) {
        expect("Compile Error: variable '" + varName + "' is not declared in this scope", idNode->line, idNode->column);
    }

    if (info->isConst) {
        expect("Compile Error: cannot assign to variable '" + varName + "' because it is a constant", idNode->line, idNode->column);
    }

    if (info->isSticky) {
        if (info->stickyUsed) {
            expect("Compile Error: variable '" + varName + "' is 'sticky' and has already been reassigned once", idNode->line, idNode->column);
        } else {
            info->stickyUsed = true;
        }
    }

    if (!isCompatible(info->type, valueType)) {
        expect("Compile Error: type mismatch; cannot assign " + nodeTypeToString(valueType) + " to variable '" + varName + "' of type '" + info->type + "'", valNode->line, valNode->column);
    }
}

void SemanticAnalyzer::visitFunctionCall(const ASTNode* node) {
    std::string funcName = node->value;


    if (funcName == "random") {
        if (node->children.size() != 2) expect("Compile Error: function '" + funcName + "' expects 2 arguments, but " + std::to_string(node->children.size()) + " were provided", node->line, node->column);
    }

    if (funcName == "shout" || funcName == "shin"  || funcName == "random" || funcName == "toInt" || funcName == "toFloat" || funcName == "toDouble" ) return;

    SymbolInfo* info = symbolTable.lookup(funcName);
    if (!info) expect("Compile Error: function '" + funcName + "' is not declared", node->line, node->column);


    if (node->children.size() != info->args.size()) {
        expect("Compile Error: function '" + funcName + "' expects " + std::to_string(info->args.size()) +
               " arguments, but " + std::to_string(node->children.size()) + " were provided", node->line, node->column);
        return;
    }

    for (size_t i = 0; i < node->children.size(); ++i) {
        const auto* argNode = node->children[i].get();
        NodeType inferredType = inferType(argNode);
        const std::string& expectedType = info->args[i];

        if (!isCompatible(expectedType, inferredType)) {
            expect("Compile Error: argument " + std::to_string(i + 1) + " of function '" + funcName +
                   "' is of type " + nodeTypeToString(inferredType) + ", but expected type is '" + expectedType + "'",
                   argNode->line, argNode->column);
        }
    }
}

void SemanticAnalyzer::visitFunctionDeclaration(const ASTNode* node) {
    const std::string& funcName = node->value;
    const std::string& returnType = node->children[0]->value;

    currentFunctionName = funcName;
    currentFunctionReturnType = returnType;

    SymbolInfo info;
    info.type = returnType;

    const ASTNode* paramsNode = (node->children.size() > 2) ? node->children[1].get() : nullptr;

    if (paramsNode) {
        for (const auto& paramNode : paramsNode->children) {
            info.args.push_back(paramNode->children[1]->value); // paramType
        }
    }

    symbolTable.declare(funcName, info);

    const ASTNode* bodyNode = node->children.back().get();
    bool isMain = (funcName == "main" || funcName == "Main");
    bool hasReturnAtEnd = hasEndingReturn(bodyNode);

    if (isMain) {
        if (returnType != "int") {
            expect("Compile Error: 'main' function must return type 'int'", node->line, node->column);
        }
        if (!hasReturnAtEnd) {
            expect("Compile Error: Function 'main' must end with a return statement (e.g., return 0;)", node->line, node->column);
        }
    } else if (returnType != "void" && !hasReturnAtEnd) {
        expect("Compile Error: Function '" + funcName + "' with return type '" + returnType + "' must return a value", node->line, node->column);
    }

    symbolTable.enterScope();

    if (paramsNode) {
        for (const auto& param : paramsNode->children) {
            const std::string& paramName = param->children[0]->value;
            const std::string& paramType = param->children[1]->value;

            SymbolInfo paramInfo{paramType, {}, false, false};

            if (!symbolTable.declare(paramName, paramInfo)) {
                expect("Compile Error: redeclaration of parameter '" + paramName + "'", param->line, param->column);
            }
        }
    }

    visit(bodyNode);

    symbolTable.exitScope();
}






