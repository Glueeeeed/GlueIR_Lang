#include "../lexer/lexer.h"
#include "parser.h"
#include "../AST/ast.h"
#include <complex>
#include <functional>
#include <functional>
#include <iostream>

#include "../semantic/semantic.h"

void Parser::parse() {
    if (tokenPos == 0) {
        ast.startProgramTree();
    }

    while (currentToken().type != TokenType::END_OF_FILE) {
        Token cToken = currentToken();
        Token nToken = peekToken();

        if (cToken.type == TokenType::KEYWORD && cToken.value == "func") {
            parseFunctionDeclaration();
        } else if (cToken.type == TokenType::UNKNOWN) {
            expect("Syntax Error: unexpected token '" + cToken.value + "'", cToken.line, cToken.column);
        } else {
            expect("Syntax Error: Only function definitions allowed at global scope. Found: '" + cToken.value + "'", cToken.line, cToken.column);
        }
    }
}


void Parser::parseFunctionDeclaration() {

    nextToken();
    std::pair<std::string, std::string> funcHeader = parseFunctionHeader();
    std::string returnType = funcHeader.first;
    std::string funcName = funcHeader.second;

    funcName = funcName == "Main" ? "main" : funcName;
    nextToken();

    consume(TokenType::LPAREN, "expected '(' after function name");

    std::vector<std::pair<std::string, std::string>> params = parse_args();

    consume(TokenType::RPAREN, "expected ')' after function arguments");

    consume(TokenType::LBRACE, "expected '{' to start function body");

    auto body = std::make_unique<ASTNode>(NodeType::BLOCK);

    while (currentToken().type != TokenType::RBRACE && currentToken().type != TokenType::END_OF_FILE) {
        parseStatement(body.get());
    }
    consume(TokenType::RBRACE, "expected '}' at the end of function");

    auto paramsNode = std::make_unique<ASTNode>(NodeType::BLOCK);
    for (const auto& [pType, pName] : params) {
        paramsNode->children.push_back(ast.makeParameter(pName, pType));
    }

    ast.addFunctionDefinition(funcName, returnType, std::move(paramsNode), std::move(body));
}



void Parser::parseStatement(ASTNode* parentBlock) {
    Token token = currentToken();

    if (token.type == TokenType::LBRACE) {
        parseBlockStatement(parentBlock);
    } else if (token.type == TokenType::KEYWORD && token.value == "return") {
        parseReturnStatement(parentBlock, token);
    } else if (token.type == TokenType::TYPE || (token.type == TokenType::KEYWORD && (token.value == "const" || token.value == "sticky"))) {
        parseDeclarationStatement(parentBlock);
    } else if (token.type == TokenType::IDENTIFIER) {
        parseIdentifierStatement(parentBlock, token);
    } else if (token.type == TokenType::KEYWORD && token.value == "if") {
        parseIfStatement(parentBlock, token);
    } else if (token.type == TokenType::KEYWORD && token.value == "while") {
        parseWhileStatement(parentBlock, token);
    } else if (token.type == TokenType::KEYWORD && (token.value == "shout" || token.value == "Shout" || token.value == "shin")) {
        parseCallStatement(parentBlock);
    } else {
        expect("Syntax Error: unknown statement '" + token.value + "'", token.line, token.column);
    }
}

std::unique_ptr<ASTNode> Parser::parseLiteral() {
    Token token = currentToken();


    if (token.type == TokenType::IDENTIFIER || (token.type == TokenType::KEYWORD && peekToken().type == TokenType::LPAREN)) {
        if (peekToken().type == TokenType::LPAREN) {
            return parseFunctionCall();
        }
        auto node = ast.makeIdentifier(token.value, token.line, token.column);
        nextToken();
        return node;
    }

    if (token.type == TokenType::MINUS) {
        nextToken();
        auto operand = parseLiteral();
        auto zeroNode = ast.makeNumber("0", token.line, token.column);
        return ast.makeBinaryOp("-", std::move(zeroNode), std::move(operand), token.line, token.column);
    }

    if (token.type == TokenType::NOT) {
        nextToken();
        auto operand = parseLiteral();
        auto falseNode = ast.makeBool("false", token.line, token.column);
        return ast.makeBinaryOp("==", std::move(operand), std::move(falseNode), token.line, token.column);
    }

    if (token.type == TokenType::PLUS) {
        nextToken();
        return parseLiteral();
    }


    if (token.type == TokenType::NUMBER) {
        auto node = ast.makeNumber(token.value, token.line, token.column);
        nextToken();
        return node;
    }

    if (token.type == TokenType::NUMBER_DOUBLE) {
        auto node = ast.makeDouble(token.value, token.line, token.column);
        nextToken();
        return node;
    }

    if (token.type == TokenType::NUMBER_FLOAT) {
        auto node = ast.makeFloat(token.value, token.line, token.column);
        nextToken();
        return node;
    }

    if (token.type == TokenType::STRING) {
        auto node = ast.makeString(token.value, token.line, token.column);
        nextToken();
        return node;
    }

    if (token.type == TokenType::BOOLEAN_TRUE || token.type == TokenType::BOOLEAN_FALSE) {
        auto node = ast.makeBool(token.value, token.line, token.column);
        nextToken();
        return node;
    }

    if (token.type == TokenType::IDENTIFIER) {
        if (peekToken().type == TokenType::LPAREN) {
            return parseFunctionCall();
        }
        auto node = ast.makeIdentifier(token.value, token.line, token.column);
        nextToken();
        return node;
    }

    if (token.type == TokenType::LPAREN) {
        nextToken();
        auto node = parseExpression();
        if (currentToken().type != TokenType::RPAREN) {
            expect("Syntax Error: expected ')'", currentToken().line, currentToken().column);
        }
        nextToken();
        return node;
    }

    expect("Syntax Error: expected a literal or identifier", token.line, token.column);
    return nullptr;
}



std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseLogicalOr();
}

std::unique_ptr<ASTNode> Parser::parseAddition() {
    auto left = parseMultiplication();

    while (currentToken().type == TokenType::PLUS || currentToken().type == TokenType::MINUS) {
        Token opToken = currentToken();
        nextToken();

        auto right = parseMultiplication();
        left = ast.makeBinaryOp(opToken.value, std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (currentToken().type == TokenType::OR) {
        Token op = currentToken();
        nextToken();
        auto right = parseLogicalAnd();
        left = ast.makeBinaryOp("or", std::move(left), std::move(right), op.line, op.column);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd() {
    auto left = parseComparison();
    while (currentToken().type == TokenType::AND) {
        Token op = currentToken();
        nextToken();
        auto right = parseComparison();
        left = ast.makeBinaryOp("and", std::move(left), std::move(right), op.line, op.column);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseAddition();

    while (currentToken().type == TokenType::EQUALS_EQUALS || currentToken().type == TokenType::NOT_EQUALS ||currentToken().type == TokenType::GREATER ||currentToken().type == TokenType::GREATER_EQUALS ||currentToken().type == TokenType::LESS ||currentToken().type == TokenType::LESS_EQUALS) {
        Token opToken = currentToken();
        nextToken();

        auto right = parseAddition();
        left = ast.makeBinaryOp(opToken.value, std::move(left), std::move(right), opToken.line, opToken.column);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseMultiplication() {
    auto left = parseLiteral();

    while (currentToken().type == TokenType::MULTIPLY || currentToken().type == TokenType::DIVISION) {
        Token opToken = currentToken();
        nextToken();

        auto right = parseLiteral();
        left = ast.makeBinaryOp(opToken.value, std::move(left), std::move(right), opToken.line, opToken.column);
    }

    return left;
}


std::unique_ptr<ASTNode> Parser::parseAssign() {
    Token idToken = currentToken();
    std::string identifier = idToken.value;
    if (idToken.type != TokenType::IDENTIFIER) {
        expect("Syntax Error: expected an identifier before '" + idToken.value + "'", idToken.line, idToken.column);
    }
    nextToken();

    Token eqToken = currentToken();
    if (eqToken.type != TokenType::EQUALS) {
        expect("Syntax Error: expected '=' after identifier '" + identifier + "'", eqToken.line, eqToken.column);
    }
    nextToken();

    auto expr = parseExpression();

    if (currentToken().type != TokenType::SEMICOLON) {
        expect("Syntax Error: expected ';' after assignment to '" + identifier + "'", currentToken().line,
               currentToken().column);
    }
    nextToken();

    auto assign = std::make_unique<ASTNode>(NodeType::ASSIGNMENT, "", false, false, false, idToken.line, idToken.column);
    assign->children.push_back(ast.makeIdentifier(identifier, idToken.line, idToken.column));
    assign->children.push_back(std::move(expr));
    return assign;
}




std::unique_ptr<ASTNode> Parser::parseDeclaration() {
    bool isConstant = false;
    bool isSticky = false;
    bool stickyUsed = false;
    int declLine = currentToken().line;
    int declCol = currentToken().column;

    if (currentToken().type == TokenType::KEYWORD && currentToken().value == "const") {
        isConstant = true;
        nextToken();
    } else if (currentToken().type == TokenType::KEYWORD && currentToken().value == "sticky") {
        isSticky = true;
        nextToken();
    }

    Token typeToken = currentToken();
    std::string type = typeToken.value;
    if (typeToken.type != TokenType::TYPE) {
        expect("Syntax Error: expected a type name (e.g., int, string) in declaration", typeToken.line,
               typeToken.column);
    }
    nextToken();

    Token idToken = currentToken();
    std::string identifier = idToken.value;
    if (idToken.type != TokenType::IDENTIFIER) {
        expect("Syntax Error: expected an identifier after type '" + type + "'", idToken.line, idToken.column);
    }
    nextToken();

    if (currentToken().type != TokenType::EQUALS) {
        expect("Syntax Error: expected '=' after identifier '" + identifier + "'", currentToken().line,
               currentToken().column);
    }
    nextToken();

    auto expr = parseExpression();
    auto decl = std::make_unique<ASTNode>(NodeType::DECLARATION, "", isConstant, isSticky, stickyUsed, declLine, declCol);
    decl->children.push_back(ast.makeDeclaration(identifier, isConstant, stickyUsed, isSticky, idToken.line, idToken.column));
    decl->children.push_back(ast.makeType(type, typeToken.line, typeToken.column));
    decl->children.push_back(std::move(expr));
    return decl;
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall() {
    Token idToken = currentToken();
    std::string functionName = idToken.value;
    
    if (idToken.type != TokenType::KEYWORD && idToken.type != TokenType::IDENTIFIER) {
        expect("Syntax Error: expected a function name to start a function call", idToken.line,
               idToken.column);
    }
    nextToken();

    if (currentToken().type != TokenType::LPAREN) {
        expect("Syntax Error: expected '(' before function arguments", currentToken().line, currentToken().column);
    }
    nextToken();

    std::vector<std::unique_ptr<ASTNode>> args;
    if (currentToken().type != TokenType::RPAREN) {
        bool isEnd = false;
        while (!isEnd) {
            args.push_back(parseExpression());

            if (currentToken().type == TokenType::RPAREN) {
                isEnd = true;
            } else if (currentToken().type == TokenType::COMMA) {
                nextToken();
            } else {
                expect("Syntax Error: expected ',' or ')' between function arguments", currentToken().line,
                       currentToken().column);
            }
        }
    }
    
    if (currentToken().type != TokenType::RPAREN) {
        expect("Syntax Error: expected ')' after function arguments", currentToken().line, currentToken().column);
    }
    nextToken();


    auto call = std::make_unique<ASTNode>(NodeType::FUNCTION_CALL, functionName, false, false, false, idToken.line, idToken.column);
    for (auto &arg : args) {
        call->children.push_back(std::move(arg));
    }
    return call;
}

void Parser::parseArgument() {
    bool isEnd = false;
    while (!isEnd) {
        auto expr = parseExpression();
        ast.addFunctionArgument(std::move(expr));

        if (currentToken().type == TokenType::RPAREN) {
            nextToken();
            isEnd = true;
        } else {
            if (currentToken().type == TokenType::COMMA) {
                nextToken();
            } else {
                expect("Syntax Error: expected ',' or ')' between function arguments", currentToken().line,
                       currentToken().column);
            }
        }
    }

    std::string function = "shout";
    ast.addFunctionCall(function, std::move(ast.args));
}


Token Parser::currentToken() const {
    if (tokenPos >= tokens.size()) {
        throw std::out_of_range("Parser: no more tokens!");
    }

    return tokens[tokenPos];
}

Token Parser::peekToken() const {
    if (tokenPos >= tokens.size()) {
        throw std::out_of_range("Parser: no more tokens!");
    }

    return tokens[tokenPos + 1];
}

Token Parser::nextToken() {
    if (tokenPos >= tokens.size()) {
        throw std::out_of_range("Parser: no more tokens!");
    }

    ++tokenPos;
    return tokens[tokenPos];
}

void Parser::printASTCall() {
    // ast.printAST(ast.getRoot()); // FOR DEBUG
    semantic.analyse(ast.getRoot());
    codegen.generateCode(ast.getRoot());
}


void Parser::expect(std::string msg, int line, int column) {
    std::stringstream stream;
    stream << msg << " at " << line << ":" << column << std::endl;
    throw ParseError(stream.str());
}
