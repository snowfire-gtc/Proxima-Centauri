#ifndef CENTAURI_COMPLETIONPROVIDOR_H
#define CENTAURI_COMPLETIONPROVIDOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QTextCursor>
#include "stdlib/StdLib.h"

namespace proxima {

struct CompletionItem {
    QString text;
    QString type;       // "method", "property", "keyword", "variable", "class"
    QString description;
    QString signature;  // Для методов: (arg1: type, arg2: type): returnType
    QString detail;     // Дополнительная информация
    int relevance;      // Приоритет сортировки
    bool isStatic;      // Для методов класса
};

struct CompletionContext {
    QString currentWord;
    QString precedingText;
    bool isMemberAccess;      // После "."
    bool isClassAccess;       // После "ClassName."
    QString className;        // Имя класса для static methods
    QString objectType;       // Тип объекта для instance methods
    int cursorPosition;
    int lineNumber;
    int columnNumber;
};

class CompletionProvider : public QObject {
    Q_OBJECT
    
public:
    static CompletionProvider& getInstance();
    
    // Получение подсказок
    QVector<CompletionItem> getCompletions(const CompletionContext& context);
    
    // Анализ контекста
    CompletionContext analyzeContext(const QString& text, int cursorPosition);
    
    // Регистрация пользовательских типов
    void registerUserType(const QString& typeName,
                         const QStringList& methods,
                         const QStringList& properties,
                         const QString& description);
    
    // Обновление информации о типах из компилятора
    void updateTypeInfo(const QString& typeName,
                       const QStringList& staticMethods,
                       const QStringList& instanceMethods,
                       const QStringList& properties);
    
    // Получение подробной информации о методе
    QString getMethodSignature(const QString& className, const QString& methodName) const;
    QString getMethodDescription(const QString& className, const QString& methodName) const;
    
    // Обучающие подсказки
    QString getLearningTip(const QString& className) const;
    QString getExampleUsage(const QString& className, const QString& methodName) const;
    
    // Фильтрация и сортировка
    QVector<CompletionItem> filterCompletions(const QVector<CompletionItem>& items,
                                             const QString& filter);
    QVector<CompletionItem> sortCompletions(QVector<CompletionItem>& items,
                                           const QString& currentWord);
    
signals:
    void completionsReady(const QVector<CompletionItem>& completions);
    void documentationUpdated(const QString& className, const QString& member);
    
private:
    CompletionProvider();
    ~CompletionProvider();
    CompletionProvider(const CompletionProvider&) = delete;
    CompletionProvider& operator=(const CompletionProvider&) = delete;
    
    void initializeStdLibCompletions();
    void initializeKeywordCompletions();
    void initializeSnippetCompletions();
    
    QVector<CompletionItem> getClassMemberCompletions(const QString& className, 
                                                     bool isStatic);
    QVector<CompletionItem> getVariableCompletions(const QString& currentWord);
    QVector<CompletionItem> getKeywordCompletions(const QString& currentWord);
    QVector<CompletionItem> getSnippetCompletions(const QString& currentWord);
    
    QString buildMethodSignature(const QString& methodName, 
                                const QString& className) const;
    QString formatDescription(const QString& description) const;
    
    struct TypeInfo {
        QStringList staticMethods;
        QStringList instanceMethods;
        QStringList properties;
        QString description;
        QMap<QString, QString> methodSignatures;
        QMap<QString, QString> methodDescriptions;
    };
    
    QMap<QString, TypeInfo> typeInfo;
    QVector<CompletionItem> keywordCompletions;
    QVector<CompletionItem> snippetCompletions;
    
    mutable QMap<QString, CompletionItem> completionCache;
};

} // namespace proxima

#endif // CENTAURI_COMPLETIONPROVIDOR_H