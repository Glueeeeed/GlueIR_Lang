#ifndef LEXER_H
#define LEXER_H
#include <string>
#include <unordered_map>
#include <vector>

enum struct TokenType {
    TYPE,KEYWORD,IDENTIFIER, GLUE,

    EQUALS, EQUALS_EQUALS, GREATER, GREATER_EQUALS, LESS, LESS_EQUALS, NEGATIVE, NEGATIVE_EQUALS,
    NOT_EQUALS, NOT, AND, OR, MOD,

    LPAREN, RPAREN, LBRACE, RBRACE,

    DOT,SEMICOLON, COMMA, COLON,

    STRING, NUMBER, NUMBER_FLOAT, NUMBER_DOUBLE, BOOLEAN_TRUE, BOOLEAN_FALSE,

    PLUS, MINUS, MULTIPLY,DIVISION,

    END_OF_FILE,
    UNKNOWN
};



struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};




std::vector<Token> lexer(const char* input);

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"fin", TokenType::KEYWORD},
    {"sigue", TokenType::KEYWORD},
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"shout", TokenType::KEYWORD},
    {"shin", TokenType::KEYWORD},
    {"func", TokenType::KEYWORD},
    {"const", TokenType::KEYWORD},
    {"sticky", TokenType::KEYWORD},
    {"if", TokenType::KEYWORD},
    {"else", TokenType::KEYWORD},
    {"while", TokenType::KEYWORD},
    {"break", TokenType::KEYWORD},
    {"continue", TokenType::KEYWORD},
    {"return", TokenType::KEYWORD},
    {"true", TokenType::BOOLEAN_TRUE},
    {"false", TokenType::BOOLEAN_FALSE},
    {"string", TokenType::TYPE},
    {"bool", TokenType::TYPE},
    {"boolean", TokenType::TYPE},
    {"float", TokenType::TYPE},
    {"double", TokenType::TYPE},
    // {"tiny", TokenType::TYPE}, // TODO: move to std library
    // {"small", TokenType::TYPE}, // TODO: move to std library
    {"int", TokenType::TYPE},
    // {"long", TokenType::TYPE}, // TODO: move to std library
    // {"huge", TokenType::TYPE}, // TODO: move to std library
};

#endif