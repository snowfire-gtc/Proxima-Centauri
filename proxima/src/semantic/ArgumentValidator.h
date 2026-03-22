#ifndef PROXIMA_ARGUMENT_VALIDATOR_H
#define PROXIMA_ARGUMENT_VALIDATOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QVariant>
#include <QRegularExpression>
#include <QDateTime>
#include "parser/AST.h"
#include "semantic/TypeChecker.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Типы валидаторов аргументов
 * 
 * Согласно требованию language.txt пункт 27:
 * Стандартные функции валидации аргументов
 */
enum class ValidatorType {
    IsLogical,          // @islogical - логическое значение
    IsPositive,         // @ispositive - положительное число
    IsNegative,         // @isnegative - отрицательное число
    IsNonNegative,      // @isnonnegative - неотрицательное число
    IsInteger,          // @isinteger - целое число
    IsReal,             // @isreal - вещественное число
    IsNumeric,          // @isnumeric - числовое значение
    IsString,           // @isstring - строка
    IsVector,           // @isvector - вектор
    IsMatrix,           // @ismatrix - матрица
    IsLayer,            // @islayer - слой
    IsCollection,       // @iscollection - коллекция
    IsClass,            // @isclass - экземпляр класса
    IsInterface,        // @isinterface - экземпляр интерфейса
    IsCallable,         // @iscallable - вызываемый объект
    IsIterable,         // @isiterable - итерируемый объект
    HasLength,          // @haslength - имеет длину
    HasSize,            // @hassize - имеет размер
    IsEmpty,            // @isempty - пустой
    IsNotEmpty,         // @isnotempty - не пустой
    IsInRange,          // @isinrange - в диапазоне
    IsOneOf,            // @isoneof - одно из значений
    MatchesPattern,     // @matchespattern - соответствует шаблону
    Custom              // @custom - пользовательский валидатор
};

/**
 * @brief Структура правила валидации
 */
struct ValidationRule {
    QString argumentName;       // Имя аргумента
    ValidatorType validator;    // Тип валидатора
    QString validatorName;      // Имя валидатора (для custom)
    QVariant parameters;        // Параметры валидатора
    QString errorMessage;       // Сообщение об ошибке
    bool isOptional;            // Необязательный аргумент
    QVariant defaultValue;      // Значение по умолчанию
    int line;                   // Номер строки
    int column;                 // Номер колонки
    
    ValidationRule() : validator(ValidatorType::Custom), isOptional(false), 
                       line(0), column(0) {}
};

/**
 * @brief Результат валидации
 */
struct ValidationResult {
    bool isValid;               // Успешная валидация
    QString argumentName;       // Имя аргумента
    QString errorMessage;       // Сообщение об ошибке
    QVariant validatedValue;    // Валидированное значение
    ValidatorType validator;    // Использованный валидатор
    
    ValidationResult() : isValid(true), validator(ValidatorType::Custom) {}
    
    static ValidationResult success(const QString& argName, const QVariant& value) {
        ValidationResult result;
        result.isValid = true;
        result.argumentName = argName;
        result.validatedValue = value;
        return result;
    }
    
    static ValidationResult failure(const QString& argName, const QString& error, 
                                   ValidatorType validator = ValidatorType::Custom) {
        ValidationResult result;
        result.isValid = false;
        result.argumentName = argName;
        result.errorMessage = error;
        result.validator = validator;
        return result;
    }
};

/**
 * @brief Класс валидации аргументов функций
 * 
 * Согласно требованиям:
 * - language.txt пункт 26 - конструкция arguments для валидации
 * - language.txt пункт 27 - стандартные функции валидации
 */
class ArgumentValidator : public QObject {
    Q_OBJECT
    
public:
    explicit ArgumentValidator(QObject *parent = nullptr);
    ~ArgumentValidator();
    
    // ========================================================================
    // Парсинг конструкции arguments
    // ========================================================================
    
    /**
     * @brief Парсинг блока arguments из AST
     * @param argumentsNode Узел arguments из AST
     * @param functionNode Узел функции
     * @return Вектор правил валидации
     */
    QVector<ValidationRule> parseArgumentsBlock(
        ArgumentsNodePtr argumentsNode,
        FunctionDeclNodePtr functionNode);
    
    /**
     * @brief Парсинг аннотаций валидации из комментариев
     * @param commentText Текст комментария
     * @param argName Имя аргумента
     * @return Вектор правил валидации
     */
    QVector<ValidationRule> parseValidationAnnotations(
        const QString& commentText,
        const QString& argName);
    
    // ========================================================================
    // Валидация аргументов
    // ========================================================================
    
    /**
     * @brief Валидация всех аргументов функции
     * @param rules Правила валидации
     * @param actualValues Фактические значения аргументов
     * @param argNames Имена аргументов
     * @return Результаты валидации
     */
    QVector<ValidationResult> validateArguments(
        const QVector<ValidationRule>& rules,
        const QVector<QVariant>& actualValues,
        const QStringList& argNames);
    
    /**
     * @brief Валидация отдельного аргумента
     * @param rule Правило валидации
     * @param value Значение аргумента
     * @return Результат валидации
     */
    ValidationResult validateArgument(const ValidationRule& rule, const QVariant& value);
    
    // ========================================================================
    // Стандартные валидаторы
    // ========================================================================
    
    /**
     * @brief Проверка на логическое значение
     * @param value Значение
     * @return true если логическое
     */
    static bool isLogical(const QVariant& value);
    
    /**
     * @brief Проверка на положительное число
     * @param value Значение
     * @return true если положительное
     */
    static bool isPositive(const QVariant& value);
    
    /**
     * @brief Проверка на отрицательное число
     * @param value Значение
     * @return true если отрицательное
     */
    static bool isNegative(const QVariant& value);
    
    /**
     * @brief Проверка на неотрицательное число
     * @param value Значение
     * @return true если неотрицательное
     */
    static bool isNonNegative(const QVariant& value);
    
    /**
     * @brief Проверка на целое число
     * @param value Значение
     * @return true если целое
     */
    static bool isInteger(const QVariant& value);
    
    /**
     * @brief Проверка на вещественное число
     * @param value Значение
     * @return true если вещественное
     */
    static bool isReal(const QVariant& value);
    
    /**
     * @brief Проверка на числовое значение
     * @param value Значение
     * @return true если числовое
     */
    static bool isNumeric(const QVariant& value);
    
    /**
     * @brief Проверка на строку
     * @param value Значение
     * @return true если строка
     */
    static bool isString(const QVariant& value);
    
    /**
     * @brief Проверка на вектор
     * @param value Значение
     * @return true если вектор
     */
    static bool isVector(const QVariant& value);
    
    /**
     * @brief Проверка на матрицу
     * @param value Значение
     * @return true если матрица
     */
    static bool isMatrix(const QVariant& value);
    
    /**
     * @brief Проверка на слой
     * @param value Значение
     * @return true если слой
     */
    static bool isLayer(const QVariant& value);
    
    /**
     * @brief Проверка на коллекцию
     * @param value Значение
     * @return true если коллекция
     */
    static bool isCollection(const QVariant& value);
    
    /**
     * @brief Проверка на пустоту
     * @param value Значение
     * @return true если пустое
     */
    static bool isEmpty(const QVariant& value);
    
    /**
     * @brief Проверка на непустоту
     * @param value Значение
     * @return true если не пустое
     */
    static bool isNotEmpty(const QVariant& value);
    
    /**
     * @brief Проверка на наличие длины
     * @param value Значение
     * @return true если имеет длину
     */
    static bool hasLength(const QVariant& value);
    
    /**
     * @brief Проверка на наличие размера
     * @param value Значение
     * @return true если имеет размер
     */
    static bool hasSize(const QVariant& value);
    
    /**
     * @brief Проверка на диапазон
     * @param value Значение
     * @param min Минимальное значение
     * @param max Максимальное значение
     * @return true если в диапазоне
     */
    static bool isInRange(const QVariant& value, double min, double max);
    
    /**
     * @brief Проверка на одно из значений
     * @param value Значение
     * @param allowedValues Разрешённые значения
     * @return true если одно из разрешённых
     */
    static bool isOneOf(const QVariant& value, const QVector<QVariant>& allowedValues);
    
    /**
     * @brief Проверка на соответствие шаблону
     * @param value Значение
     * @param pattern Регулярное выражение
     * @return true если соответствует
     */
    static bool matchesPattern(const QVariant& value, const QString& pattern);
    
    /**
     * @brief Проверка на класс
     * @param value Значение
     * @param className Имя класса
     * @return true если экземпляр класса
     */
    static bool isClass(const QVariant& value, const QString& className);
    
    /**
     * @brief Проверка на интерфейс
     * @param value Значение
     * @param interfaceName Имя интерфейса
     * @return true если экземпляр интерфейса
     */
    static bool isInterface(const QVariant& value, const QString& interfaceName);
    
    /**
     * @brief Проверка на вызываемость
     * @param value Значение
     * @return true если вызываемое
     */
    static bool isCallable(const QVariant& value);
    
    /**
     * @brief Проверка на итерируемость
     * @param value Значение
     * @return true если итерируемое
     */
    static bool isIterable(const QVariant& value);
    
    // ========================================================================
    // Регистрация пользовательских валидаторов
    // ========================================================================
    
    /**
     * @brief Регистрация пользовательского валидатора
     * @param name Имя валидатора
     * @param validatorFunc Функция валидатора
     */
    void registerCustomValidator(const QString& name, 
                                 std::function<bool(const QVariant&)> validatorFunc);
    
    /**
     * @brief Проверка наличия пользовательского валидатора
     * @param name Имя валидатора
     * @return true если зарегистрирован
     */
    bool hasCustomValidator(const QString& name) const;
    
    /**
     * @brief Выполнение пользовательского валидатора
     * @param name Имя валидатора
     * @param value Значение
     * @return true если валидация успешна
     */
    bool executeCustomValidator(const QString& name, const QVariant& value);
    
    // ========================================================================
    // Генерация сообщений об ошибках
    // ========================================================================
    
    /**
     * @brief Генерация сообщения об ошибке валидации
     * @param rule Правило валидации
     * @param value Фактическое значение
     * @return Сообщение об ошибке
     */
    QString generateErrorMessage(const ValidationRule& rule, const QVariant& value);
    
    /**
     * @brief Получение описания валидатора
     * @param type Тип валидатора
     * @return Описание
     */
    static QString getValidatorDescription(ValidatorType type);
    
    /**
     * @brief Получение имени валидатора
     * @param type Тип валидатора
     * @return Имя
     */
    static QString getValidatorName(ValidatorType type);
    
signals:
    void validationStarted(const QString& functionName);
    void validationCompleted(const QString& functionName, bool success);
    void validationFailed(const QString& functionName, const QString& argument, 
                         const QString& error);
    
private:
    /**
     * @brief Парсинг аннотации валидации
     * @param annotationText Текст аннотации
     * @return Правило валидации
     */
    ValidationRule parseValidationAnnotation(const QString& annotationText);
    
    /**
     * @brief Извлечение параметров из аннотации
     * @param annotationText Текст аннотации
     * @return Параметры
     */
    QVariant extractAnnotationParameters(const QString& annotationText);
    
    /**
     * @brief Преобразование строки в тип валидатора
     * @param validatorName Имя валидатора
     * @return Тип валидатора
     */
    ValidatorType stringToValidatorType(const QString& validatorName);
    
    /**
     * @brief Преобразование типа валидатора в строку
     * @param type Тип валидатора
     * @return Имя валидатора
     */
    QString validatorTypeToString(ValidatorType type);
    
    // Пользовательские валидаторы
    QMap<QString, std::function<bool(const QVariant&)>> customValidators;
    
    // Статистика
    int validationCount;
    int successCount;
    int failureCount;
};

} // namespace proxima

#endif // PROXIMA_ARGUMENT_VALIDATOR_H