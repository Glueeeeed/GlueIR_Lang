#include "ast.h"
#include <iostream>

void AST::startProgramTree() {
    root = std::make_unique<ASTNode>(NodeType::PROGRAM);
}

std::unique_ptr<ASTNode> AST::makeString(const std::string& value, int line, int column) {
    return std::make_unique<ASTNode>(NodeType::STRING, value, false, false, false, line, column);
}

std::unique_ptr<ASTNode> AST::makeNumber(const std::string& value1, int line, int column) {
    return std::make_unique<ASTNode>(NodeType::NUMBER, value1, false, false, false, line, column);
}

std::unique_ptr<ASTNode> AST::makeDouble(const std::string& value1, int line, int column) {
    return std::make_unique<ASTNode>(NodeType::NUMBER_DOUBLE, value1, false, false, false, line, column);
}

std::unique_ptr<ASTNode> AST::makeBool(const std::string& value1, int line, int column) {
    return std::make_unique<ASTNode>(NodeType::BOOLEAN, value1, false, false, false, line, column);
}
std::unique_ptr<ASTNode> AST::makeFloat(const std::string& value1, int line, int column) {
    return std::make_unique<ASTNode>(NodeType::NUMBER_FLOAT, value1, false, false, false, line, column);
}

std::unique_ptr<ASTNode> AST::makeIdentifier(const std::string& name, int line, int column) {
    return std::make_unique<ASTNode>(NodeType::IDENTIFIER, name, false, false, false, line, column);
}


std::unique_ptr<ASTNode> AST::makeDeclaration(const std::string& name, bool isConst, bool stickyUsed, bool isSticky, int line, int column ) {
    return std::make_unique<ASTNode>(NodeType::IDENTIFIER, name, isConst, isSticky, stickyUsed, line, column);
}

std::unique_ptr<ASTNode> AST::makeBinaryOp(const std::string& op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right, int line, int column) {
    auto node = std::make_unique<ASTNode>(NodeType::BINARY_OPERATION, op, false, false, false, line, column);
    node->children.push_back(std::move(left));
    node->children.push_back(std::move(right));
    return node;
}

std::unique_ptr<ASTNode> AST::makeType(const std::string &name, int line, int column) {
    return std::make_unique<ASTNode>(NodeType::TYPE, name, false, false, false, line, column);
}

std::unique_ptr<ASTNode> AST::makeParameter(const std::string& name, const std::string& type) {
    auto pDecl = std::make_unique<ASTNode>(NodeType::PARAMETER, name);
    pDecl->children.push_back(makeIdentifier(name));
    pDecl->children.push_back(makeType(type));
    return pDecl;
}


void AST::addChild(std::unique_ptr<ASTNode> child) {
    root->children.push_back(std::move(child));
}

void AST::addAssignment(const std::string& var, std::unique_ptr<ASTNode> expr, int line, int column) {
    auto assign = std::make_unique<ASTNode>(NodeType::ASSIGNMENT, "", false, false, false, line, column);
    assign->children.push_back(makeIdentifier(var, line, column));
    assign->children.push_back(std::move(expr));
    addChild(std::move(assign));
}

void AST::addDeclaration(const std::string& var, std::unique_ptr<ASTNode> expr, std::string type, bool isConst, bool stickyUsed, bool isSticky, int line, int column) {
    auto assign = std::make_unique<ASTNode>(NodeType::DECLARATION, "", false, false, false, line, column);
    assign->children.push_back(makeDeclaration(var, isConst,  stickyUsed,  isSticky, line, column));
    assign->children.push_back(makeType(type, line, column));
    assign->children.push_back(std::move(expr));
    addChild(std::move(assign));
}

void AST::addFunctionDefinition(std::string name, std::string returnType, std::unique_ptr<ASTNode> params, std::unique_ptr<ASTNode> body) {
    auto func = std::make_unique<ASTNode>(NodeType::FUNCTION_DECLARATION, name);
    func->children.push_back(makeType(returnType));
    func->children.push_back(std::move(params));
    func->children.push_back(std::move(body));
    addChild(std::move(func));
}

void AST::addFunctionCall(std::string &name, std::vector<std::unique_ptr<ASTNode>> args) {
    auto call = std::make_unique<ASTNode>(NodeType::FUNCTION_CALL, name);
    for (auto &arg : args) {
        call->children.push_back(std::move(arg));
    }
    addChild(std::move(call));
}

void AST::addFunctionArgument(std::unique_ptr<ASTNode> arg) {
    args.push_back(std::move(arg));
}


// Supporting method

void AST::printAST(const ASTNode* node, int indent) {
    for (int i = 0; i < indent; i++) std::cout << "  ";

    std::cout << "[";

    switch (node->type) {
        case NodeType::PROGRAM: std::cout << "PROGRAM"; break;
        case NodeType::EXPRESSION: std::cout << "EXPRESSION"; break;
        case NodeType::ASSIGNMENT: std::cout << "ASSIGNMENT"; break;
        case NodeType::FUNCTION_CALL: std::cout << "FUNCTION_CALL"; break;
        case NodeType::FUNCTION_DECLARATION: std::cout << "FUNCTION_DECLARATION"; break;
        case NodeType::BLOCK: std::cout << "BLOCK"; break;
        case NodeType::WHILE_STATEMENT: std::cout << "WHILE_STATEMENT"; break;
        case NodeType::RETURN_STATEMENT: std::cout << "RETURN_STATEMENT"; break;
        case NodeType::IDENTIFIER: std::cout << "IDENTIFIER"; break;
        case NodeType::DECLARATION: std::cout << "DECLARATION"; break;
        case NodeType::STRING: std::cout << "STRING"; break;
        case NodeType::TYPE: std::cout << "TYPE"; break;
        case NodeType::NUMBER: std::cout << "NUMBER"; break;
        case NodeType::BOOLEAN: std::cout << "BOOLEAN"; break;
        case NodeType::NUMBER_DOUBLE: std::cout << "NUMBER_DOUBLE"; break;
        case NodeType::NUMBER_FLOAT: std::cout << "NUMBER_FLOAT"; break;
        case NodeType::BINARY_OPERATION: std::cout << "BINARY_OPERATION"; break;
        case NodeType::IF_STATEMENT: std::cout << "IF_STATEMENT"; break;
        case NodeType::BOND: std::cout << "BOND"; break;
    }

    if (!node->value.empty())
        std::cout << ": " << node->value;

    std::cout << "]" << std::endl;

    for (const auto& child : node->children) {
        printAST(child.get(), indent + 1);
    }
}


