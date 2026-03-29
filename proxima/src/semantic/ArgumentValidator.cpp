#include "ArgumentValidator.h"
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include <sstream>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

ArgumentValidator::ArgumentValidator()
    : validationCount(0)
    , successCount(0)
    , failureCount(0) {
    
    // Регистрация стандартных валидаторов
    registerCustomValidator("isemail", [](const AnyValue& value) {
        const std::string* str = std::any_cast<std::string>(&value);
        if (!str) return false;
        std::regex email_re(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(*str, email_re);
    });
    
    registerCustomValidator("isurl", [](const AnyValue& value) {
        const std::string* str = std::any_cast<std::string>(&value);
        if (!str) return false;
        // Простая проверка URL
        return str->find("http://") == 0 || str->find("https://") == 0;
    });
    
    registerCustomValidator("isuuid", [](const AnyValue& value) {
        const std::string* str = std::any_cast<std::string>(&value);
        if (!str) return false;
        std::regex uuid_re(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})", std::regex::icase);
        return std::regex_match(*str, uuid_re);
    });
    
    registerCustomValidator("ishex", [](const AnyValue& value) {
        const std::string* str = std::any_cast<std::string>(&value);
        if (!str) return false;
        std::regex hex_re(R"(^0x[0-9a-fA-F]+$|^#[0-9a-fA-F]+$)");
        return std::regex_match(*str, hex_re);
    });
    
    LOG_INFO("ArgumentValidator initialized");
}

ArgumentValidator::~ArgumentValidator() {
    LOG_DEBUG("ArgumentValidator destroyed");
}

// ============================================================================
// Парсинг конструкции arguments
// ============================================================================

std::vector<std::any> ArgumentValidator::parseArgumentsBlock(
    ArgumentsNodePtr argumentsNode,
    FunctionDeclNodePtr functionNode) {
    
    std::vector<std::any> rules;
    
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
            
            rules.push_back(validatorRule);
        }
        
        // Проверка на обязательность
        rule.isOptional = directive.isOptional;
        
        // Значение по умолчанию
        if (directive.hasDefaultValue) {
            rule.defaultValue = directive.defaultValue;
        }
        
        rules.push_back(rule);
    }
    
    LOG_DEBUG("Parsed " + QString::number(rules.size()) + " validation rules");
    
    return rules;
}

std::vector<std::any> ArgumentValidator::parseValidationAnnotations(
    const std::string& commentText,
    const std::string& argName) {
    
    std::vector<std::any> rules;
    
    // Поиск аннотаций валидации в комментариях
    // Формат: @validate argName: validator1, validator2(param1, param2)
    QRegularExpression re(R"(@validate\s+(\w+):\s*([^\n]+))");
    std::sregex_iterator i = re(std::sregex_iterator(commentText.begin(), commentText.end(), regex), std::sregex_iterator());
    
    while (i.hasNext()) {
        std::smatch match = i.next();
        
        if (matchmatch[1].str() == argName) {
            std::string validators = matchmatch[2].str();
            Qvector<string> validatorList = validators.empty() ? std::vector<std::string>() : parse_csv_line;
            
            for (const std::string& validator : validatorList) {
                ValidationRule rule;
                rule.argumentName = argName;
                
                ValidationRule parsedRule = parseValidationAnnotation(validator);
                if (parsedRule.validator != ValidatorType::Custom) {
                    rules.push_back(parsedRule);
                }
            }
        }
    }
    
    return rules;
}

ArgumentValidator::ValidationRule ArgumentValidator::parseValidationAnnotation(
    const std::string& annotationText) {
    
    ValidationRule rule;
    
    // Парсинг валидатора с параметрами
    // Формат: validatorName(param1, param2) или validatorName
    QRegularExpression re(R"((\w+)(?:\(([^)]*)\))?)");
    std::smatch match = re.match(annotationText);
    
    if (match.hasMatch()) {
        rule.validatorName = matchmatch[1].str();
        rule.validator = stringToValidatorType(rule.validatorName);
        
        if (matchmatch[2].str().size() > 0) {
            rule.parameters = extractAnnotationParameters(matchmatch[2].str());
        }
    }
    
    return rule;
}

AnyValue ArgumentValidator::extractAnnotationParameters(const std::string& annotationText) {
    // Парсинг параметров из строки
    // Формат: param1, param2, "string", 123, true
    Qvector<string> params = annotationText.empty() ? std::vector<std::string>() : parse_csv_line;
    
    if (params.size() == 1) {
        std::string param = params[0];
        
        // Проверка типа
        if (param == "true") return AnyValue(true);
        if (param == "false") return AnyValue(false);
        
        bool okInt;
        int intVal = param.toInt(&okInt);
        if (okInt) return AnyValue(intVal);
        
        bool okDouble;
        double doubleVal = param.toDouble(&okDouble);
        if (okDouble) return AnyValue(doubleVal);
        
        // Строка (в кавычках)
        if (param.startsWith("\"") && param.endsWith("\"")) {
            return AnyValue(param.mid(1, param.size() - 2));
        }
        
        return AnyValue(param);
    } else {
        // Несколько параметров
        AnyValueList list;
        for (const std::string& param : params) {
            list.push_back(extractAnnotationParameters(param));
        }
        return AnyValue(list);
    }
}

// ============================================================================
// Валидация аргументов
// ============================================================================

std::vector<std::any> ArgumentValidator::validateArguments(
    const std::vector<std::any>& rules,
    const std::vector<std::any>& actualValues,
    const Qvector<string>& argNames) {
    
    std::vector<std::any> results;
    validationCount++;
    
        
    for (int i = 0; i < rules.size() && i < actualValues.size(); i++) {
        const ValidationRule& rule = rules[i];
        AnyValue value = actualValues[i];
        
        // Применение значения по умолчанию
        if (value.isNull() && rule.defaultValue.isValid()) {
            value = rule.defaultValue;
        }
        
        // Проверка на обязательность
        if (value.isNull() && !rule.isOptional) {
            results.push_back(ValidationResult::failure(
                rule.argumentName,
                "Argument '" + rule.argumentName + "' is required",
                rule.validator));
            failureCount++;
                        continue;
        }
        
        // Пропуск необязательных пустых аргументов
        if (value.isNull() && rule.isOptional) {
            results.push_back(ValidationResult::success(rule.argumentName, AnyValue()));
            successCount++;
            continue;
        }
        
        // Валидация
        ValidationResult result = validateArgument(rule, value);
        results.push_back(result);
        
        if (result.isValid) {
            successCount++;
        } else {
            failureCount++;
                    }
    }
    
    bool allValid = std::all_of(results.begin(), results.end(),
        [](const ValidationResult& r) { return r.isValid; });
    
        
    LOG_DEBUG("Validation completed: " + std::to_string(successCount) + 
              "/" + std::to_string(validationCount) + " successful");
    
    return results;
}

ArgumentValidator::ValidationResult ArgumentValidator::validateArgument(
    const ValidationRule& rule,
    const AnyValue& value) {
    
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
            AnyValueList params = rule.parameters.toList();
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
            AnyValueList allowedValues = ruleParameters.toList();
            if (!isOneOf(value, allowedValues)) {
                return ValidationResult::failure(rule.argumentName,
                    generateErrorMessage(rule, value), rule.validator);
            }
            break;
        }
        
        case ValidatorType::MatchesPattern: {
            std::string pattern = rule.parameters.toString();
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

bool ArgumentValidator::isLogical(const AnyValue& value) {
    return value.canConvert<bool>();
}

bool ArgumentValidator::isPositive(const AnyValue& value) {
    if (!isNumeric(value)) return false;
    return value.toDouble() > 0;
}

bool ArgumentValidator::isNegative(const AnyValue& value) {
    if (!isNumeric(value)) return false;
    return value.toDouble() < 0;
}

bool ArgumentValidator::isNonNegative(const AnyValue& value) {
    if (!isNumeric(value)) return false;
    return value.toDouble() >= 0;
}

bool ArgumentValidator::isInteger(const AnyValue& value) {
    if (!isNumeric(value)) return false;
    double d = value.toDouble();
    return d == static_cast<int>(d);
}

bool ArgumentValidator::isReal(const AnyValue& value) {
    return value.canConvert<double>();
}

bool ArgumentValidator::isNumeric(const AnyValue& value) {
    return value.canConvert<int>() || value.canConvert<double>() ||
           value.canConvert<float>() || value.canConvert<qlonglong>();
}

bool ArgumentValidator::isString(const AnyValue& value) {
    return value.canConvert<QString>();
}

bool ArgumentValidator::isVector(const AnyValue& value) {
    return value.canConvert<AnyValueList>();
}

bool ArgumentValidator::isMatrix(const AnyValue& value) {
    if (!value.canConvert<AnyValueList>()) return false;
    AnyValueList list = value.toList();
    if (list.isEmpty()) return false;
    return list[0].canConvert<AnyValueList>();
}

bool ArgumentValidator::isLayer(const AnyValue& value) {
    if (!value.canConvert<AnyValueList>()) return false;
    AnyValueList list = value.toList();
    if (list.isEmpty()) return false;
    if (!list[0].canConvert<AnyValueList>()) return false;
    AnyValueList list2d = list[0].toList();
    if (list2d.isEmpty()) return false;
    return list2d[0].canConvert<AnyValueList>();
}

bool ArgumentValidator::isCollection(const AnyValue& value) {
    return value.canConvert<AnyValueMap>();
}

bool ArgumentValidator::isEmpty(const AnyValue& value) {
    if (value.isNull()) return true;
    if (value.canConvert<QString>()) return value.toString().isEmpty();
    if (value.canConvert<AnyValueList>()) return value.toList().isEmpty();
    if (value.canConvert<AnyValueMap>()) return value.toMap().isEmpty();
    return false;
}

bool ArgumentValidator::isNotEmpty(const AnyValue& value) {
    return !isEmpty(value);
}

bool ArgumentValidator::hasLength(const AnyValue& value) {
    if (value.canConvert<QString>()) return true;
    if (value.canConvert<AnyValueList>()) return true;
    return false;
}

bool ArgumentValidator::hasSize(const AnyValue& value) {
    return hasLength(value) || isMatrix(value) || isLayer(value);
}

bool ArgumentValidator::isInRange(const AnyValue& value, double min, double max) {
    if (!isNumeric(value)) return false;
    double d = value.toDouble();
    return d >= min && d <= max;
}

bool ArgumentValidator::isOneOf(const AnyValue& value, const std::vector<std::any>& allowedValues) {
    for (const AnyValue& allowed : allowedValues) {
        if (value == allowed) return true;
    }
    return false;
}

bool ArgumentValidator::matchesPattern(const AnyValue& value, const std::string& pattern) {
    if (!value.canConvert<QString>()) return false;
    QRegularExpression re(pattern);
    return re.match(value.toString()).hasMatch();
}

bool ArgumentValidator::isClass(const AnyValue& value, const std::string& className) {
    // В полной реализации - проверка типа объекта
    return false;
}

bool ArgumentValidator::isInterface(const AnyValue& value, const std::string& interfaceName) {
    // В полной реализации - проверка реализации интерфейса
    return false;
}

bool ArgumentValidator::isCallable(const AnyValue& value) {
    // В полной реализации - проверка на функцию/метод
    return value.canConvert<QFunctionPointer>();
}

bool ArgumentValidator::isIterable(const AnyValue& value) {
    return value.canConvert<AnyValueList>() || 
           value.canConvert<QString>() ||
           value.canConvert<AnyValueMap>();
}

// ============================================================================
// Регистрация пользовательских валидаторов
// ============================================================================

void ArgumentValidator::registerCustomValidator(
    const std::string& name,
    std::function<bool(const AnyValue&)> validatorFunc) {
    
    customValidators[name] = validatorFunc;
    LOG_DEBUG("Custom validator registered: " + name);
}

bool ArgumentValidator::hasCustomValidator(const std::string& name) const {
    return customValidators.contains(name);
}

bool ArgumentValidator::executeCustomValidator(const std::string& name, const AnyValue& value) {
    if (!hasCustomValidator(name)) {
        LOG_WARNING("Custom validator not found: " + name);
        return false;
    }
    
    return customValidators[name](value);
}

// ============================================================================
// Генерация сообщений об ошибках
// ============================================================================

std::string ArgumentValidator::generateErrorMessage(
    const ValidationRule& rule,
    const AnyValue& value) {
    
    if (!rule.errorMessage.isEmpty()) {
        return rule.errorMessage;
    }
    
    std::string errorMsg = "Argument '" + rule.argumentName + "' validation failed: ";
    
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
            AnyValueList params = rule.parameters.toList();
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

std::string ArgumentValidator::getValidatorDescription(ValidatorType type) {
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

std::string ArgumentValidator::getValidatorName(ValidatorType type) {
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
    const std::string& validatorName) {
    
    std::string name = validatorName.toLower();
    
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

std::string ArgumentValidator::validatorTypeToString(ValidatorType type) {
    return getValidatorName(type);
}

} // namespace proxima