#include "SyntaxHighlighter.h"
#include <QTextBlock>

namespace proxima {

const QStringList SyntaxHighlighter::keywords = {
    "if", "elseif", "else", "end", "for", "in", "while", "do",
    "switch", "case", "default", "return", "continue", "break",
    "class", "interface", "template", "public", "protected", "private",
    "constructor", "destructor", "namespace", "using", "include",
    "exclude", "define", "ifdef", "endif", "auto", "void", "type",
    "arguments", "parallel", "true", "false", "null", "nan", "inf",
    "pi", "region", "endregion", "suite", "test", "assert", "gem"
};

const QStringList SyntaxHighlighter::types = {
    "int4", "int8", "int16", "int32", "int64", "int",
    "single", "double", "bool", "char", "string",
    "vector", "matrix", "layer", "collection",
    "point2", "point3", "point4",
    "matrix22", "matrix33", "matrix44", "matrix34",
    "time", "file", "rtti", "method", "parallel"
};

const QStringList SyntaxHighlighter::builtins = {
    "print", "write", "read", "open", "close",
    "zeros", "ones", "eye", "rand", "size", "length",
    "sum", "mean", "max", "min", "abs", "sqrt", "exp",
    "sin", "cos", "tan", "log", "log10",
    "type", "rtti", "dbgstop", "dbgprint", "dbgcontext", "dbgstack"
};

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , displayMode(DisplayMode::Standard) {
    
    setupProximaRules();
}

SyntaxHighlighter::~SyntaxHighlighter() {}

void SyntaxHighlighter::setDisplayMode(DisplayMode mode) {
    displayMode = mode;
    rehighlight();
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

void SyntaxHighlighter::setupProximaRules() {
    setupKeywordRules();
    setupTypeRules();
    setupFunctionRules();
    setupCommentRules();
    setupStringRules();
    setupNumberRules();
    setupOperatorRules();
    
    // Initialize formats
    keywordFormat.setForeground(QColor(56, 139, 253));
    keywordFormat.setFontWeight(QFont::Bold);
    
    typeFormat.setForeground(QColor(78, 201, 176));
    typeFormat.setFontWeight(QFont::Bold);
    
    functionFormat.setForeground(QColor(220, 220, 170));
    
    commentFormat.setForeground(QColor(106, 135, 89));
    commentFormat.setFontItalic(true);
    
    stringFormat.setForeground(QColor(206, 145, 120));
    
    numberFormat.setForeground(QColor(181, 206, 168));
    
    operatorFormat.setForeground(QColor(212, 212, 212));
    
    preprocessorFormat.setForeground(QColor(197, 134, 192));
    
    // Mode-specific formats
    typeHighlightFormat.setBackground(QColor(50, 50, 80));
    typeHighlightFormat.setForeground(QColor(150, 150, 255));
    
    dimensionHighlightFormat.setBackground(QColor(50, 80, 50));
    dimensionHighlightFormat.setForeground(QColor(150, 255, 150));
    
    timingHighlightFormat.setBackground(QColor(80, 50, 50));
    timingHighlightFormat.setForeground(QColor(255, 150, 150));
    
    gitAddFormat.setBackground(QColor(50, 80, 50));
    gitRemoveFormat.setBackground(QColor(80, 50, 50));
    gitModifyFormat.setBackground(QColor(80, 80, 50));
}

void SyntaxHighlighter::setupKeywordRules() {
    for (const QString& keyword : keywords) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + keyword + "\\b");
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupTypeRules() {
    for (const QString& type : types) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + type + "\\b");
        rule.format = typeFormat;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::setupFunctionRules() {
    for (const QString& builtin : builtins) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + builtin + "\\b(?=\\s*\\()");
        rule.format = functionFormat;
        highlightingRules.append(rule);
    }
    
    // User-defined functions
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupCommentRules() {
    // Single line comment
    HighlightingRule rule;
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);
    
    // Multi-line comment (handled in highlightBlock)
}

void SyntaxHighlighter::setupStringRules() {
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\"[^\"]*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);
    
    // Char literal
    rule.pattern = QRegularExpression("'[^']*'");
    rule.format = stringFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupNumberRules() {
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);
}

void SyntaxHighlighter::setupOperatorRules() {
    QStringList operators = {
        "\\+", "-", "\\*", "/", "\\\\", "%", "\\^",
        "=", "==", "!=", "<", ">", "<=", ">=",
        "&", "\\|", "!", "\\?", ":", "'",
        "\\.", "\\[", "\\]", "\\(", "\\)", "\\{", "\\}"
    };
    
    for (const QString& op : operators) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(op);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // Apply standard highlighting rules
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    
    // Handle multi-line comments
    setCurrentBlockState(0);
    
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf("/\\*");
    }
    
    while (startIndex >= 0) {
        int endIndex = text.indexOf("\\*/", startIndex);
        int commentLength;
        
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + 3;
        }
        
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf("/\\*", startIndex + commentLength);
    }
    
    // Apply mode-specific highlighting
    applyModeHighlighting(text, currentBlockState());
}

void SyntaxHighlighter::applyModeHighlighting(const QString& text, int state) {
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
        case DisplayMode::GitDiff:
            applyGitDiffHighlighting(text);
            break;
        default:
            break;
    }
}

void SyntaxHighlighter::applyTypeHighlighting(const QString& text) {
    int blockNum = currentBlock().blockNumber();
    
    if (typeInfo.contains(blockNum + 1)) {
        QString type = typeInfo[blockNum + 1];
        // Apply type highlight format to the entire line
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
        // Color intensity based on execution time
        int intensity = qMin(255, static_cast<int>(time * 100));
        QColor color(intensity, 50, 50);
        
        QTextCharFormat format;
        format.setBackground(color);
        setFormat(0, text.length(), format);
    }
}

void SyntaxHighlighter::applyGitDiffHighlighting(const QString& text) {
    // Would parse git diff data and highlight added/removed/modified lines
    // This is a simplified version
    if (text.startsWith("+") && !text.startsWith("++")) {
        setFormat(0, text.length(), gitAddFormat);
    } else if (text.startsWith("-") && !text.startsWith("--")) {
        setFormat(0, text.length(), gitRemoveFormat);
    }
}

} // namespace proxima