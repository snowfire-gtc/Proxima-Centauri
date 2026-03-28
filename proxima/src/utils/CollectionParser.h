#ifndef PROXIMA_COLLECTION_PARSER_H
#define PROXIMA_COLLECTION_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Класс для парсинга и сериализации формата collection
 *
 * Формат collection согласно language.txt пункт 40, 53-54:
 * [
 *     "key1", "value1",,
 *     "key2", "value2",,
 *     "nested", [
 *         "key3", "value3",,
 *         "key4", "value4"
 *     ],,
 *     "array", ["item1", "item2", "item3"]
 * ]
 *
 * Согласно требованию пункт 54:
 * "Не использовать формат JSON - вместо него использовать тип collection"
 */
class CollectionParser {
public:
    /**
     * @brief Типы значений в collection
     */
    enum class ValueType {
        String,
        Number,
        Boolean,
        Null,
        Array,
        Object,
        Collection
    };

    /**
     * @brief Представление значения collection
     */
    struct Value {
        ValueType type;
        std::string stringValue;
        double numberValue;
        bool boolValue;
        std::vector<Value> arrayValue;
        std::map<std::string, Value> objectValue;

        Value() : type(ValueType::Null), numberValue(0.0), boolValue(false) {}

        //Индексация массивов начинается с 1.
        static Value evaluateArrayAccess(const Value& array, const std::vector<Value>& indices);

        // Конвертеры
        static Value fromString(const std::string& str);
        static Value fromNumber(double num);
        static Value fromBoolean(bool b);
        static Value fromArray(const std::vector<Value>& arr);
        static Value fromObject(const std::map<std::string, Value>& obj);
        static Value fromCollection(const std::string& collectionStr);
        static Value fromNull();

        // Сериализация
        std::string toString() const;
        std::string toCollectionString(int indent = 0) const;

        // Проверки типов
        bool isString() const { return type == ValueType::String; }
        bool isNumber() const { return type == ValueType::Number; }
        bool isBoolean() const { return type == ValueType::Boolean; }
        bool isNull() const { return type == ValueType::Null; }
        bool isArray() const { return type == ValueType::Array; }
        bool isObject() const { return type == ValueType::Object; }
        bool isCollection() const { return type == ValueType::Collection; }

        // Доступ к значениям
        std::string asString() const { return stringValue; }
        double asNumber() const { return numberValue; }
        bool asBoolean() const { return boolValue; }
        const std::vector<Value>& asArray() const { return arrayValue; }
        const std::map<std::string, Value>& asObject() const { return objectValue; }

        // Удобные методы доступа
        Value get(const std::string& key) const;
        Value get(size_t index) const;
        bool has(const std::string& key) const;
        size_t size() const;

        // Операторы
        bool operator==(const Value& other) const;
        bool operator!=(const Value& other) const { return !(*this == other); }
    };

    /**
     * @brief Результат парсинга
     */
    struct ParseResult {
        bool success;
        Value value;
        std::string error;
        int errorLine;
        int errorColumn;

        ParseResult() : success(false), errorLine(0), errorColumn(0) {}
    };

    /**
     * @brief Конструктор
     */
    CollectionParser();

    /**
     * @brief Деструктор
     */
    ~CollectionParser();

    // ========================================================================
    // Парсинг
    // ========================================================================

    /**
     * @brief Парсинг строки в формате collection
     * @param input Входная строка
     * @return Результат парсинга
     */
    ParseResult parse(const std::string& input);

    /**
     * @brief Парсинг файла в формате collection
     * @param filePath Путь к файлу
     * @return Результат парсинга
     */
    ParseResult parseFile(const std::string& filePath);

    /**
     * @brief Парсинг потока в формате collection
     * @param stream Входной поток
     * @return Результат парсинга
     */
    ParseResult parseStream(std::istream& stream);

    // ========================================================================
    // Сериализация
    // ========================================================================

    /**
     * @brief Сериализация Value в строку collection
     * @param value Значение для сериализации
     * @param indent Уровень отступа
     * @return Строка в формате collection
     */
    std::string serialize(const Value& value, int indent = 0);

    /**
     * @brief Сериализация Value в файл
     * @param value Значение для сериализации
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool serializeToFile(const Value& value, const std::string& filePath);

    // ========================================================================
    // Утилиты
    // ========================================================================

    /**
     * @brief Создание collection из пары ключ-значение
     * @param key Ключ
     * @param value Значение
     * @return Value в формате collection
     */
    static Value createPair(const std::string& key, const Value& value);

    /**
     * @brief Создание collection из списка пар
     * @param pairs Список пар ключ-значение
     * @return Value в формате collection
     */
    static Value createCollection(const std::vector<std::pair<std::string, Value>>& pairs);

    /**
     * @brief Создание массива из списка значений
     * @param values Список значений
     * @return Value в формате array
     */
    static Value createArray(const std::vector<Value>& values);

    /**
     * @brief Проверка валидности формата collection
     * @param input Входная строка
     * @return true если валидно
     */
    static bool isValid(const std::string& input);

    /**
     * @brief Минификация collection строки
     * @param input Входная строка
     * @return Минифицированная строка
     */
    static std::string minify(const std::string& input);

    /**
     * @brief Красивое форматирование collection строки
     * @param input Входная строка
     * @param indent Размер отступа
     * @return Отформатированная строка
     */
    static std::string prettify(const std::string& input, int indent = 4);

    /**
     * @brief Конвертация из JSON в collection (для миграции)
     * @param jsonInput JSON строка
     * @return Collection строка
     */
    static std::string jsonToCollection(const std::string& jsonInput);

    /**
     * @brief Конвертация из collection в JSON (для совместимости)
     * @param collectionInput Collection строка
     * @return JSON строка
     */
    static std::string collectionToJson(const std::string& collectionInput);

    // ========================================================================
    // Статистика
    // ========================================================================

    /**
     * @brief Получить количество ошибок парсинга
     * @return Количество ошибок
     */
    int getErrorCount() const { return errorCount; }

    /**
     * @brief Получить количество успешных парсингов
     * @return Количество успешных парсингов
     */
    int getSuccessCount() const { return successCount; }

    /**
     * @brief Сбросить статистику
     */
    void resetStatistics();

private:
    // Внутренние методы парсинга
    Value parseValue(const std::string& input, size_t& pos, int& line, int& column);
    Value parseString(const std::string& input, size_t& pos, int& line, int& column);
    Value parseNumber(const std::string& input, size_t& pos, int& line, int& column);
    Value parseBoolean(const std::string& input, size_t& pos);
    Value parseNull(const std::string& input, size_t& pos);
    Value parseArray(const std::string& input, size_t& pos, int& line, int& column);
    Value parseObject(const std::string& input, size_t& pos, int& line, int& column);
    Value parseCollection(const std::string& input, size_t& pos, int& line, int& column);

    // Утилиты
    void skipWhitespace(const std::string& input, size_t& pos, int& line, int& column);
    void skipComment(const std::string& input, size_t& pos, int& line, int& column);
    char peek(const std::string& input, size_t pos);
    char advance(const std::string& input, size_t& pos, int& line, int& column);
    bool isAtEnd(const std::string& input, size_t pos);
    std::string extractString(const std::string& input, size_t& pos, int& line, int& column);
    std::string extractNumber(const std::string& input, size_t& pos);
    std::string extractIdentifier(const std::string& input, size_t& pos);

    // Ошибки
    void error(const std::string& message, int line, int column);

    // Статистика
    int errorCount;
    int successCount;

    // Текущее состояние парсинга
    std::string currentError;
    int currentErrorLine;
    int currentErrorColumn;
};

} // namespace proxima

#endif // PROXIMA_COLLECTION_PARSER_H
