
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>
#include "lexer.h"
#include "../parser/parser.h"



std::vector<Token> lexer(const char* input) {

    std::vector<Token> tokens;
    std::ifstream file(input);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::size_t i = 0;
    std::size_t len = content.length();
    int line = 1;
    int column = 1;


    while (i < len) {
        char c = content[i];
        if (c == '\n') {
            line++;
            column = 1;
            i++;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            column++;
            continue;
        }

        if (i + 1 < len && content[i] == '/' && content[i+1] == '/') {
            i += 2;
            while (i < len && content[i] != '\n') {
                i++;
                column++;
            }
            if (i < len && content[i] == '\n') {
                i++;
                line++;
                column = 1;
            }
            continue;
        }

        if (i + 1 < len && content[i] == '/' && content[i+1] == '*' && content[i+2] == 'g') {
            i += 2;
            column += 2;
            while (i + 1 < len && !(content[i] == 'g' && content[i+1] == '*' && content[i+2] == '/')) {
                if (content[i] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                i++;
            }

            if (i + 1 < len && content[i] == 'g' && content[i+1] == '*' && content[i+2] == '/') {
                i += 3;
                column += 2;
            } else {
                std::cerr << "Error: Unclosed block comment starting on line " << line << "\n";
            }
            continue;
        }


        /*
         *  LATER!!
         *
         * TODO: add += *= /= -=
         * TODO: add increment and decrement token
         *
        */



        if (c == '.') {
            int start_column = column;
            tokens.push_back({TokenType::DOT, std::string(1, '.'), line, start_column});
            i++; column++;
            continue;
        }

        if (c == '=') {
            int start_column = column;
            if (i + 1 < len && content[i + 1] == '=') {
                tokens.push_back({TokenType::EQUALS_EQUALS, "==", line, start_column});
                i += 2;
                column += 2;
            } else {
                tokens.push_back({TokenType::EQUALS, "=", line, start_column});
                i++;
                column++;
            }
            continue;
        }

        if (c == '>') {
            int start_column = column;
            if (i + 1 < len && content[i + 1] == '=') {
                tokens.push_back({TokenType::GREATER_EQUALS, ">=", line, start_column});
                i += 2;
                column += 2;
            } else {
                tokens.push_back({TokenType::GREATER, ">", line, start_column});
                i++;
                column++;
            }
            continue;
        }

        if (c == '!') {
            int start_column = column;
            if (i + 1 < len && content[i + 1] == '=') {
                tokens.push_back({TokenType::NOT_EQUALS, "!=", line, start_column});
                i += 2;
                column += 2;
            } else {
                tokens.push_back({TokenType::NOT, "!", line, start_column});
                i++;
            }
            continue;
        }

        if (c == '|' && i + 1 < len && content[i + 1] == '|') {
            int start_column = column;
            tokens.push_back({TokenType::OR, "||", line, start_column});
            i += 2; column += 2;
            continue;
        }

        if (c == '&' && i + 1 < len && content[i + 1] == '&') {
            int start_column = column;
            tokens.push_back({TokenType::AND, "&&", line, start_column});
            i += 2; column += 2;
            continue;
        }

        if (c == '<') {
            int start_column = column;
            if (i + 1 < len && content[i + 1] == '=') {
                tokens.push_back({TokenType::LESS_EQUALS, "<=", line, start_column});
                i += 2; column += 2;
            } else {
                tokens.push_back({TokenType::LESS, "<", line, start_column});
                i++; column++;
            }
            continue;
        }

        if (c == '(') {
            int start_column = column;
            tokens.push_back({TokenType::LPAREN, std::string(1, '('), line, start_column});
            i++; column++;
            continue;
        }

        if (c == ')') {
            int start_column = column;
            tokens.push_back({TokenType::RPAREN, std::string(1, ')'), line, start_column});
            i++; column++;
            continue;
        }


        if (c == '{') {
            int start_column = column;
            tokens.push_back({TokenType::LBRACE, std::string(1, '{'), line, start_column});
            i++; column++;
            continue;
        }

        if (c == '}') {
            int start_column = column;
            tokens.push_back({TokenType::RBRACE, std::string(1, '}'), line, start_column});
            i++; column++;
            continue;
        }



        if (c == ';') {
            int start_column = column;
            tokens.push_back({TokenType::SEMICOLON, std::string(1, ';'), line, start_column});
            i++; column++;
            continue;
        }

        if (c == '+') {
            int start_column = column;
            tokens.push_back({TokenType::PLUS, std::string(1, '+'), line, start_column});
            i++; column++;
            continue;
        }

        if (c == '-') {
            int start_column = column;
            tokens.push_back({TokenType::MINUS, std::string(1, '-'), line, start_column});
            i++; column++;
            continue;
        }

        if (c == '*') {
            int start_column = column;
            tokens.push_back({TokenType::MULTIPLY, std::string(1, '*'), line, start_column});
            i++; column++;
            continue;
        }

        if (c == '%') {
            int start_column = column;
            tokens.push_back({TokenType::MOD, std::string(1, '%'), line, start_column});
            i++; column++;
            continue;
        }

        if (c == ',') {
            int start_column = column;
            tokens.push_back({TokenType::COMMA, std::string(1, ','), line, start_column});
            i++; column++;
            continue;
        }

        if (c == '/') {
            int start_column = column;
            tokens.push_back({TokenType::DIVISION, std::string(1, '/'), line, start_column});
            i++; column++;
            continue;
        }


        if (c == '\"') {
            std::string word;
            int start_column = column;
            i++; column++;
            while (i < len && content[i] != '\"') {
                word += content[i++];
                column++;
            }
            if (i < len && content[i] == '\"') {
                i++; column++;
            } else {
                std::cerr << "Unterminated string starting at " << line << ":" << start_column << '\n';
            }
            tokens.push_back({TokenType::STRING, word, line, start_column});
            continue;
        }


        if (std::isdigit(static_cast<unsigned char>(c))) {
            int start_column = column;
            std::string num;

            while (i < len && std::isdigit(static_cast<unsigned char>(content[i]))) {
                num += content[i++];
                column++;
            }

            bool hasDot = false;

            if (i < len && content[i] == '.' && (i + 1 < len) && std::isdigit(static_cast<unsigned char>(content[i + 1]))) {
                hasDot = true;
                num += content[i++];
                column++;
                while (i < len && std::isdigit(static_cast<unsigned char>(content[i]))) {
                    num += content[i++];
                    column++;
                }
            }

            bool isFloat = false;
            if (i < len && content[i] == 'f') {
                isFloat = true;
                i++; column++;
            }

            if (isFloat) {
                tokens.push_back({TokenType::NUMBER_FLOAT, num, line, start_column});
            } else if (hasDot) {
                tokens.push_back({TokenType::NUMBER_DOUBLE, num, line, start_column});
            } else {
                tokens.push_back({TokenType::NUMBER, num, line, start_column});
            }
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::string word;
            int start_column = column;
            while (i < len && (std::isalnum(static_cast<unsigned char>(content[i])) || content[i] == '_')) {
                word += content[i++];
                column++;
            }

            auto it = KEYWORDS.find(word);
            if (it != KEYWORDS.end()) {
                tokens.push_back({it->second, word, line, start_column});
            } else if (word == "true") {
                tokens.push_back({TokenType::BOOLEAN_TRUE, word, line, start_column});
            } else if (word == "false") {
                tokens.push_back({TokenType::BOOLEAN_FALSE, word, line, start_column});
            } else {
                tokens.push_back({TokenType::IDENTIFIER, word, line, start_column});
            }

            continue;
        }
        std::string word(1,c);
        tokens.push_back({TokenType::UNKNOWN, word, line, column});

        i++; column++;
    }

    tokens.push_back({TokenType::END_OF_FILE, std::string(1, ' '), line, column});

    return tokens;
}
