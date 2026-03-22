#ifndef PROXIMA_COLLECTION_PARSER_H
#define PROXIMA_COLLECTION_PARSER_H

#include <QString>
#include <QVariant>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QStack>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
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
        QString stringValue;
        double numberValue;
        bool boolValue;
        QVector<Value> arrayValue;
        QMap<QString, Value> objectValue;

        Value() : type(ValueType::Null), numberValue(0.0), boolValue(false) {}

        //Индексация массивов начинается с 1.
        static Value evaluateArrayAccess(const Value& array, const QVector<Value>& indices);

        // Конвертеры
        static Value fromString(const QString& str);
        static Value fromNumber(double num);
        static Value fromBoolean(bool b);
        static Value fromArray(const QVector<Value>& arr);
        static Value fromObject(const QMap<QString, Value>& obj);
        static Value fromCollection(const QString& collectionStr);

        // Сериализация
        QString toString() const;
        QString toCollectionString(int indent = 0) const;

        // Проверки типов
        bool isString() const { return type == ValueType::String; }
        bool isNumber() const { return type == ValueType::Number; }
        bool isBoolean() const { return type == ValueType::Boolean; }
        bool isNull() const { return type == ValueType::Null; }
        bool isArray() const { return type == ValueType::Array; }
        bool isObject() const { return type == ValueType::Object; }
        bool isCollection() const { return type == ValueType::Collection; }

        // Доступ к значениям
        QString asString() const { return stringValue; }
        double asNumber() const { return numberValue; }
        bool asBoolean() const { return boolValue; }
        const QVector<Value>& asArray() const { return arrayValue; }
        const QMap<QString, Value>& asObject() const { return objectValue; }

        // Удобные методы доступа
        Value get(const QString& key) const;
        Value get(int index) const;
        bool has(const QString& key) const;
        int size() const;

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
        QString error;
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
    ParseResult parse(const QString& input);

    /**
     * @brief Парсинг файла в формате collection
     * @param filePath Путь к файлу
     * @return Результат парсинга
     */
    ParseResult parseFile(const QString& filePath);

    /**
     * @brief Парсинг потока в формате collection
     * @param stream Входной поток
     * @return Результат парсинга
     */
    ParseResult parseStream(QTextStream& stream);

    // ========================================================================
    // Сериализация
    // ========================================================================

    /**
     * @brief Сериализация Value в строку collection
     * @param value Значение для сериализации
     * @param indent Уровень отступа
     * @return Строка в формате collection
     */
    QString serialize(const Value& value, int indent = 0);

    /**
     * @brief Сериализация Value в файл
     * @param value Значение для сериализации
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool serializeToFile(const Value& value, const QString& filePath);

    // ========================================================================
    // Утилиты
    // ========================================================================

    /**
     * @brief Создание collection из пары ключ-значение
     * @param key Ключ
     * @param value Значение
     * @return Value в формате collection
     */
    static Value createPair(const QString& key, const Value& value);

    /**
     * @brief Создание collection из списка пар
     * @param pairs Список пар ключ-значение
     * @return Value в формате collection
     */
    static Value createCollection(const QVector<QPair<QString, Value>>& pairs);

    /**
     * @brief Создание массива из списка значений
     * @param values Список значений
     * @return Value в формате array
     */
    static Value createArray(const QVector<Value>& values);

    /**
     * @brief Проверка валидности формата collection
     * @param input Входная строка
     * @return true если валидно
     */
    static bool isValid(const QString& input);

    /**
     * @brief Минификация collection строки
     * @param input Входная строка
     * @return Минифицированная строка
     */
    static QString minify(const QString& input);

    /**
     * @brief Красивое форматирование collection строки
     * @param input Входная строка
     * @param indent Размер отступа
     * @return Отформатированная строка
     */
    static QString prettify(const QString& input, int indent = 4);

    /**
     * @brief Конвертация из JSON в collection (для миграции)
     * @param jsonInput JSON строка
     * @return Collection строка
     */
    static QString jsonToCollection(const QString& jsonInput);

    /**
     * @brief Конвертация из collection в JSON (для совместимости)
     * @param collectionInput Collection строка
     * @return JSON строка
     */
    static QString collectionToJson(const QString& collectionInput);

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
    Value parseValue(const QString& input, int& pos, int& line, int& column);
    Value parseString(const QString& input, int& pos, int& line, int& column);
    Value parseNumber(const QString& input, int& pos, int& line, int& column);
    Value parseBoolean(const QString& input, int& pos);
    Value parseNull(const QString& input, int& pos);
    Value parseArray(const QString& input, int& pos, int& line, int& column);
    Value parseObject(const QString& input, int& pos, int& line, int& column);
    Value parseCollection(const QString& input, int& pos, int& line, int& column);

    // Утилиты
    void skipWhitespace(const QString& input, int& pos, int& line, int& column);
    void skipComment(const QString& input, int& pos, int& line, int& column);
    char peek(const QString& input, int pos);
    char advance(const QString& input, int& pos, int& line, int& column);
    bool isAtEnd(const QString& input, int pos);
    QString extractString(const QString& input, int& pos, int& line, int& column);
    QString extractNumber(const QString& input, int& pos);
    QString extractIdentifier(const QString& input, int& pos);

    // Ошибки
    void error(const QString& message, int line, int column);

    // Статистика
    int errorCount;
    int successCount;

    // Текущее состояние парсинга
    QString currentError;
    int currentErrorLine;
    int currentErrorColumn;
};

} // namespace proxima

#endif // PROXIMA_COLLECTION_PARSER_H
