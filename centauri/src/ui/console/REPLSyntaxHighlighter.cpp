#include "REPLSyntaxHighlighter.h"
#include <QTextBlock>
#include "utils/Logger.h"

namespace proxima {

REPLSyntaxHighlighter::REPLSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , prompt(">> ")
    , repl(nullptr) {
    
    setupFormats();
    setupRules();
    
    LOG_DEBUG("REPLSyntaxHighlighter initialized");
}

REPLSyntaxHighlighter::~REPLSyntaxHighlighter() {
    LOG_DEBUG("REPLSyntaxHighlighter destroyed");
}

void REPLSyntaxHighlighter::setPrompt(const QString& p) {
    prompt = p;
}

void REPLSyntaxHighlighter::setREPL(REPL* r) {
    repl = r;
}

void REPLSyntaxHighlighter::setPromptFormat(const QTextCharFormat& format) {
    promptFormat = format;
}

void REPLSyntaxHighlighter::setInputFormat(const QTextCharFormat& format) {
    inputFormat = format;
}

void REPLSyntaxHighlighter::setOutputFormat(const QTextCharFormat& format) {
    outputFormat = format;
}

void REPLSyntaxHighlighter::setErrorFormat(const QTextCharFormat& format) {
    errorFormat = format;
}

void REPLSyntaxHighlighter::setCommandFormat(const QTextCharFormat& format) {
    commandFormat = format;
}

void REPLSyntaxHighlighter::setupFormats() {
    // Prompt - фиолетовый, жирный
    promptFormat.setForeground(QColor(197, 134, 192));  // #C586C0
    promptFormat.setFontWeight(QFont::Bold);
    
    // Ввод пользователя - белый
    inputFormat.setForeground(QColor(212, 212, 212));  // #D4D4D4
    
    // Вывод - бирюзовый
    outputFormat.setForeground(QColor(78, 201, 176));  // #4EC9B0
    
    // Ошибки - красный
    errorFormat.setForeground(QColor(244, 71, 71));  // #F44747
    errorFormat.setBackground(QColor(60, 30, 30));
    
    // Команды REPL - розовый
    commandFormat.setForeground(QColor(220, 120, 180));
    commandFormat.setFontWeight(QFont::Bold);
    
    // Ключевые слова - синий
    keywordFormat.setForeground(QColor(56, 139, 253));
    keywordFormat.setFontWeight(QFont::Bold);
    
    // Типы - бирюзовый
    typeFormat.setForeground(QColor(78, 201, 176));
    
    // Строки - оранжевый
    stringFormat.setForeground(QColor(206, 145, 120));
    
    // Числа - зелёный
    numberFormat.setForeground(QColor(181, 206, 168));
    
    // Комментарии - серый
    commentFormat.setForeground(QColor(100, 100, 100));
    commentFormat.setFontItalic(true);
}

void REPLSyntaxHighlighter::setupRules() {
    // Ключевые слова Proxima
    QStringList keywords = {
        "if", "elseif", "else", "end", "for", "in", "while", "do",
        "return", "continue", "break", "class", "interface", "function",
        "true", "false", "null", "nan", "inf"
    };
    
    for (const QString& keyword : keywords) {
        REPLRule rule;
        rule.pattern = QRegularExpression("\\b" + QRegularExpression::escape(keyword) + "\\b");
        rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        rules.append(rule);
    }
    
    // Типы
    QStringList types = {
        "int", "int32", "int64", "single", "double", "bool",
        "string", "vector", "matrix", "collection", "time"
    };
    
    for (const QString& type : types) {
        REPLRule rule;
        rule.pattern = QRegularExpression("\\b" + QRegularExpression::escape(type) + "\\b");
        rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        rule.format = typeFormat;
        rules.append(rule);
    }
    
    // Строки
    REPLRule rule;
    rule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
    rule.format = stringFormat;
    rules.append(rule);
    
    // Числа
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    rules.append(rule);
    
    // Комментарии
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    rules.append(rule);
}

void REPLSyntaxHighlighter::highlightBlock(const QString &text) {
    // Проверка на prompt
    if (text.startsWith(prompt)) {
        // Подсветка prompt
        setFormat(0, prompt.length(), promptFormat);
        
        // Остальной текст - ввод пользователя
        QString input = text.mid(prompt.length());
        
        // Подсветка синтаксиса ввода
        for (const REPLRule &rule : rules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(input);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(prompt.length() + match.capturedStart(), 
                         match.capturedLength(), 
                         rule.format);
            }
        }
        
        // Проверка на команду REPL (начинается с !)
        if (input.trimmed().startsWith("!")) {
            // Подсветка команды
            QStringList parts = input.trimmed().split(" ");
            if (!parts.isEmpty()) {
                QString cmd = parts[0];
                setFormat(prompt.length(), cmd.length(), commandFormat);
            }
        }
    }
    // Проверка на вывод (начинается с определённых паттернов)
    else if (text.contains("Error:") || text.contains("error:")) {
        setFormat(0, text.length(), errorFormat);
    }
    else if (text.contains(">>") || text.contains("ans =")) {
        setFormat(0, text.length(), outputFormat);
    }
    // Обычный текст - без подсветки или минимальная
    else {
        // Применение правил ко всему тексту
        for (const REPLRule &rule : rules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }
}

} // namespace proxima