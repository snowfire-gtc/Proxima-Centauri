#include "CollectionParser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QRegularExpression>
#include <QStack>
#include <QDebug>

namespace proxima {

// Поддержка 1-based индексации в runtime:
CollectionParser::Value CollectionParser::evaluateArrayAccess(
    const CollectionParser::Value& array,
    const QVector<CollectionParser::Value>& indices) {

    if (array.type != ValueType::Array) {
        throw QString("Cannot index non-array value");
    }

    for (const auto& indexValue : indices) {
        if (indexValue.type != ValueType::Number) {
            throw QString("Array index must be numeric");
        }

        // Индекс в стиле Proxima (1-based)
        int proximaIndex = static_cast<int>(indexValue.numberValue);

        // Конвертация в 0-based для внутреннего хранения
        int cppIndex = IndexConverter::toCppIndex(proximaIndex);

        // Проверка валидности
        if (!IndexConverter::isValidProximaIndex(proximaIndex, array.arrayValue.size())) {
            throw QString("Array index out of range: %1 (valid: 1-%2)")
                .arg(proximaIndex)
                .arg(array.arrayValue.size());
        }

        // Доступ к элементу
        // array.arrayValue[cppIndex]
    }

// ============================================================================
// Value Implementation
// ============================================================================

CollectionParser::Value CollectionParser::Value::fromString(const QString& str) {
    Value v;
    v.type = ValueType::String;
    v.stringValue = str;
    return v;
}

CollectionParser::Value CollectionParser::Value::fromNumber(double num) {
    Value v;
    v.type = ValueType::Number;
    v.numberValue = num;
    return v;
}

CollectionParser::Value CollectionParser::Value::fromBoolean(bool b) {
    Value v;
    v.type = ValueType::Boolean;
    v.boolValue = b;
    return v;
}

CollectionParser::Value CollectionParser::Value::fromArray(const QVector<Value>& arr) {
    Value v;
    v.type = ValueType::Array;
    v.arrayValue = arr;
    return v;
}

CollectionParser::Value CollectionParser::Value::fromObject(const QMap<QString, Value>& obj) {
    Value v;
    v.type = ValueType::Object;
    v.objectValue = obj;
    return v;
}

CollectionParser::Value CollectionParser::Value::fromCollection(const QString& collectionStr) {
    CollectionParser parser;
    ParseResult result = parser.parse(collectionStr);
    if (result.success) {
        return result.value;
    }
    return Value::fromNull();
}

QString CollectionParser::Value::toString() const {
    switch (type) {
        case ValueType::String:
            return "\"" + stringValue + "\"";
        case ValueType::Number:
            return QString::number(numberValue);
        case ValueType::Boolean:
            return boolValue ? "true" : "false";
        case ValueType::Null:
            return "null";
        case ValueType::Array:
        case ValueType::Object:
        case ValueType::Collection:
            return toCollectionString();
        default:
            return "null";
    }
}

QString CollectionParser::Value::toCollectionString(int indent) const {
    QString result;
    QString indentStr(indent * 4, ' ');
    QString nextIndentStr((indent + 1) * 4, ' ');

    switch (type) {
        case ValueType::Collection:
        case ValueType::Object: {
            result += "[\n";
            int count = 0;
            for (auto it = objectValue.begin(); it != objectValue.end(); ++it) {
                if (count > 0) {
                    result += ",,\n";
                }
                result += nextIndentStr + "\"" + it.key() + "\", " + it.value().toCollectionString(indent + 1);
                count++;
            }
            if (count > 0) {
                result += "\n";
            }
            result += indentStr + "]";
            break;
        }
        case ValueType::Array: {
            result += "[";
            for (int i = 0; i < arrayValue.size(); i++) {
                if (i > 0) {
                    result += ", ";
                }
                result += arrayValue[i].toCollectionString(indent + 1);
            }
            result += "]";
            break;
        }
        default:
            result = toString();
            break;
    }

    return result;
}

CollectionParser::Value CollectionParser::Value::get(const QString& key) const {
    if (type == ValueType::Object || type == ValueType::Collection) {
        auto it = objectValue.find(key);
        if (it != objectValue.end()) {
            return it.value();
        }
    }
    return Value();
}

CollectionParser::Value CollectionParser::Value::get(int index) const {
    if (type == ValueType::Array && index >= 0 && index < arrayValue.size()) {
        return arrayValue[index];
    }
    return Value();
}

bool CollectionParser::Value::has(const QString& key) const {
    if (type == ValueType::Object || type == ValueType::Collection) {
        return objectValue.contains(key);
    }
    return false;
}

int CollectionParser::Value::size() const {
    switch (type) {
        case ValueType::Array:
            return arrayValue.size();
        case ValueType::Object:
        case ValueType::Collection:
            return objectValue.size();
        default:
            return 0;
    }
}

bool CollectionParser::Value::operator==(const Value& other) const {
    if (type != other.type) return false;

    switch (type) {
        case ValueType::String:
            return stringValue == other.stringValue;
        case ValueType::Number:
            return qAbs(numberValue - other.numberValue) < 1e-10;
        case ValueType::Boolean:
            return boolValue == other.boolValue;
        case ValueType::Null:
            return true;
        case ValueType::Array:
            return arrayValue == other.arrayValue;
        case ValueType::Object:
        case ValueType::Collection:
            return objectValue == other.objectValue;
        default:
            return false;
    }
}

// ============================================================================
// CollectionParser Implementation
// ============================================================================

CollectionParser::CollectionParser()
    : errorCount(0)
    , successCount(0)
    , currentErrorLine(0)
    , currentErrorColumn(0) {
}

CollectionParser::~CollectionParser() {
}

CollectionParser::ParseResult CollectionParser::parse(const QString& input) {
    ParseResult result;

    if (input.trimmed().isEmpty()) {
        result.error = "Empty input";
        result.errorLine = 1;
        result.errorColumn = 1;
        errorCount++;
        return result;
    }

    int pos = 0;
    int line = 1;
    int column = 1;

    currentError.clear();
    currentErrorLine = 0;
    currentErrorColumn = 0;

    try {
        result.value = parseValue(input, pos, line, column);
        result.success = true;
        successCount++;

        LOG_DEBUG("Collection parsed successfully");
    } catch (const QString& errorMsg) {
        result.success = false;
        result.error = errorMsg;
        result.errorLine = currentErrorLine;
        result.errorColumn = currentErrorColumn;
        errorCount++;

        LOG_ERROR("Collection parse error: " + errorMsg.toStdString() +
                 " at line " + QString::number(currentErrorLine) +
                 ", column " + QString::number(currentErrorColumn));
    }

    return result;
}

CollectionParser::ParseResult CollectionParser::parseFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ParseResult result;
        result.error = "Cannot open file: " + filePath;
        result.errorLine = 1;
        result.errorColumn = 1;
        errorCount++;
        return result;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();

    return parse(content);
}

CollectionParser::ParseResult CollectionParser::parseStream(QTextStream& stream) {
    QString content = stream.readAll();
    return parse(content);
}

QString CollectionParser::serialize(const Value& value, int indent) {
    return value.toCollectionString(indent);
}

bool CollectionParser::serializeToFile(const Value& value, const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR("Cannot write to file: " + filePath.toStdString());
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << serialize(value, 4);
    file.close();

    LOG_INFO("Collection serialized to: " + filePath.toStdString());
    return true;
}

CollectionParser::Value CollectionParser::createPair(const QString& key, const Value& value) {
    Value v;
    v.type = ValueType::Collection;
    v.objectValue[key] = value;
    return v;
}

CollectionParser::Value CollectionParser::createCollection(const QVector<QPair<QString, Value>>& pairs) {
    Value v;
    v.type = ValueType::Collection;

    for (const auto& pair : pairs) {
        v.objectValue[pair.first] = pair.second;
    }

    return v;
}

CollectionParser::Value CollectionParser::createArray(const QVector<Value>& values) {
    return Value::fromArray(values);
}

bool CollectionParser::isValid(const QString& input) {
    CollectionParser parser;
    ParseResult result = parser.parse(input);
    return result.success;
}

QString CollectionParser::minify(const QString& input) {
    QString result;
    bool inString = false;
    bool escapeNext = false;

    for (int i = 0; i < input.size(); i++) {
        char c = input[i].toLatin1();

        if (escapeNext) {
            result += c;
            escapeNext = false;
            continue;
        }

        if (c == '\\' && inString) {
            result += c;
            escapeNext = true;
            continue;
        }

        if (c == '"') {
            inString = !inString;
            result += c;
            continue;
        }

        if (!inString) {
            // Пропуск пробелов вне строк
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                continue;
            }

            // Пропуск комментариев
            if (c == '/' && i + 1 < input.size()) {
                char next = input[i + 1].toLatin1();
                if (next == '/') {
                    // Однострочный комментарий
                    while (i < input.size() && input[i] != '\n') {
                        i++;
                    }
                    continue;
                } else if (next == '*') {
                    // Многострочный комментарий
                    i += 2;
                    while (i + 1 < input.size()) {
                        if (input[i] == '*' && input[i + 1] == '/') {
                            i++;
                            break;
                        }
                        i++;
                    }
                    continue;
                }
            }
        }

        result += c;
    }

    return result;
}

QString CollectionParser::prettify(const QString& input, int indent) {
    ParseResult result = parse(input);
    if (result.success) {
        return serialize(result.value, indent / 4);
    }
    return input;
}

QString CollectionParser::jsonToCollection(const QString& jsonInput) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonInput.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        return "[]";
    }

    Value value = jsonValueToCollectionValue(doc.toVariant());
    return value.toCollectionString();
}

QString CollectionParser::collectionToJson(const QString& collectionInput) {
    ParseResult result = parse(collectionInput);
    if (!result.success) {
        return "{}";
    }

    QJsonValue jsonValue = collectionValueToJsonValue(result.value);
    QJsonDocument doc(jsonValue);
    return QString::fromUtf8(doc.toJson());
}

void CollectionParser::resetStatistics() {
    errorCount = 0;
    successCount = 0;
}

// ============================================================================
// Internal Parsing Methods
// ============================================================================

CollectionParser::Value CollectionParser::parseValue(const QString& input, int& pos, int& line, int& column) {
    skipWhitespace(input, pos, line, column);

    if (isAtEnd(input, pos)) {
        error("Unexpected end of input", line, column);
        return Value();
    }

    char c = peek(input, pos);

    if (c == '[') {
        return parseCollection(input, pos, line, column);
    } else if (c == '"') {
        return parseString(input, pos, line, column);
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        return parseNumber(input, pos, line, column);
    } else if (c == 't' || c == 'f') {
        return parseBoolean(input, pos);
    } else if (c == 'n') {
        return parseNull(input, pos);
    } else {
        error(QString("Unexpected character: %1").arg(c), line, column);
        return Value();
    }
}

CollectionParser::Value CollectionParser::parseString(const QString& input, int& pos, int& line, int& column) {
    return Value::fromString(extractString(input, pos, line, column));
}

CollectionParser::Value CollectionParser::parseNumber(const QString& input, int& pos, int& line, int& column) {
    QString numStr = extractNumber(input, pos);
    bool ok;
    double num = numStr.toDouble(&ok);

    if (!ok) {
        error(QString("Invalid number: %1").arg(numStr), line, column);
        return Value::fromNumber(0.0);
    }

    return Value::fromNumber(num);
}

CollectionParser::Value CollectionParser::parseBoolean(const QString& input, int& pos) {
    if (input.mid(pos, 4) == "true") {
        pos += 4;
        return Value::fromBoolean(true);
    } else if (input.mid(pos, 5) == "false") {
        pos += 5;
        return Value::fromBoolean(false);
    }

    return Value::fromBoolean(false);
}

CollectionParser::Value CollectionParser::parseNull(const QString& input, int& pos) {
    if (input.mid(pos, 4) == "null") {
        pos += 4;
        return Value();
    }

    return Value();
}

CollectionParser::Value CollectionParser::parseArray(const QString& input, int& pos, int& line, int& column) {
    QVector<Value> array;

    advance(input, pos, line, column); // Skip '['
    skipWhitespace(input, pos, line, column);

    if (peek(input, pos) == ']') {
        advance(input, pos, line, column);
        return Value::fromArray(array);
    }

    while (!isAtEnd(input, pos)) {
        Value value = parseValue(input, pos, line, column);
        array.append(value);

        skipWhitespace(input, pos, line, column);

        if (peek(input, pos) == ']') {
            advance(input, pos, line, column);
            break;
        } else if (peek(input, pos) == ',') {
            advance(input, pos, line, column);
            skipWhitespace(input, pos, line, column);
        } else {
            error("Expected ',' or ']'", line, column);
            break;
        }
    }

    return Value::fromArray(array);
}

CollectionParser::Value CollectionParser::parseObject(const QString& input, int& pos, int& line, int& column) {
    QMap<QString, Value> object;

    advance(input, pos, line, column); // Skip '{'
    skipWhitespace(input, pos, line, column);

    if (peek(input, pos) == '}') {
        advance(input, pos, line, column);
        return Value::fromObject(object);
    }

    while (!isAtEnd(input, pos)) {
        skipWhitespace(input, pos, line, column);

        if (peek(input, pos) != '"') {
            error("Expected string key", line, column);
            break;
        }

        Value key = parseString(input, pos, line, column);

        skipWhitespace(input, pos, line, column);

        if (peek(input, pos) != ':') {
            error("Expected ':'", line, column);
            break;
        }
        advance(input, pos, line, column);

        skipWhitespace(input, pos, line, column);

        Value value = parseValue(input, pos, line, column);
        object[key.stringValue] = value;

        skipWhitespace(input, pos, line, column);

        if (peek(input, pos) == '}') {
            advance(input, pos, line, column);
            break;
        } else if (peek(input, pos) == ',') {
            advance(input, pos, line, column);
            skipWhitespace(input, pos, line, column);
        } else {
            error("Expected ',' or '}'", line, column);
            break;
        }
    }

    return Value::fromObject(object);
}

CollectionParser::Value CollectionParser::parseCollection(const QString& input, int& pos, int& line, int& column) {
    QMap<QString, Value> collection;

    advance(input, pos, line, column); // Skip '['
    skipWhitespace(input, pos, line, column);
    skipComment(input, pos, line, column);
    skipWhitespace(input, pos, line, column);

    if (peek(input, pos) == ']') {
        advance(input, pos, line, column);
        Value v;
        v.type = ValueType::Collection;
        v.objectValue = collection;
        return v;
    }

    while (!isAtEnd(input, pos)) {
        skipWhitespace(input, pos, line, column);
        skipComment(input, pos, line, column);
        skipWhitespace(input, pos, line, column);

        if (peek(input, pos) == ']') {
            advance(input, pos, line, column);
            break;
        }

        // Parse key
        if (peek(input, pos) != '"') {
            error("Expected string key in collection", line, column);
            break;
        }

        Value key = parseString(input, pos, line, column);

        skipWhitespace(input, pos, line, column);

        if (peek(input, pos) != ',') {
            error("Expected ',' after key", line, column);
            break;
        }
        advance(input, pos, line, column);

        skipWhitespace(input, pos, line, column);

        // Parse value
        Value value = parseValue(input, pos, line, column);
        collection[key.stringValue] = value;

        skipWhitespace(input, pos, line, column);
        skipComment(input, pos, line, column);
        skipWhitespace(input, pos, line, column);

        // Check for ,, separator or end
        if (peek(input, pos) == ']') {
            advance(input, pos, line, column);
            break;
        } else if (peek(input, pos) == ',') {
            advance(input, pos, line, column); // Skip first ','
            skipWhitespace(input, pos, line, column);

            // Check for second ',' (optional in collection format)
            if (peek(input, pos) == ',') {
                advance(input, pos, line, column); // Skip second ','
            }

            skipWhitespace(input, pos, line, column);
            skipComment(input, pos, line, column);
            skipWhitespace(input, pos, line, column);
        } else {
            error("Expected ',' or ']'", line, column);
            break;
        }
    }

    Value v;
    v.type = ValueType::Collection;
    v.objectValue = collection;
    return v;
}

// ============================================================================
// Utility Methods
// ============================================================================

void CollectionParser::skipWhitespace(const QString& input, int& pos, int& line, int& column) {
    while (!isAtEnd(input, pos)) {
        char c = peek(input, pos);

        if (c == ' ' || c == '\t') {
            advance(input, pos, line, column);
        } else if (c == '\n') {
            advance(input, pos, line, column);
        } else if (c == '\r') {
            advance(input, pos, line, column);
        } else if (c == '/') {
            skipComment(input, pos, line, column);
        } else {
            break;
        }
    }
}

void CollectionParser::skipComment(const QString& input, int& pos, int& line, int& column) {
    if (isAtEnd(input, pos)) return;

    if (peek(input, pos) != '/') return;
    advance(input, pos, line, column);

    if (isAtEnd(input, pos)) return;

    char next = peek(input, pos);

    if (next == '/') {
        // Однострочный комментарий
        while (!isAtEnd(input, pos) && peek(input, pos) != '\n') {
            advance(input, pos, line, column);
        }
    } else if (next == '*') {
        // Многострочный комментарий
        advance(input, pos, line, column);

        while (!isAtEnd(input, pos)) {
            if (peek(input, pos) == '*' && pos + 1 < input.size() && input[pos + 1] == '/') {
                advance(input, pos, line, column);
                advance(input, pos, line, column);
                break;
            }
            advance(input, pos, line, column);
        }
    } else {
        // Не комментарий, возвращаем '/'
        pos--;
        column--;
    }
}

char CollectionParser::peek(const QString& input, int pos) {
    if (pos >= input.size()) return '\0';
    return input[pos].toLatin1();
}

char CollectionParser::advance(const QString& input, int& pos, int& line, int& column) {
    if (pos >= input.size()) return '\0';

    char c = input[pos].toLatin1();
    pos++;

    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }

    return c;
}

bool CollectionParser::isAtEnd(const QString& input, int pos) {
    return pos >= input.size();
}

QString CollectionParser::extractString(const QString& input, int& pos, int& line, int& column) {
    advance(input, pos, line, column); // Skip opening '"'

    QString result;

    while (!isAtEnd(input, pos) && peek(input, pos) != '"') {
        char c = advance(input, pos, line, column);

        if (c == '\\') {
            if (!isAtEnd(input, pos)) {
                char escaped = advance(input, pos, line, column);
                switch (escaped) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        // Unicode escape
                        if (pos + 4 <= input.size()) {
                            QString hex = input.mid(pos, 4);
                            pos += 4;
                            column += 4;
                            result += QChar(hex.toInt(nullptr, 16));
                        }
                        break;
                    }
                    default:
                        result += escaped;
                        break;
                }
            }
        } else {
            result += c;
        }
    }

    if (!isAtEnd(input, pos)) {
        advance(input, pos, line, column); // Skip closing '"'
    }

    return result;
}

QString CollectionParser::extractNumber(const QString& input, int& pos) {
    QString result;

    // Sign
    if (peek(input, pos) == '-') {
        result += advance(input, pos, line, column);
    }

    // Integer part
    while (!isAtEnd(input, pos) && peek(input, pos) >= '0' && peek(input, pos) <= '9') {
        result += advance(input, pos, line, column);
    }

    // Fractional part
    if (peek(input, pos) == '.') {
        result += advance(input, pos, line, column);

        while (!isAtEnd(input, pos) && peek(input, pos) >= '0' && peek(input, pos) <= '9') {
            result += advance(input, pos, line, column);
        }
    }

    // Exponent part
    if (peek(input, pos) == 'e' || peek(input, pos) == 'E') {
        result += advance(input, pos, line, column);

        if (peek(input, pos) == '+' || peek(input, pos) == '-') {
            result += advance(input, pos, line, column);
        }

        while (!isAtEnd(input, pos) && peek(input, pos) >= '0' && peek(input, pos) <= '9') {
            result += advance(input, pos, line, column);
        }
    }

    return result;
}

QString CollectionParser::extractIdentifier(const QString& input, int& pos) {
    QString result;

    while (!isAtEnd(input, pos)) {
        char c = peek(input, pos);

        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_') {
            result += advance(input, pos, line, column);
        } else {
            break;
        }
    }

    return result;
}

void CollectionParser::error(const QString& message, int line, int column) {
    currentError = message;
    currentErrorLine = line;
    currentErrorColumn = column;
    throw message;
}

// ============================================================================
// JSON Conversion Utilities
// ============================================================================

CollectionParser::Value CollectionParser::jsonValueToCollectionValue(const QVariant& jsonValue) {
    if (jsonValue.isNull()) {
        return Value();
    } else if (jsonValue.userType() == QMetaType::QString) {
        return Value::fromString(jsonValue.toString());
    } else if (jsonValue.userType() == QMetaType::Double ||
               jsonValue.userType() == QMetaType::Int ||
               jsonValue.userType() == QMetaType::LongLong) {
        return Value::fromNumber(jsonValue.toDouble());
    } else if (jsonValue.userType() == QMetaType::Bool) {
        return Value::fromBoolean(jsonValue.toBool());
    } else if (jsonValue.userType() == QMetaType::QVariantList) {
        QVariantList list = jsonValue.toList();
        QVector<Value> array;
        for (const QVariant& item : list) {
            array.append(jsonValueToCollectionValue(item));
        }
        return Value::fromArray(array);
    } else if (jsonValue.userType() == QMetaType::QVariantMap) {
        QVariantMap map = jsonValue.toMap();
        QMap<QString, Value> object;
        for (auto it = map.begin(); it != map.end(); ++it) {
            object[it.key()] = jsonValueToCollectionValue(it.value());
        }
        return Value::fromObject(object);
    }

    return Value();
}

QJsonValue CollectionParser::collectionValueToJsonValue(const Value& collectionValue) {
    switch (collectionValue.type) {
        case ValueType::String:
            return QJsonValue(collectionValue.stringValue);
        case ValueType::Number:
            return QJsonValue(collectionValue.numberValue);
        case ValueType::Boolean:
            return QJsonValue(collectionValue.boolValue);
        case ValueType::Null:
            return QJsonValue();
        case ValueType::Array: {
            QJsonArray array;
            for (const Value& item : collectionValue.arrayValue) {
                array.append(collectionValueToJsonValue(item));
            }
            return QJsonValue(array);
        }
        case ValueType::Object:
        case ValueType::Collection: {
            QJsonObject object;
            for (auto it = collectionValue.objectValue.begin(); it != collectionValue.objectValue.end(); ++it) {
                object[it.key()] = collectionValueToJsonValue(it.value());
            }
            return QJsonValue(object);
        }
        default:
            return QJsonValue();
    }
}

} // namespace proxima
