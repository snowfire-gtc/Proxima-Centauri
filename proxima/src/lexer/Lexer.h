#ifndef PROXIMA_LEXER_H
#define PROXIMA_LEXER_H

#include "Token.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace proxima {

/**
 * @brief Лексический анализатор языка Proxima
 *
 * Преобразует исходный код в последовательность токенов.
 */
class Lexer {
public:
    /**
     * @brief Конструктор
     * @param source Исходный код
     * @param filename Имя файла (для сообщений об ошибках)
     */
    Lexer(const std::string& source, const std::string& filename = "<input>");

    /**
     * @brief Токенизация всего исходного кода
     * @return Вектор токенов
     */
    std::vector<Token> tokenize();

    /**
     * @brief Получение следующего токена
     * @return Токен
     */
    Token nextToken();

    /**
     * @brief Получение текущей строки
     * @return Номер строки (1-based)
     */
    int getCurrentLine() const { return line; }

    /**
     * @brief Получение текущей колонки
     * @return Номер колонки (1-based)
     */
    int getCurrentColumn() const { return column; }

    /**
     * @brief Получение имени файла
     * @return Имя файла
     */
    std::string getFilename() const { return filename; }

private:
    std::string source;             ///< Исходный код
    std::string filename;           ///< Имя файла
    size_t pos;                     ///< Текущая позиция
    int line;                       ///< Текущая строка
    int column;                     ///< Текущая колонка
    int startLine;                  ///< Начальная строка токена
    int startColumn;                ///< Начальная колонка токена
    std::unordered_map<std::string, TokenType> keywords; ///< Ключевые слова

    /**
     * @brief Инициализация ключевых слов
     */
    void initKeywords();

    /**
     * @brief Получение текущего символа
     * @return Текущий символ
     */
    char currentChar() const;

    /**
     * @brief Получение следующего символа
     * @return Следующий символ
     */
    char peekChar() const;

    /**
     * @brief Переход к следующему символу
     */
    void advance();

    /**
     * @brief Пропуск пробельных символов
     */
    void skipWhitespace();

    /**
     * @brief Пропуск комментариев
     */
    void skipComment();

    /**
     * @brief Чтение числа
     * @return Токен числа
     */
    Token readNumber();

    /**
     * @brief Чтение идентификатора
     * @return Токен идентификатора
     */
    Token readIdentifier();

    /**
     * @brief Чтение строки
     * @return Токен строки
     */
    Token readString();

    /**
     * @brief Чтение символа
     * @return Токен символа
     */
    Token readChar();

    /**
     * @brief Чтение оператора
     * @return Токен оператора
     */
    Token readOperator();

    /**
     * @brief Создание токена
     * @param type Тип токена
     * @param value Значение токена
     * @return Токен
     */
    Token makeToken(TokenType type, const std::string& value);
};

} // namespace proxima

#endif // PROXIMA_LEXER_H
