#ifndef CENTAURI_REPL_SYNTAXHIGHLIGHTER_H
#define CENTAURI_REPL_SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QColor>
#include "runtime/REPL.h"

namespace proxima {

/**
 * @brief Подсветка синтаксиса для REPL консоли
 * 
 * Отличается от обычной подсветки:
 * - Подсветка prompt
 * - Подсветка ввода пользователя
 * - Подсветка вывода
 * - Подсветка ошибок
 * - Более компактные форматы
 */
class REPLSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    
public:
    explicit REPLSyntaxHighlighter(QTextDocument *parent = nullptr);
    ~REPLSyntaxHighlighter();
    
    // Настройка
    void setPrompt(const QString& prompt);
    QString getPrompt() const { return prompt; }
    
    void setREPL(REPL* repl);
    
    // Форматы
    void setPromptFormat(const QTextCharFormat& format);
    void setInputFormat(const QTextCharFormat& format);
    void setOutputFormat(const QTextCharFormat& format);
    void setErrorFormat(const QTextCharFormat& format);
    void setCommandFormat(const QTextCharFormat& format);
    
protected:
    void highlightBlock(const QString &text) override;
    
private:
    void setupFormats();
    void setupRules();
    
    QTextCharFormat promptFormat;
    QTextCharFormat inputFormat;
    QTextCharFormat outputFormat;
    QTextCharFormat errorFormat;
    QTextCharFormat commandFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat commentFormat;
    
    QString prompt;
    REPL* repl;
    
    struct REPLRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    
    QVector<REPLRule> rules;
};

} // namespace proxima

#endif // CENTAURI_REPL_SYNTAXHIGHLIGHTER_H