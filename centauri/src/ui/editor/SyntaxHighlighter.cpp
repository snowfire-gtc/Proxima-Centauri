#include "SyntaxHighlighter.h"
#include <QTextBlock>
#include <QDateTime>
#include <QDebug>
#include "utils/Logger.h"

namespace proxima {

// ============================================================================
// Статические списки ключевых слов (согласно language.txt)
// ============================================================================

const QStringList SyntaxHighlighter::keywords = {
    // Управление потоком (language.txt пункт 45)
    "if", "elseif", "else", "end", "for", "in", "while", "do",
    "switch", "case", "default", "return", "continue", "break",
    
    // Классы и ООП (language.txt пункт 23)
    "class", "interface", "template", "public", "protected", "private",
    "constructor", "destructor", "extends", "implements",
    
    // Пространства имён (language.txt пункт 37)
    "namespace", "using", "include", "exclude",
    
    // Компиляция (language.txt пункт 37)
    "define", "ifdef", "endif", "ifndef", "undef",
    
    // Типы (language.txt пункт 22)
    "auto", "void", "type", "method", "rtti",
    
    // Аргументы (language.txt пункт 26)
    "arguments",
    
    // Параллелизм (language.txt пункт 17)
    "parallel",
    
    // Литералы (language.txt пункт 2, 4)
    "true", "false", "null", "nan", "inf", "ninf",
    
    // Константы (language.txt пункт 2)
    "pi", "pi2", "exp",
    
    // Регионы (language.txt пункт 47, ide.txt пункт 31)
    "region", "endregion",
    
    // Тестирование (language.txt пункт 49)
    "suite", "test", "assert",
    
    // GEM интерфейс (language.txt пункт 44)
    "gem",
    
    // Отладка (language.txt пункт 43)
    "dbgstop", "dbgprint", "dbgcontext", "dbgstack"
};

const QStringList SyntaxHighlighter::types = {
    // Целочисленные типы (language.txt пункт 3)
    "int4", "int8", "int16", "int32", "int64", "int",
    
    // Числа с плавающей точкой (language.txt пункт 2)
    "single", "double", "float",
    
    // Логический и символьный (language.txt пункт 4, 9)
    "bool", "char",
    
    // Строковый (language.txt пункт 8)
    "string",
    
    // Вектора и матрицы (language.txt пункт 5, 6, 5.1, 5.2)
    "vector", "matrix", "layer",
    "point2", "point3", "point4",
    "matrix22", "matrix33", "matrix44", "matrix34",
    
    // Коллекции (language.txt пункт 7)
    "collection",
    
    // Специальные типы (language.txt пункт 2.1, 15, 17)
    "time", "file", "parallel", "document",
    
    // RTTI (language.txt пункт 18, 32)
    "rtti"
};

const QStringList SyntaxHighlighter::builtins = {
    // Ввод-вывод (language.txt пункт 28, 29)
    "print", "write", "read", "open", "close",
    
    // Матричные операции (language.txt пункт 6, 13)
    "zeros", "ones", "eye", "rand", "size", "length",
    "sum", "mean", "max", "min", "abs", "sqrt", "exp", "log", "log10",
    
    // Тригонометрия
    "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
    
    // Время (language.txt пункт 2.1)
    "time_now", "sleep",
    
    // RTTI (language.txt пункт 32)
    "type", "rtti",
    
    // Отладка (language.txt пункт 43)
    "dbgstop", "dbgprint", "dbgcontext", "dbgstack",
    
    // Утверждения (language.txt пункт 49)
    "assert",
    
    // GPU (language.txt пункт 17)
    "parallel.set", "parallel.get", "parallel.copy"
};

const QStringList SyntaxHighlighter::operators = {
    // Арифметические (language.txt пункт 36)
    "+", "-", "*", "/", "\\", "%", "^",
    
    // Логические (language.txt пункт 34)
    "!", "!=", "==", "===", "&", "|", "%",
    ">", "<", ">=", "<=",
    
    // Поэлементные (language.txt пункт 34)
    ".=", ".==", ".&", ".|", ".>", ".<", ".>=", ".<=",
    
    // Бинарные (language.txt пункт 35)
    "!!", "&&", "||", "%%", "<<", ">>",
    
    // Другие (language.txt пункт 13, 19, 22.3)
    ".", ",", ";", ":", "=", "'", "?",
    "[", "]", "(", ")", "{", "}",
    
    // Конкатенация (language.txt пункт 10, 11, 12)
    ",,", ",,,"
};

const QStringList SyntaxHighlighter::directives = {
    // Директивы (language.txt пункт 37)
    "include", "exclude", "namespace", "using",
    "define", "ifdef", "endif", "ifndef", "undef"
};

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , filePath("")
    , displayMode(DisplayMode::Standard)
    , keywordCount(0)
    , typeCount(0)
    , functionCount(0) {
    
    setupProximaRules();
    
    LOG_DEBUG("SyntaxHighlighter initialized for Proxima language");
}

SyntaxHighlighter::~SyntaxHighlighter() {
    LOG_DEBUG("SyntaxHighlighter destroyed");
}

// ============================================================================
// Настройка
// ============================================================================

void SyntaxHighlighter::setFilePath(const QString& path) {
    filePath = path;
}

void SyntaxHighlighter::setDisplayMode(DisplayMode mode) {
    if (displayMode != mode) {
        displayMode = mode;
        rehighlight();
    }
}

void SyntaxHighlighter::setTypeInfo(const QMap<int, QString>& types) {
    typeInfo = types;
    if (displayMode == DisplayMode::TypeHighlight) {
        rehighlight();
    }
}

void SyntaxHighlighter::setDimensionInfo(const QMap<int, QString>& dimensions) {
    dimensionInfo = dimensions;
    if (displayMode == DisplayMode::DimensionHighlight) {
        rehighlight();
    }
}

void SyntaxHighlighter::setTimingInfo(const QMap<int, double>& timings) {
    timingInfo = timings;
    if (displayMode == DisplayMode::TimingHighlight) {
        rehighlight();
    }
}

void SyntaxHighlighter::setFrequencyInfo(const QMap<int, int>& frequencies) {
    frequencyInfo = frequencies;
    if (displayMode == DisplayMode::FrequencyHighlight) {
        rehighlight();
    }
}

void SyntaxHighlighter::setGitDiffInfo(const QString& diff) {
    gitDiffInfo = diff;
    if (displayMode == DisplayMode::GitDiff) {
        rehighlight();
    }
}

void SyntaxHighlighter::setAuthorInfo(const QMap<int, QString>& authors) {
    authorInfo = authors;
    if (displayMode == DisplayMode::AuthorHighlight) {
        rehighlight();
    }
}

void SyntaxHighlighter::setAgeInfo(const QMap<int, QDateTime>& ages) {
    ageInfo = ages;
    if (displayMode == DisplayMode::AgeHighlight) {
        rehighlight();
    }
}

void SyntaxHighlighter::rehighlightBlock(QTextBlock block) {
    if (block.isValid()) {
        highlightBlock(block.text());
    }
}

void SyntaxHighlighter::rehighlightRange(int start, int end) {
    QTextDocument* doc = document();
    if (!doc) return;
    
    QTextBlock block = doc->findBlock(start);
    while (block.isValid() && block.position() < end) {
        rehighlightBlock(block);
        block = block.next();
    }
}

// ============================================================================
// Инициализация правил подсветки
// ============================================================================

void SyntaxHighlighter::setupProximaRules() {
    // Инициализация форматов
    
    // Ключевые слова - фиолетовый/синий
    keywordFormat.setForeground(QColor(56, 139, 253));  // #388BFD
    keywordFormat.setFontWeight(QFont::Bold);
    
    // Типы - бирюзовый
    typeFormat.setForeground(QColor(78, 201, 176));  // #4EC9B0
    typeFormat.setFontWeight(QFont::Bold);
    
    // Функции - жёлтый
    functionFormat.setForeground(QColor(220, 220, 170));  // #DCDCAA
    
    // Комментарии - зелёный
    commentFormat.setForeground(QColor(106, 135, 89));  // #6A8759
    commentFormat.setFontItalic(true);
    
    // Документационные комментарии - более яркий зелёный
    docCommentFormat.setForeground(QColor(120, 160, 100));
    docCommentFormat.setFontItalic(true);
    
    // Строки - оранжевый
    stringFormat.setForeground(QColor(206, 145, 120));  // #CE9178
    
    // Символы - оранжевый
    charFormat.setForeground(QColor(206, 145, 120));
    
    // Числа - светло-зелёный
    numberFormat.setForeground(QColor(181, 206, 168));  // #B5CEA8
    
    // Операторы - белый
    operatorFormat.setForeground(QColor(212, 212, 212));  // #D4D4D4
    
    // Директивы - розовый
    directiveFormat.setForeground(QColor(197, 134, 192));  // #C586C0
    
    // Препроцессор - розовый
    preprocessorFormat.setForeground(QColor(197, 134, 192));
    
    // Аннотации - голубой
    annotationFormat.setForeground(QColor(80, 180, 220));
    
    // Форматы режимов
    typeHighlightFormat.setBackground(QColor(50, 50, 80));
    typeHighlightFormat.setForeground(QColor(150, 150, 255));
    
    dimensionHighlightFormat.setBackground(QColor(50, 80, 50));
    dimensionHighlightFormat.setForeground(QColor(150, 255, 150));
    
    timingHighlightFormat.setBackground(QColor(80, 50, 50));
    timingHighlightFormat.setForeground(QColor(255, 150, 150));
    
    frequencyHighlightFormat.setBackground(QColor(80, 80, 50));
    frequencyHighlightFormat.setForeground(QColor(255, 255, 150));
    
    gitAddFormat.setBackground(QColor(50, 80, 50));
    gitRemoveFormat.setBackground(QColor(80, 50, 50));
    gitModifyFormat.setBackground(QColor(80, 80, 50));
    
    authorHighlightFormat.setBackground(QColor(60, 60, 90));
    authorHighlightFormat.setForeground(QColor(200, 200, 255));
    
    ageHighlightFormat.setBackground(QColor(70, 60, 50));
    ageHighlightFormat.setForeground(QColor(255, 230, 200));
    
    // Создание правил
    setupKeywordRules();
    setupTypeRules();
    setupFunctionRules();
    setupCommentRules();
    setupStringRules();
    setupNumberRules();
    setupOperatorRules();
    setupDirectiveRules();
    setupAnnotationRules();
}

void SyntaxHighlighter::setupKeywordRules() {
    for (const QString& keyword : keywords) {
        HighlightingRule rule;
        // Границы слова - чтобы не подсвечивать части идентификаторов
        rule.pattern = QRegularExpression("\\b" + QRegularExpression::escape(keyword) + "\\b");
        rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        rule.description = "Keyword: " + keyword;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupTypeRules() {
    for (const QString& type : types) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + QRegularExpression::escape(type) + "\\b");
        rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        rule.format = typeFormat;
        rule.description = "Type: " + type;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupFunctionRules() {
    // Встроенные функции
    for (const QString& builtin : builtins) {
        HighlightingRule rule;
        // Функция за которой следуют скобки
        rule.pattern = QRegularExpression("\\b" + QRegularExpression::escape(builtin) + "\\s*(?=\\()");
        rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        rule.format = functionFormat;
        rule.description = "Function: " + builtin;
        highlightingRules.append(rule);
    }
    
    // Пользовательские функции (идентификатор перед скобками)
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?=\\()");
    rule.format = functionFormat;
    rule.description = "Function call";
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupCommentRules() {
    // Однострочные комментарии (language.txt)
    HighlightingRule rule;
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    rule.description = "Single-line comment";
    highlightingRules.append(rule);
    
    // Многострочные комментарии обрабатываются отдельно в highlightBlock
    // Для поддержки вложенных комментариев с номерами (language.txt пункт 31)
}

void SyntaxHighlighter::setupStringRules() {
    // Строки в двойных кавычках (language.txt пункт 8)
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
    rule.format = stringFormat;
    rule.description = "String literal";
    highlightingRules.append(rule);
    
    // Символы в одинарных кавычках (language.txt пункт 9)
    rule.pattern = QRegularExpression("'(?:[^'\\\\]|\\\\.)*'");
    rule.format = charFormat;
    rule.description = "Char literal";
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupNumberRules() {
    // Целые числа (language.txt пункт 3)
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b\\d+\\b");
    rule.format = numberFormat;
    rule.description = "Integer literal";
    highlightingRules.append(rule);
    
    // Числа с плавающей точкой (language.txt пункт 2)
    rule.pattern = QRegularExpression("\\b\\d+\\.\\d*([eE][+-]?\\d+)?\\b");
    rule.format = numberFormat;
    rule.description = "Float literal";
    highlightingRules.append(rule);
    
    // Научная нотация
    rule.pattern = QRegularExpression("\\b\\d+[eE][+-]?\\d+\\b");
    rule.format = numberFormat;
    rule.description = "Scientific notation";
    highlightingRules.append(rule);
    
    // Специальные значения (language.txt пункт 2)
    rule.pattern = QRegularExpression("\\b(nan|inf|ninf|pi|pi2|exp)\\b");
    rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    rule.format = numberFormat;
    rule.description = "Special constant";
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupOperatorRules() {
    // Операторы (language.txt пункты 34, 35, 36)
    QStringList sortedOperators = operators;
    // Сортировка по длине - сначала длинные операторы
    std::sort(sortedOperators.begin(), sortedOperators.end(),
        [](const QString& a, const QString& b) {
            return a.length() > b.length();
        });
    
    for (const QString& op : sortedOperators) {
        HighlightingRule rule;
        // Экранирование специальных символов
        QString escaped = QRegularExpression::escape(op);
        rule.pattern = QRegularExpression(escaped);
        rule.format = operatorFormat;
        rule.description = "Operator: " + op;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupDirectiveRules() {
    // Директивы препроцессора (language.txt пункт 37)
    for (const QString& directive : directives) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("^\\s*" + QRegularExpression::escape(directive) + "\\b");
        rule.pattern.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        rule.format = directiveFormat;
        rule.description = "Directive: " + directive;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupAnnotationRules() {
    // Аннотации валидации (language.txt пункт 26, 27)
    HighlightingRule rule;
    rule.pattern = QRegularExpression("@\\w+");
    rule.format = annotationFormat;
    rule.description = "Annotation";
    highlightingRules.append(rule);
    
    // Документационные теги (language.txt пункт 50)
    rule.pattern = QRegularExpression("//\\s*@(method|param|return|description|example|see|option|deprecated|since|author|version)");
    rule.format = docCommentFormat;
    rule.format.setFontWeight(QFont::Bold);
    rule.description = "Documentation tag";
    highlightingRules.append(rule);
}

// ============================================================================
// Основная функция подсветки
// ============================================================================

void SyntaxHighlighter::highlightBlock(const QString &text) {
    keywordCount = 0;
    typeCount = 0;
    functionCount = 0;
    
    // Применение стандартных правил
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            
            // Подсчёт для статистики
            if (rule.format == keywordFormat) keywordCount++;
            else if (rule.format == typeFormat) typeCount++;
            else if (rule.format == functionFormat) functionCount++;
        }
    }
    
    // Обработка многострочных комментариев
    setCurrentBlockState(0);
    
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf("/*");
    }
    
    while (startIndex >= 0) {
        int endIndex = text.indexOf("*/", startIndex);
        int commentLength;
        
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + 2;
        }
        
        // Проверка на документационный комментарий
        QTextCharFormat format = commentFormat;
        if (text.mid(startIndex, 3) == "/**") {
            format = docCommentFormat;
        }
        
        setFormat(startIndex, commentLength, format);
        startIndex = text.indexOf("/*", startIndex + commentLength);
    }
    
    // Применение режимов подсветки
    applyModeHighlighting(text, currentBlockState());
}

// ============================================================================
// Режимы подсветки
// ============================================================================

void SyntaxHighlighter::applyModeHighlighting(const QString& text, int blockState) {
    switch (displayMode) {
        case DisplayMode::TypeHighlight:
            applyTypeHighlighting(text);
            break;
        case DisplayMode::DimensionHighlight:
            applyDimensionHighlighting(text);
            break;
        case DisplayMode::TimingHighlight:
            applyTimingHighlighting(text);
            break;
        case DisplayMode::FrequencyHighlight:
            applyFrequencyHighlighting(text);
            break;
        case DisplayMode::GitDiff:
            applyGitDiffHighlighting(text);
            break;
        case DisplayMode::AuthorHighlight:
            applyAuthorHighlighting(text);
            break;
        case DisplayMode::AgeHighlight:
            applyAgeHighlighting(text);
            break;
        default:
            break;
    }
}

void SyntaxHighlighter::applyTypeHighlighting(const QString& text) {
    int blockNum = currentBlock().blockNumber();
    
    if (typeInfo.contains(blockNum + 1)) {
        QString type = typeInfo[blockNum + 1];
        // Подсветка всей строки
        setFormat(0, text.length(), typeHighlightFormat);
    }
}

void SyntaxHighlighter::applyDimensionHighlighting(const QString& text) {
    int blockNum = currentBlock().blockNumber();
    
    if (dimensionInfo.contains(blockNum + 1)) {
        setFormat(0, text.length(), dimensionHighlightFormat);
    }
}

void SyntaxHighlighter::applyTimingHighlighting(const QString& text) {
    int blockNum = currentBlock().blockNumber();
    
    if (timingInfo.contains(blockNum + 1)) {
        double time = timingInfo[blockNum + 1];
        // Цвет зависит от времени выполнения
        int intensity = qMin(255, static_cast<int>(time * 100));
        QColor color(intensity, 50, 50);
        
        QTextCharFormat format;
        format.setBackground(color);
        setFormat(0, text.length(), format);
    }
}

void SyntaxHighlighter::applyFrequencyHighlighting(const QString& text) {
    int blockNum = currentBlock().blockNumber();
    
    if (frequencyInfo.contains(blockNum + 1)) {
        int freq = frequencyInfo[blockNum + 1];
        // Цвет зависит от частоты выполнения
        int intensity = qMin(255, freq * 10);
        QColor color(intensity, intensity, 50);
        
        QTextCharFormat format;
        format.setBackground(color);
        setFormat(0, text.length(), format);
    }
}

void SyntaxHighlighter::applyGitDiffHighlighting(const QString& text) {
    // Парсинг git diff формата
    if (text.startsWith("+") && !text.startsWith("++") && !text.startsWith("+,,")) {
        setFormat(0, text.length(), gitAddFormat);
    } else if (text.startsWith("-") && !text.startsWith("--") && !text.startsWith("-,,")) {
        setFormat(0, text.length(), gitRemoveFormat);
    } else if (text.startsWith("@@")) {
        // Заголовок hunk
        QTextCharFormat format;
        format.setBackground(QColor(80, 80, 120));
        setFormat(0, text.length(), format);
    }
}

void SyntaxHighlighter::applyAuthorHighlighting(const QString& text) {
    int blockNum = currentBlock().blockNumber();
    
    if (authorInfo.contains(blockNum + 1)) {
        setFormat(0, text.length(), authorHighlightFormat);
    }
}

void SyntaxHighlighter::applyAgeHighlighting(const QString& text) {
    int blockNum = currentBlock().blockNumber();
    
    if (ageInfo.contains(blockNum + 1)) {
        QDateTime age = ageInfo[blockNum + 1];
        qint64 days = age.daysTo(QDateTime::currentDateTime());
        
        // Цвет зависит от возраста
        QColor color;
        if (days < 1) {
            color = QColor(50, 80, 50);  // Сегодня - зелёный
        } else if (days < 7) {
            color = QColor(80, 80, 50);  // Неделя - жёлтый
        } else if (days < 30) {
            color = QColor(80, 60, 50);  // Месяц - оранжевый
        } else {
            color = QColor(80, 50, 50);  // Старше - красный
        }
        
        QTextCharFormat format;
        format.setBackground(color);
        setFormat(0, text.length(), format);
    }
}

// ============================================================================
// Утилиты
// ============================================================================

bool SyntaxHighlighter::isInString(int pos, const QString& text) const {
    int stringStart = -1;
    for (int i = 0; i < pos && i < text.length(); i++) {
        if (text[i] == '"' && (i == 0 || text[i-1] != '\\')) {
            if (stringStart == -1) {
                stringStart = i;
            } else {
                stringStart = -1;
            }
        }
    }
    return stringStart != -1;
}

bool SyntaxHighlighter::isInComment(int pos, const QString& text) const {
    // Проверка на однострочный комментарий
    int commentPos = text.indexOf("//");
    if (commentPos >= 0 && commentPos < pos) {
        return true;
    }
    
    // Проверка на многострочный комментарий
    int blockStart = text.indexOf("/*");
    int blockEnd = text.indexOf("*/");
    if (blockStart >= 0 && (blockEnd == -1 || pos > blockStart)) {
        return true;
    }
    
    return false;
}

bool SyntaxHighlighter::isInCharLiteral(int pos, const QString& text) const {
    int charStart = -1;
    for (int i = 0; i < pos && i < text.length(); i++) {
        if (text[i] == '\'' && (i == 0 || text[i-1] != '\\')) {
            if (charStart == -1) {
                charStart = i;
            } else {
                charStart = -1;
            }
        }
    }
    return charStart != -1;
}

QColor SyntaxHighlighter::blendColors(const QColor& base, const QColor& overlay, qreal alpha) const {
    int r = base.red() * (1 - alpha) + overlay.red() * alpha;
    int g = base.green() * (1 - alpha) + overlay.green() * alpha;
    int b = base.blue() * (1 - alpha) + overlay.blue() * alpha;
    return QColor(r, g, b);
}

} // namespace proxima