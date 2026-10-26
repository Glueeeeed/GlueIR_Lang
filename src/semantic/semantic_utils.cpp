#include "semantic.h"


NodeType SemanticAnalyzer::mapStringToNodeType(std::string_view typeStr) const {
    static const std::unordered_map<std::string_view, NodeType> typeMap = {
        {"int", NodeType::NUMBER},
        {"double", NodeType::NUMBER_DOUBLE},
        {"float", NodeType::NUMBER_FLOAT},
        {"string", NodeType::STRING},
        {"bond", NodeType::BOND},
        {"bool", NodeType::BOOLEAN},
        {"boolean", NodeType::BOOLEAN}
    };

    if (auto it = typeMap.find(typeStr); it != typeMap.end()) {
        return it->second;
    }
    return NodeType::BOND;
}


std::string SemanticAnalyzer::extractNotNullVar(const ASTNode* cond) {
    if (!cond) return "";

    if (cond->type == NodeType::IDENTIFIER) {
        SymbolInfo* info = symbolTable.lookup(cond->value);
        if (info && info->isNullable) {
            return cond->value;
        }
        return "";
    }

    if (cond->type == NodeType::BINARY_OPERATION && cond->value == "!=") {
        const ASTNode* left = cond->children[0].get();
        const ASTNode* right = cond->children[1].get();

        if (left->type == NodeType::IDENTIFIER && right->type == NodeType::NULO) {
            return left->value;
        }
        if (left->type == NodeType::NULO && right->type == NodeType::IDENTIFIER) {
            return right->value;
        }
    }

    return "";
}


std::string SemanticAnalyzer::extractNullVar(const ASTNode* cond) {
    if (!cond || cond->type != NodeType::BINARY_OPERATION) return "";

    if (cond->value == "==") {
        const ASTNode* left = cond->children[0].get();
        const ASTNode* right = cond->children[1].get();

        if (left->type == NodeType::IDENTIFIER && right->type == NodeType::NULO) {
            return left->value;
        }
        if (left->type == NodeType::NULO && right->type == NodeType::IDENTIFIER) {
            return right->value;
        }

        if (left->type == NodeType::IDENTIFIER && right->type == NodeType::BOOLEAN && right->value == "false") {
            SymbolInfo* info = symbolTable.lookup(left->value);
            if (info && info->isNullable) {
                return left->value;
            }
        }
    }


    return "";
}

bool SemanticAnalyzer::isExprNullable(const ASTNode* node) {
    if (!node) return false;

    switch (node->type) {
        case NodeType::NULO:
            return true;

        case NodeType::IDENTIFIER: {
            SymbolInfo* info = symbolTable.lookup(node->value);
            if (info) {
                return info->isNullable;
            }
            return false;
        }

        case NodeType::FUNCTION_CALL: {
            SymbolInfo* info = symbolTable.lookup(node->value);
            if (info) {
                return info->isNullable;
            }
            return false;
        }

        case NodeType::BINARY_OPERATION: {
            if (node->value == "??") {
                return isExprNullable(node->children[1].get());
            }
            return false;
        }

        default:
            return false;
    }
}


void SemanticAnalyzer::checkDivisionByZero(const ASTNode* rightChild, int line, int column) {
    if (!rightChild) return;

    bool isNumericLiteral = (rightChild->type == NodeType::NUMBER ||
                             rightChild->type == NodeType::NUMBER_DOUBLE ||
                             rightChild->type == NodeType::NUMBER_FLOAT);

    if (isNumericLiteral && std::stod(rightChild->value) == 0.0) {
        expect("Compile Error: Division by zero", line, column);
    }
}

void SemanticAnalyzer::visitProgram(const ASTNode* node) {
    bool hasMain = false;
    for (const auto& child : node->children) {
        if (child->type == NodeType::FUNCTION_DECLARATION && (child->value == "main" || child->value == "Main")) {
            hasMain = true;
        }
        visit(child.get());
    }
    if (!hasMain) {
        expect("Compile Error: Program must contain a 'main' function");
    }
}

void SemanticAnalyzer::visitReturn(const ASTNode* node) {
    if (currentFunctionReturnType == "void") {
        if (!node->children.empty()) {
            expect("Compile Error: Function '" + currentFunctionName + "' with return type 'void' cannot return a value", node->line, node->column);
        }
    } else {
        if (node->children.empty()) {
            expect("Compile Error: Function '" + currentFunctionName + "' with return type '" + currentFunctionReturnType + "' must return a value", node->line, node->column);
        } else {
            const ASTNode* exprNode = node->children[0].get();
            NodeType retType = inferType(exprNode);
            bool retIsNullable = isExprNullable(exprNode);

            if (!isCompatible(currentFunctionReturnType, retType)) {
                expect("Compile Error: Function '" + currentFunctionName + "' with return type '" + currentFunctionReturnType + "' cannot return value of type " + nodeTypeToString(retType), node->line, node->column);
            }

            if (!currentFunctionIsNullable && retIsNullable) {
                expect("Compile Error: cannot return nullable value from function '" + currentFunctionName + "' with non-nullable return type '" + currentFunctionReturnType + "'", node->line, node->column);
            }
        }
    }
}

void SemanticAnalyzer::visitIf(const ASTNode* node) {
    const ASTNode* condNode = node->children[0].get();
    const ASTNode* thenNode = node->children[1].get();
    const ASTNode* elseNode = (node->children.size() > 2) ? node->children[2].get() : nullptr;

    NodeType condType = inferType(condNode);
    if (condType != NodeType::BOOLEAN && condType != NodeType::NUMBER &&
        condType != NodeType::NUMBER_DOUBLE && condType != NodeType::NUMBER_FLOAT) {
        expect("Compile Error: If statement condition must be of type boolean or number", node->line, node->column);
    }

    std::string notNullVar = extractNotNullVar(condNode); // x != nulo
    std::string nullVar    = extractNullVar(condNode);    // x == nulo

    symbolTable.enterScope();

    if (!notNullVar.empty()) {
        SymbolInfo* outerInfo = symbolTable.lookup(notNullVar);
        if (outerInfo && outerInfo->isNullable) {
            SymbolInfo narrowedInfo = *outerInfo;
            narrowedInfo.isNullable = false;
            symbolTable.declare(notNullVar, narrowedInfo);
        }
    }

    if (thenNode->type == NodeType::BLOCK) {
        for (const auto& child : thenNode->children) {
            visit(child.get());
        }
    } else {
        visit(thenNode);
    }

    symbolTable.exitScope();

    if (elseNode) {
        symbolTable.enterScope();

        if (!nullVar.empty()) {
            SymbolInfo* outerInfo = symbolTable.lookup(nullVar);
            if (outerInfo && outerInfo->isNullable) {
                SymbolInfo narrowedInfo = *outerInfo;
                narrowedInfo.isNullable = false;
                symbolTable.declare(nullVar, narrowedInfo);
            }
        }

        if (elseNode->type == NodeType::BLOCK) {
            for (const auto& child : elseNode->children) {
                visit(child.get());
            }
        } else {
            visit(elseNode);
        }

        symbolTable.exitScope();
    }


    if (!nullVar.empty() && hasEndingReturn(thenNode)) {
        SymbolInfo* currentInfo = symbolTable.lookup(nullVar);
        if (currentInfo && currentInfo->isNullable) {
            currentInfo->isNullable = false;
        }
    }
}

void SemanticAnalyzer::visitWhile(const ASTNode* node) {
    isWhile = 1;
    const ASTNode* condNode = node->children[0].get();
    const ASTNode* bodyNode = node->children[1].get();

    NodeType condType = inferType(condNode);
    bool isCondNullable = isExprNullable(condNode);

    if (!isCondNullable && condType != NodeType::BOOLEAN && condType != NodeType::NUMBER &&
        condType != NodeType::NUMBER_DOUBLE && condType != NodeType::NUMBER_FLOAT) {
        expect("Compile Error: While loop condition must be of boolean, numeric, or nullable type", node->line, node->column);
        }

    std::string notNullVar = extractNotNullVar(condNode);

    symbolTable.enterScope();

    if (!notNullVar.empty()) {
        SymbolInfo* outerInfo = symbolTable.lookup(notNullVar);
        if (outerInfo && outerInfo->isNullable) {
            SymbolInfo narrowedInfo = *outerInfo;
            narrowedInfo.isNullable = false;
            symbolTable.declare(notNullVar, narrowedInfo);
        }
    }

    loopDepth++;
    if (bodyNode->type == NodeType::BLOCK) {
        for (const auto& child : bodyNode->children) {
            visit(child.get());
        }
    } else {
        visit(bodyNode);
    }
    loopDepth--;

    symbolTable.exitScope();
    isWhile = 0;
}

bool SemanticAnalyzer::hasEndingReturn(const ASTNode* bodyNode) const {
    if (bodyNode && bodyNode->type == NodeType::BLOCK && !bodyNode->children.empty()) {
        return bodyNode->children.back()->type == NodeType::RETURN_STATEMENT;
    }
    return false;
}

