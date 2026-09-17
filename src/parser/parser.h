#ifndef PARSER_H
#define PARSER_H
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include "../lexer/lexer.h"
#include "../AST/ast.h"
#include "../codegen/codegen.h"
#include "../semantic/semantic.h"

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

class Parser {
    std::vector<Token> tokens;
        std::vector<std::string> types = {"int", "string", "float", "double", "bool"};
        int tokenPos = 0;
        AST ast;
        SemanticAnalyzer semantic;
        CodeGenerator codegen;
    public:
       Parser(const std::vector<Token> & tokens_,llvm::LLVMContext &context ) : codegen(context) {
       this->tokens = tokens_;
    }

        Token currentToken() const;
        Token peekToken() const;
        Token nextToken();
        std::unique_ptr<ASTNode>  parseAssign();
        std::unique_ptr<ASTNode>  parseDeclaration();
        void parseArgument();
        void parseFunctionDeclaration();
        void parseStatement(ASTNode* parentBlock) ;
        std::unique_ptr<ASTNode>  parseExpression();
        std::unique_ptr<ASTNode>  parseComparison();
        std::unique_ptr<ASTNode>  parseLogicalOr();
        std::unique_ptr<ASTNode>  parseLogicalAnd();
        std::unique_ptr<ASTNode>  parseAddition();
        std::unique_ptr<ASTNode>  parseMultiplication();
        std::unique_ptr<ASTNode> parseLiteral();
        std::unique_ptr<ASTNode>  parseFunctionCall();
        void parse();

        // STATEMENTS

        void parseBlockStatement(ASTNode* parentBlock);
        void parseReturnStatement(ASTNode* parentBlock, const Token& token);
        void parseDeclarationStatement(ASTNode* parentBlock);
        void parseIdentifierStatement(ASTNode* parentBlock, const Token& token);
        void parseIfStatement(ASTNode* parentBlock, const Token& token);
        void parseWhileStatement(ASTNode* parentBlock, const Token& token);
        void parseCallStatement(ASTNode* parentBlock);










        // UTILS

        std::vector<std::pair<std::string, std::string>>  parse_args();
        void consume(TokenType expectedType, const std::string& errMsg);
        std::pair<std::string, std::string> parseFunctionHeader();
        static void expect(std::string msg,  int line = 0, int column = 0);
        void printASTCall();

};

#endif