#include "parser.hpp"
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace iodine {
    std::unordered_map<char, TokenType> singleCharTokens = {
        { ';', TokenType::Semicolon },
        { '(', TokenType::OpenParenthesis },
        { ')', TokenType::CloseParenthesis },
        { ',', TokenType::Comma },
        { '"', TokenType::DoubleQuote },
        { '+', TokenType::Operator },
        { '-', TokenType::Operator },
        { '*', TokenType::Operator },
        { '/', TokenType::Operator },
        { '=', TokenType::Equals },
        { '{', TokenType::OpenBrace },
        { '}', TokenType::CloseBrace },
        { '\n', TokenType::NewLine }
    };

    std::unordered_map<std::string, TokenType> keywords = {
        { "if", TokenType::If },
        { "true", TokenType::True },
        { "false", TokenType::False }
    };

    bool isStrNumber(const std::string& str) {
        for (const auto& c : str) {
            if (!std::isdigit(c))
                return false;
        }

        return true;
    }

    bool isStrDecimalNumber(const std::string& str) {
        bool foundDecimalPoint = false;
        for (const auto& c : str) {
            if (!std::isdigit(c)) {
                if (c == '.' && !foundDecimalPoint) {
                    foundDecimalPoint = true;
                } else {
                    return false;
                }
            }
        }

        return foundDecimalPoint;
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

                if (*charIt == '/' && *(charIt + 1) == '/') {
                    while (charIt < str.end() - 1) {
                        if (*charIt == '\n') {
                            charIt++;
                            break;
                        }

                        charIt++;
                    }

                    continue;
                }
            }

            auto pair = singleCharTokens.find(*charIt);

            if (pair != singleCharTokens.end()) {
                if (pair->second == TokenType::DoubleQuote) {
                    std::cout << "parsing str1: charit " << *charIt << "\n";
                    bool reachedEnd = false;
                    std::string parsedStr = "";
                    charIt++;
                    while (charIt < str.end()) {
                        if (*charIt == '"') {
                            reachedEnd = true;
                            break;
                        }
                        parsedStr += *charIt;
                        charIt++;
                    } 

                    if (!reachedEnd)
                        throw std::runtime_error("EOF while parsing string");

                    Token t;
                    t.type = TokenType::StringContents;
                    t.val = parsedStr;
                    tokens.push_back(t);
                    continue;
                }

                if (!currentText.empty()) {
                    auto keywordIt = keywords.find(currentText);
                    Token newToken;
                    newToken.type = isStrNumber(currentText) ? TokenType::Number : TokenType::Name;

                    if (keywordIt != keywords.end()) {
                        newToken.type = keywordIt->second;
                    }

                    if (isStrNumber(currentText)) {
                        newToken.numberVal = std::atoi(currentText.c_str());
                    }

                    if (isStrDecimalNumber(currentText)) {
                        newToken.type = TokenType::DecimalNumber;
                    }
                    newToken.val = currentText;
                    currentText.clear();
                    tokens.push_back(newToken);
                }

                Token newToken;
                newToken.val = *charIt;
                newToken.type = pair->second;
                tokens.push_back(newToken);
                continue;
            }

            if (*charIt == ' ' || *charIt == ';') {
                if (!currentText.empty()) {
                    auto keywordIt = keywords.find(currentText);
                    Token newToken;
                    newToken.type = isStrNumber(currentText) ? TokenType::Number : TokenType::Name;

                    if (keywordIt != keywords.end()) {
                        newToken.type = keywordIt->second;
                    }

                    if (isStrNumber(currentText)) {
                        newToken.numberVal = std::atoi(currentText.c_str());
                    }

                    if (isStrDecimalNumber(currentText)) {
                        newToken.type = TokenType::DecimalNumber;
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

            if (isStrDecimalNumber(currentText)) {
                newToken.type = TokenType::DecimalNumber;
            }

            currentText.clear();
            tokens.push_back(newToken);
        }

        return tokens;
    }
}
