#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <string>
#include <unordered_map>
#include "scope.h"
#include "../AST/ast.h"

class SemanticAnalyzer {
    Scope symbolTable;
    std::unordered_map<std::string, SymbolInfo> symbols;
    std::string currentFunctionReturnType;
    std::string currentFunctionName;
    int loopDepth = 0;


public:
    void analyse(const ASTNode* node);
    void visit(const ASTNode* node);
    void visitDeclaration(const ASTNode* node);
    void visitAssignment(const ASTNode* node);
    void visitFunctionDeclaration(const ASTNode* node);
    void visitFunctionCall(const ASTNode* node);
    NodeType inferType(const ASTNode* node);
    bool isCompatible(const std::string& declaredType, NodeType valueType);
    void declareSymbol(const ASTNode* node, bool isConst, bool stickyUsed, bool isSticky);
    static void expect(std::string msg, int line = 0, int column = 0);

    // utils

    std::string nodeTypeToString(NodeType t);
    NodeType mapStringToNodeType(std::string_view typeStr) const;
    bool hasEndingReturn(const ASTNode* bodyNode) const;
    void checkDivisionByZero(const ASTNode* rightChild, int line, int column);
    void visitProgram(const ASTNode* node);
    void visitReturn(const ASTNode* node);
    void visitIf(const ASTNode* node);
    void visitWhile(const ASTNode* node);

};

#endif // SEMANTIC_H