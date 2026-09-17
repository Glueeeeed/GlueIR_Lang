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
            NodeType retType = inferType(node->children[0].get());
            if (!isCompatible(currentFunctionReturnType, retType)) {
                expect("Compile Error: Function '" + currentFunctionName + "' with return type '" + currentFunctionReturnType + "' cannot return value of type " + nodeTypeToString(retType), node->line, node->column);
            }
        }
    }
}

void SemanticAnalyzer::visitIf(const ASTNode* node) {
    NodeType condType = inferType(node->children[0].get());
    if (condType != NodeType::BOOLEAN && condType != NodeType::NUMBER && condType != NodeType::NUMBER_DOUBLE && condType != NodeType::NUMBER_FLOAT) {
        expect("Compile Error: If statement condition must be of type boolean or number", node->line, node->column);
    }

    visit(node->children[1].get());

    if (node->children.size() > 2) {
        visit(node->children[2].get());
    }
}

void SemanticAnalyzer::visitWhile(const ASTNode* node) {
    NodeType condType = inferType(node->children[0].get());
    if (condType != NodeType::BOOLEAN && condType != NodeType::NUMBER && condType != NodeType::NUMBER_DOUBLE && condType != NodeType::NUMBER_FLOAT) {
        expect("Compile Error: While loop condition must be of type boolean or number", node->line, node->column);
    }

    visit(node->children[1].get());
}

bool SemanticAnalyzer::hasEndingReturn(const ASTNode* bodyNode) const {
    if (bodyNode && bodyNode->type == NodeType::BLOCK && !bodyNode->children.empty()) {
        return bodyNode->children.back()->type == NodeType::RETURN_STATEMENT;
    }
    return false;
}

