#ifndef PROXIMA_PARSER_H
#define PROXIMA_PARSER_H

#include "../lexer/Token.h"
#include "AST.h"
#include <vector>
#include <memory>
#include <string>

namespace proxima {

// Forward declarations for pointer types
class ASTNode;
class ExpressionNode;
class StatementNode;
class DeclarationNode;
class ProgramNode;

using ASTNodePtr = std::shared_ptr<ASTNode>;
using ExpressionNodePtr = std::shared_ptr<ExpressionNode>;
using StatementNodePtr = std::shared_ptr<StatementNode>;
using DeclarationNodePtr = std::shared_ptr<DeclarationNode>;
using ProgramNodePtr = std::shared_ptr<ProgramNode>;

/**
 * @brief Синтаксический анализатор языка Proxima
 *
 * Преобразует последовательность токенов в AST.
 */
class Parser {
public:
    /**
     * @brief Конструктор
     * @param tokens Вектор токенов
     * @param filename Имя файла (для сообщений об ошибках)
     */
    Parser(const std::vector<Token>& tokens, const std::string& filename = "<input>");

    /**
     * @brief Парсинг программы
     * @return Корневой узел AST
     */
    ProgramNodePtr parse();

private:
    std::vector<Token> tokens;    ///< Вектор токенов
    size_t pos;                    ///< Текущая позиция
    std::string filename;          ///< Имя файла

    /**
     * @brief Получение текущего токена
     * @return Текущий токен
     */
    Token currentToken() const;

    /**
     * @brief Получение следующего токена
     * @return Следующий токен
     */
    Token peekToken() const;

    /**
     * @brief Переход к следующему токену
     */
    void advance();

    /**
     * @brief Проверка и потребление токена
     * @param type Ожидаемый тип токена
     * @return true если токен совпал
     */
    bool match(TokenType type);

    /**
     * @brief Проверка типа текущего токена
     * @param type Ожидаемый тип токена
     * @return true если тип совпадает
     */
    bool check(TokenType type) const;

    /**
     * @brief Ожидание токена определённого типа
     * @param type Ожидаемый тип токена
     * @param message Сообщение об ошибке
     * @return Токен
     */
    Token expect(TokenType type, const std::string& message);

    // Парсинг программы
    ProgramNodePtr parseProgram();
    DeclarationNodePtr parseDeclaration();

    // Парсинг классов и интерфейсов
    DeclarationNodePtr parseClass();
    DeclarationNodePtr parseInterface();
    std::vector<std::pair<std::string, std::string>> parseClassMembers();
    std::vector<std::string> parseInheritanceList();

    // Парсинг statement'ов
    StatementNodePtr parseStatement();
    StatementNodePtr parseIf(bool isElseIf = false);
    StatementNodePtr parseFor();
    StatementNodePtr parseWhile();
    StatementNodePtr parseReturn();
    StatementNodePtr parseBlock();
    
    // Парсинг LLM директив
    StatementNodePtr parseIntentBlock();
    StatementNodePtr parseGeneratedBlock();
    StatementNodePtr parseFixedBlock();

    // Парсинг выражений
    ExpressionNodePtr parseExpression();
    ExpressionNodePtr parseAssignment();
    ExpressionNodePtr parseTernary();
    ExpressionNodePtr parseOr();
    ExpressionNodePtr parseAnd();
    ExpressionNodePtr parseEquality();
    ExpressionNodePtr parseComparison();
    ExpressionNodePtr parseTerm();
    ExpressionNodePtr parseFactor();
    ExpressionNodePtr parseUnary();
    ExpressionNodePtr parseCall();
    ExpressionNodePtr parsePrimary();

    // Вспомогательные методы
    std::vector<ExpressionNodePtr> parseArguments();
    std::vector<std::pair<std::string, std::string>> parseParameters();
};

} // namespace proxima

#endif // PROXIMA_PARSER_H
