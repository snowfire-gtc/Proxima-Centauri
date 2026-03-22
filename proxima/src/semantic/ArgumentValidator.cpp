#include "ArgumentValidator.h"
#include <QVariantList>
#include <QVariantMap>
#include <QVector>
#include <QStringList>
#include <QRegularExpressionMatch>
#include <QMetaType>
#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QDateTime>
#include <QUrl>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <cmath>
#include <typeinfo>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

ArgumentValidator::ArgumentValidator(QObject *parent)
    : QObject(parent)
    , validationCount(0)
    , successCount(0)
    , failureCount(0) {
    
    // Регистрация стандартных валидаторов
    registerCustomValidator("isemail", [](const QVariant& value) {
        if (!value.canConvert<QString>()) return false;
        QString email = value.toString();
        QRegularExpression re(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return re.match(email).hasMatch();
    });
    
    registerCustomValidator("isurl", [](const QVariant& value) {
        if (!value.canConvert<QString>()) return false;
        QUrl url(value.toString());
        return url.isValid() && !url.isEmpty();
    });
    
    registerCustomValidator("isdate", [](const QVariant& value) {
        return value.canConvert<QDateTime>();
    });
    
    registerCustomValidator("isuuid", [](const QVariant& value) {
        if (!value.canConvert<QString>()) return false;
        QString uuid = value.toString();
        QRegularExpression re(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})",
                             QRegularExpression::CaseInsensitiveOption);
        return re.match(uuid).hasMatch();
    });
    
    registerCustomValidator("ishex", [](const QVariant& value) {
        if (!value.canConvert<QString>()) return false;
        QString hex = value.toString();
        QRegularExpression re(R"(^0x[0-9a-fA-F]+$|^#[0-9a-fA-F]+$)");
        return re.match(hex).hasMatch();
    });
    
    LOG_INFO("ArgumentValidator initialized");
}

ArgumentValidator::~ArgumentValidator() {
    LOG_DEBUG("ArgumentValidator destroyed");
}

// ============================================================================
// Парсинг конструкции arguments
// ============================================================================

QVector<ValidationRule> ArgumentValidator::parseArgumentsBlock(
    ArgumentsNodePtr argumentsNode,
    FunctionDeclNodePtr functionNode) {
    
    QVector<ValidationRule> rules;
    
    if (!argumentsNode) {
        return rules;
    }
    
    // Парсинг каждой директивы валидации
    for (const auto& directive : argumentsNode->directives) {
        ValidationRule rule;
        rule.argumentName = directive.argumentName;
        rule.line = directive.token.line;
        rule.column = directive.token.column;
        
        // Парсинг валидаторов
        for (const auto& validator : directive.validators) {
            ValidationRule validatorRule = rule;
            validatorRule.validator = stringToValidatorType(validator.name);
            validatorRule.validatorName = validator.name;
            validatorRule.parameters = validator.parameters;
            
            if (!validator.errorMessage.isEmpty()) {
                validatorRule.errorMessage = validator.errorMessage;
            }
            
            rules.append(validatorRule);
        }
        
        // Проверка на обязательность
        rule.isOptional = directive.isOptional;
        
        // Значение по умолчанию
        if (directive.hasDefaultValue) {
            rule.defaultValue = directive.defaultValue;
        }
        
        rules.append(rule);
    }
    
    LOG_DEBUG("Parsed " + QString::number(rules.size()) + " validation rules");
    
    return rules;
}

QVector<ValidationRule> ArgumentValidator::parseValidationAnnotations(
    const QString& commentText,
    const QString& argName) {
    
    QVector<ValidationRule> rules;
    
    // Поиск аннотаций валидации в комментариях
    // Формат: @validate argName: validator1, validator2(param1, param2)
    QRegularExpression re(R"(@validate\s+(\w+):\s*([^\n]+))");
    QRegularExpressionMatchIterator i = re.globalMatch(commentText);
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        
        if (match.captured(1) == argName) {
            QString validators = match.captured(2);
            QStringList validatorList = validators.split(",", Qt::SkipEmptyParts);
            
            for (const QString& validator : validatorList) {
                ValidationRule rule;
                rule.argumentName = argName;
                
                ValidationRule parsedRule = parseValidationAnnotation(validator.trimmed());
                if (parsedRule.validator != ValidatorType::Custom) {
                    rules.append(parsedRule);
                }
            }
        }
    }
    
    return rules;
}

ArgumentValidator::ValidationRule ArgumentValidator::parseValidationAnnotation(
    const QString& annotationText) {
    
    ValidationRule rule;
    
    // Парсинг валидатора с параметрами
    // Формат: validatorName(param1, param2) или validatorName
    QRegularExpression re(R"((\w+)(?:\(([^)]*)\))?)");
    QRegularExpressionMatch match = re.match(annotationText);
    
    if (match.hasMatch()) {
        rule.validatorName = match.captured(1);
        rule.validator = stringToValidatorType(rule.validatorName);
        
        if (match.captured(2).size() > 0) {
            rule.parameters = extractAnnotationParameters(match.captured(2));
        }
    }
    
    return rule;
}

QVariant ArgumentValidator::extractAnnotationParameters(const QString& annotationText) {
    // Парсинг параметров из строки
    // Формат: param1, param2, "string", 123, true
    QStringList params = annotationText.split(",", Qt::SkipEmptyParts);
    
    if (params.size() == 1) {
        QString param = params[0].trimmed();
        
        // Проверка типа
        if (param == "true") return QVariant(true);
        if (param == "false") return QVariant(false);
        
        bool okInt;
        int intVal = param.toInt(&okInt);
        if (okInt) return QVariant(intVal);
        
        bool okDouble;
        double doubleVal = param.toDouble(&okDouble);
        if (okDouble) return QVariant(doubleVal);
        
        // Строка (в кавычках)
        if (param.startsWith("\"") && param.endsWith("\"")) {
            return QVariant(param.mid(1, param.size() - 2));
        }
        
        return QVariant(param);
    } else {
        // Несколько параметров
        QVariantList list;
        for (const QString& param : params) {
            list.append(extractAnnotationParameters(param.trimmed()));
        }
        return QVariant(list);
    }
}

// ============================================================================
// Валидация аргументов
// ============================================================================

QVector<ValidationResult> ArgumentValidator::validateArguments(
    const QVector<ValidationRule>& rules,
    const QVector<QVariant>& actualValues,
    const QStringList& argNames) {
    
    QVector<ValidationResult> results;
    validationCount++;
    
    emit validationStarted("function");
    
    for (int i = 0; i < rules.size() && i < actualValues.size(); i++) {
        const ValidationRule& rule = rules[i];
        QVariant value = actualValues[i];
        
        // Применение значения по умолчанию
        if (value.isNull() && rule.defaultValue.isValid()) {
            value = rule.defaultValue;
        }
        
        // Проверка на обязательность
        if (value.isNull() && !rule.isOptional) {
            results.append(ValidationResult::failure(
                rule.argumentName,
                "Argument '" + rule.argumentName + "' is required",
                rule.validator));
            failureCount++;
            emit validationFailed("function", rule.argumentName, "Required argument missing");
            continue;
        }
        
        // Пропуск необязательных пустых аргументов
        if (value.isNull() && rule.isOptional) {
            results.append(ValidationResult::success(rule.argumentName, QVariant()));
            successCount++;
            continue;
        }
        
        // Валидация
        ValidationResult result = validateArgument(rule, value);
        results.append(result);
        
        if (result.isValid) {
            successCount++;
        } else {
            failureCount++;
            emit validationFailed("function", rule.argumentName, result.errorMessage);
        }
    }
    
    bool allValid = std::all_of(results.begin(), results.end(),
        [](const ValidationResult& r) { return r.isValid; });
    
    emit validationCompleted("function", allValid);
    
    LOG_DEBUG("Validation completed: " + QString::number(successCount) + 
              "/" + QString::number(validationCount) + " successful");
    
    return results;
}

ArgumentValidator::ValidationResult ArgumentValidator::validateArgument(
    const ValidationRule& rule,
    const QVariant& value) {
    
    switch (rule.validator) {
        case ValidatorType::IsLogical:
            if (!isLogical(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsPositive:
            if (!isPositive(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsNegative:
            if (!isNegative(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsNonNegative:
            if (!isNonNegative(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsInteger:
            if (!isInteger(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsReal:
            if (!isReal(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsNumeric:
            if (!isNumeric(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsString:
            if (!isString(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsVector:
            if (!isVector(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsMatrix:
            if (!isMatrix(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsLayer:
            if (!isLayer(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsCollection:
            if (!isCollection(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsEmpty:
            if (!isEmpty(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsNotEmpty:
            if (!isNotEmpty(value)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
            
        case ValidatorType::IsInRange: {
            QVariantList params = rule.parameters.toList();
            if (params.size() >= 2) {
                double min = params[0].toDouble();
                double max = params[1].toDouble();
                if (!isInRange(value, min, max)) {
                    return ValidationResult::failure(rule.argumentName,
                        generateErrorMessage(rule, value), rule.validator);
                }
            }
            break;
        }
        
        case ValidatorType::IsOneOf: {
            QVariantList allowedValues = ruleParameters.toList();
            if (!isOneOf(value, allowedValues)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
        }
        
        case ValidatorType::MatchesPattern: {
            QString pattern = rule.parameters.toString();
            if (!matchesPattern(value, pattern)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
        }
        
        case ValidatorType::Custom: {
            if (!rule.validatorName.isEmpty() && hasCustomValidator(rule.validatorName)) {
                if (!executeCustomValidator(rule.validatorName, value)) {
                    return ValidationResult::failure(rule.argumentName,
                        generateErrorMessage(rule, value), rule.validator);
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    return ValidationResult::success(rule.argumentName, value);
}

// ============================================================================
// Стандартные валидаторы
// ============================================================================

bool ArgumentValidator::isLogical(const QVariant& value) {
    return value.canConvert<bool>();
}

bool ArgumentValidator::isPositive(const QVariant& value) {
    if (!isNumeric(value)) return false;
    return value.toDouble() > 0;
}

bool ArgumentValidator::isNegative(const QVariant& value) {
    if (!isNumeric(value)) return false;
    return value.toDouble() < 0;
}

bool ArgumentValidator::isNonNegative(const QVariant& value) {
    if (!isNumeric(value)) return false;
    return value.toDouble() >= 0;
}

bool ArgumentValidator::isInteger(const QVariant& value) {
    if (!isNumeric(value)) return false;
    double d = value.toDouble();
    return d == static_cast<int>(d);
}

bool ArgumentValidator::isReal(const QVariant& value) {
    return value.canConvert<double>();
}

bool ArgumentValidator::isNumeric(const QVariant& value) {
    return value.canConvert<int>() || value.canConvert<double>() ||
           value.canConvert<float>() || value.canConvert<qlonglong>();
}

bool ArgumentValidator::isString(const QVariant& value) {
    return value.canConvert<QString>();
}

bool ArgumentValidator::isVector(const QVariant& value) {
    return value.canConvert<QVariantList>();
}

bool ArgumentValidator::isMatrix(const QVariant& value) {
    if (!value.canConvert<QVariantList>()) return false;
    QVariantList list = value.toList();
    if (list.isEmpty()) return false;
    return list[0].canConvert<QVariantList>();
}

bool ArgumentValidator::isLayer(const QVariant& value) {
    if (!value.canConvert<QVariantList>()) return false;
    QVariantList list = value.toList();
    if (list.isEmpty()) return false;
    if (!list[0].canConvert<QVariantList>()) return false;
    QVariantList list2d = list[0].toList();
    if (list2d.isEmpty()) return false;
    return list2d[0].canConvert<QVariantList>();
}

bool ArgumentValidator::isCollection(const QVariant& value) {
    return value.canConvert<QVariantMap>();
}

bool ArgumentValidator::isEmpty(const QVariant& value) {
    if (value.isNull()) return true;
    if (value.canConvert<QString>()) return value.toString().isEmpty();
    if (value.canConvert<QVariantList>()) return value.toList().isEmpty();
    if (value.canConvert<QVariantMap>()) return value.toMap().isEmpty();
    return false;
}

bool ArgumentValidator::isNotEmpty(const QVariant& value) {
    return !isEmpty(value);
}

bool ArgumentValidator::hasLength(const QVariant& value) {
    if (value.canConvert<QString>()) return true;
    if (value.canConvert<QVariantList>()) return true;
    return false;
}

bool ArgumentValidator::hasSize(const QVariant& value) {
    return hasLength(value) || isMatrix(value) || isLayer(value);
}

bool ArgumentValidator::isInRange(const QVariant& value, double min, double max) {
    if (!isNumeric(value)) return false;
    double d = value.toDouble();
    return d >= min && d <= max;
}

bool ArgumentValidator::isOneOf(const QVariant& value, const QVector<QVariant>& allowedValues) {
    for (const QVariant& allowed : allowedValues) {
        if (value == allowed) return true;
    }
    return false;
}

bool ArgumentValidator::matchesPattern(const QVariant& value, const QString& pattern) {
    if (!value.canConvert<QString>()) return false;
    QRegularExpression re(pattern);
    return re.match(value.toString()).hasMatch();
}

bool ArgumentValidator::isClass(const QVariant& value, const QString& className) {
    // В полной реализации - проверка типа объекта
    return false;
}

bool ArgumentValidator::isInterface(const QVariant& value, const QString& interfaceName) {
    // В полной реализации - проверка реализации интерфейса
    return false;
}

bool ArgumentValidator::isCallable(const QVariant& value) {
    // В полной реализации - проверка на функцию/метод
    return value.canConvert<QFunctionPointer>();
}

bool ArgumentValidator::isIterable(const QVariant& value) {
    return value.canConvert<QVariantList>() || 
           value.canConvert<QString>() ||
           value.canConvert<QVariantMap>();
}

// ============================================================================
// Регистрация пользовательских валидаторов
// ============================================================================

void ArgumentValidator::registerCustomValidator(
    const QString& name,
    std::function<bool(const QVariant&)> validatorFunc) {
    
    customValidators[name] = validatorFunc;
    LOG_DEBUG("Custom validator registered: " + name.toStdString());
}

bool ArgumentValidator::hasCustomValidator(const QString& name) const {
    return customValidators.contains(name);
}

bool ArgumentValidator::executeCustomValidator(const QString& name, const QVariant& value) {
    if (!hasCustomValidator(name)) {
        LOG_WARNING("Custom validator not found: " + name.toStdString());
        return false;
    }
    
    return customValidators[name](value);
}

// ============================================================================
// Генерация сообщений об ошибках
// ============================================================================

QString ArgumentValidator::generateErrorMessage(
    const ValidationRule& rule,
    const QVariant& value) {
    
    if (!rule.errorMessage.isEmpty()) {
        return rule.errorMessage;
    }
    
    QString errorMsg = "Argument '" + rule.argumentName + "' validation failed: ";
    
    switch (rule.validator) {
        case ValidatorType::IsLogical:
            errorMsg += "expected logical value";
            break;
        case ValidatorType::IsPositive:
            errorMsg += "expected positive value";
            break;
        case ValidatorType::IsNegative:
            errorMsg += "expected negative value";
            break;
        case ValidatorType::IsNonNegative:
            errorMsg += "expected non-negative value";
            break;
        case ValidatorType::IsInteger:
            errorMsg += "expected integer value";
            break;
        case ValidatorType::IsReal:
            errorMsg += "expected real value";
            break;
        case ValidatorType::IsNumeric:
            errorMsg += "expected numeric value";
            break;
        case ValidatorType::IsString:
            errorMsg += "expected string value";
            break;
        case ValidatorType::IsVector:
            errorMsg += "expected vector";
            break;
        case ValidatorType::IsMatrix:
            errorMsg += "expected matrix";
            break;
        case ValidatorType::IsLayer:
            errorMsg += "expected layer";
            break;
        case ValidatorType::IsCollection:
            errorMsg += "expected collection";
            break;
        case ValidatorType::IsEmpty:
            errorMsg += "expected empty value";
            break;
        case ValidatorType::IsNotEmpty:
            errorMsg += "expected non-empty value";
            break;
        case ValidatorType::IsInRange: {
            QVariantList params = rule.parameters.toList();
            if (params.size() >= 2) {
                errorMsg += QString("expected value in range [%1, %2]")
                    .arg(params[0].toDouble())
                    .arg(params[1].toDouble());
            }
            break;
        }
        case ValidatorType::IsOneOf:
            errorMsg += "expected one of allowed values";
            break;
        case ValidatorType::MatchesPattern:
            errorMsg += "expected value matching pattern";
            break;
        default:
            errorMsg += "validation failed";
            break;
    }
    
    return errorMsg;
}

QString ArgumentValidator::getValidatorDescription(ValidatorType type) {
    switch (type) {
        case ValidatorType::IsLogical: return "Проверка на логическое значение";
        case ValidatorType::IsPositive: return "Проверка на положительное число";
        case ValidatorType::IsNegative: return "Проверка на отрицательное число";
        case ValidatorType::IsNonNegative: return "Проверка на неотрицательное число";
        case ValidatorType::IsInteger: return "Проверка на целое число";
        case ValidatorType::IsReal: return "Проверка на вещественное число";
        case ValidatorType::IsNumeric: return "Проверка на числовое значение";
        case ValidatorType::IsString: return "Проверка на строку";
        case ValidatorType::IsVector: return "Проверка на вектор";
        case ValidatorType::IsMatrix: return "Проверка на матрицу";
        case ValidatorType::IsLayer: return "Проверка на слой";
        case ValidatorType::IsCollection: return "Проверка на коллекцию";
        case ValidatorType::IsEmpty: return "Проверка на пустоту";
        case ValidatorType::IsNotEmpty: return "Проверка на непустоту";
        case ValidatorType::HasLength: return "Проверка на наличие длины";
        case ValidatorType::HasSize: return "Проверка на наличие размера";
        case ValidatorType::IsInRange: return "Проверка на диапазон";
        case ValidatorType::IsOneOf: return "Проверка на одно из значений";
        case ValidatorType::MatchesPattern: return "Проверка на соответствие шаблону";
        default: return "Пользовательский валидатор";
    }
}

QString ArgumentValidator::getValidatorName(ValidatorType type) {
    switch (type) {
        case ValidatorType::IsLogical: return "islogical";
        case ValidatorType::IsPositive: return "ispositive";
        case ValidatorType::IsNegative: return "isnegative";
        case ValidatorType::IsNonNegative: return "isnonnegative";
        case ValidatorType::IsInteger: return "isinteger";
        case ValidatorType::IsReal: return "isreal";
        case ValidatorType::IsNumeric: return "isnumeric";
        case ValidatorType::IsString: return "isstring";
        case ValidatorType::IsVector: return "isvector";
        case ValidatorType::IsMatrix: return "ismatrix";
        case ValidatorType::IsLayer: return "islayer";
        case ValidatorType::IsCollection: return "iscollection";
        case ValidatorType::IsEmpty: return "isempty";
        case ValidatorType::IsNotEmpty: return "isnotempty";
        case ValidatorType::HasLength: return "haslength";
        case ValidatorType::HasSize: return "hassize";
        case ValidatorType::IsInRange: return "isinrange";
        case ValidatorType::IsOneOf: return "isoneof";
        case ValidatorType::MatchesPattern: return "matchespattern";
        default: return "custom";
    }
}

ArgumentValidator::ValidatorType ArgumentValidator::stringToValidatorType(
    const QString& validatorName) {
    
    QString name = validatorName.toLower();
    
    if (name == "islogical") return ValidatorType::IsLogical;
    if (name == "ispositive") return ValidatorType::IsPositive;
    if (name == "isnegative") return ValidatorType::IsNegative;
    if (name == "isnonnegative") return ValidatorType::IsNonNegative;
    if (name == "isinteger") return ValidatorType::IsInteger;
    if (name == "isreal") return ValidatorType::IsReal;
    if (name == "isnumeric") return ValidatorType::IsNumeric;
    if (name == "isstring") return ValidatorType::IsString;
    if (name == "isvector") return ValidatorType::IsVector;
    if (name == "ismatrix") return ValidatorType::IsMatrix;
    if (name == "islayer") return ValidatorType::IsLayer;
    if (name == "iscollection") return ValidatorType::IsCollection;
    if (name == "isempty") return ValidatorType::IsEmpty;
    if (name == "isnotempty") return ValidatorType::IsNotEmpty;
    if (name == "haslength") return ValidatorType::HasLength;
    if (name == "hassize") return ValidatorType::HasSize;
    if (name == "isinrange") return ValidatorType::IsInRange;
    if (name == "isoneof") return ValidatorType::IsOneOf;
    if (name == "matchespattern") return ValidatorType::MatchesPattern;
    
    return ValidatorType::Custom;
}

QString ArgumentValidator::validatorTypeToString(ValidatorType type) {
    return getValidatorName(type);
}

} // namespace proxima