#ifndef GLUESCRIPTCOMPILER_AST_H
#define GLUESCRIPTCOMPILER_AST_H

#include <memory>
#include <vector>
#include <string>


enum class NodeType {
    PROGRAM,
    EXPRESSION,
    ASSIGNMENT,
    DECLARATION,
    FUNCTION_CALL,
    FUNCTION_DECLARATION,
    BLOCK,
    RETURN_STATEMENT,
    IF_STATEMENT,
    PARAMETER,
    WHILE_STATEMENT,
    IDENTIFIER,
    STRING,
    NUMBER,
    NUMBER_DOUBLE,
    NUMBER_FLOAT,
    BOND,
    BOOLEAN,
    TYPE,
    BINARY_OPERATION
};

struct ASTNode {
    NodeType type;
    std::string value;
    bool isConst;
    bool isSticky;
    bool stickyUsed;
    int line;
    int column;
    std::vector<std::unique_ptr<ASTNode>> children;

    explicit ASTNode(NodeType t, std::string v = "", bool isconst = false, bool IsSticky = false, bool StickyUsed = false, int l = 0, int c = 0)
        : type(t), isConst(isconst) , isSticky(IsSticky), stickyUsed(StickyUsed) , value(std::move(v)), line(l), column(c) {}


};

class AST {
    std::unique_ptr<ASTNode> root;

 public:
    void startProgramTree();
    std::vector<std::unique_ptr<ASTNode>> args;
    std::unique_ptr<ASTNode> makeString(const std::string& value, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeNumber(const std::string& value1, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeDouble(const std::string& value1, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeFloat(const std::string& value1, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeBool(const std::string& name, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeAny(const std::string& name, int line = 0, int column = 0);


    std::unique_ptr<ASTNode> makeParameter(const std::string& name, const std::string& type);
    std::unique_ptr<ASTNode> makeIdentifier(const std::string& name, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeDeclaration(const std::string& name,  bool isConst, bool stickyUsed, bool isSticky, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeType(const std::string& name, int line = 0, int column = 0);
    std::unique_ptr<ASTNode> makeBinaryOp(const std::string& op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right, int line = 0, int column = 0);
    void addChild(std::unique_ptr<ASTNode> child);
    void addAssignment(const std::string& var, std::unique_ptr<ASTNode> expr, int line = 0, int column = 0);
    void addDeclaration(const std::string& var, std::unique_ptr<ASTNode> expr, std::string type,  bool isConst = false, bool stickyUsed = false, bool isSticky = false, int line = 0, int column = 0);
    void addFunctionDefinition(std::string name, std::string returnType, std::unique_ptr<ASTNode> params, std::unique_ptr<ASTNode> body);
    void addFunctionArgument(std::unique_ptr<ASTNode> arg);


    void addFunctionCall(std::string &name, std::vector<std::unique_ptr<ASTNode>> args);

    const ASTNode* getRoot() const {
        return root.get();
    }

    void printAST(const ASTNode* node, int indent = 0);
    void testAST(const ASTNode* node);


};



#endif //GLUESCRIPTCOMPILER_AST_H