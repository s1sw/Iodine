#include "parser.hpp"
#include <iostream>
#include <unordered_map>
#include <stdexcept>

namespace iodine {
    std::unordered_map<char, TokenType> singleCharTokens = {
        { ';', TokenType::Semicolon },
        { '(', TokenType::OpenParenthesis },
        { ')', TokenType::CloseParenthesis },
        { ',', TokenType::Comma },
        { '"', TokenType::DoubleQuote },
        { '+', TokenType::Operator },
        { '-', TokenType::Operator }
    };

    bool isStrNumber(const std::string& str) {
        for (const auto& c : str) {
            if (!std::isdigit(c))
                return false;
        }

        return true;
    }

    std::vector<Token> parseTokens(std::string str) {
        std::vector<Token> tokens;
        std::string currentText;

        for (auto charIt = str.begin(); charIt < str.end(); charIt++) {
            if (charIt < str.end() - 1) {
                if (*charIt == '/' && *(charIt + 1) == '*') {
                    // parsing a comment, keep going until we reach the close tag
                    // or the end of a string (in which case we throw an error)
                    bool reachedEnd = false;

                    while (charIt < str.end() - 1) {
                        if (*charIt == '*' && *(charIt + 1) == '/') {
                            reachedEnd = true;
                            charIt++;
                            break;
                        }

                        charIt++;
                    }

                    if (!reachedEnd)
                        throw std::runtime_error("EOF while parsing comment");

                    continue;
                }
            }

            auto pair = singleCharTokens.find(*charIt);

            if (pair != singleCharTokens.end()) {
                if (!currentText.empty()) {
                    Token newToken;
                    newToken.type = TokenType::Name;
                    newToken.val = currentText;
                    tokens.push_back(newToken);
                    currentText.clear();
                }

                Token newToken;
                newToken.val = *charIt;
                newToken.type = pair->second;
                tokens.push_back(newToken);
                continue;
            }

            if (*charIt == ' ' || *charIt == '\n') {
                if (!currentText.empty()) {
                    
                    Token newToken;
                    newToken.type = isStrNumber(currentText) ? TokenType::Number : TokenType::Name;
                    if (isStrNumber(currentText)) {
                        newToken.numberVal = std::atoi(currentText.c_str());
                    }
                    newToken.val = currentText;
                    currentText.clear();
                    tokens.push_back(newToken);
                }
                continue;
            }

            currentText += *charIt;
        }

        if (!currentText.empty()) {
            Token newToken;
            newToken.type = isStrNumber(currentText) ? TokenType::Number : TokenType::Name;
            newToken.val = currentText;
        if (isStrNumber(currentText)) {
                newToken.numberVal = std::atoi(currentText.c_str());
            }
            currentText.clear();
            tokens.push_back(newToken);
        }

        return tokens;
    }
}
