#ifndef CENTAURI_SYNTAXHIGHLIGHTER_H
#define CENTAURI_SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include "CodeEditor.h"

namespace proxima {

struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
};

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    
public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    ~SyntaxHighlighter();
    
    void setFilePath(const QString& path) { filePath = path; }
    QString getFilePath() const { return filePath; }
    
    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode() const { return displayMode; }
    
    void setTypeInfo(const QMap<int, QString>& types);
    void setDimensionInfo(const QMap<int, QString>& dimensions);
    void setTimingInfo(const QMap<int, double>& timings);
    
protected:
    void highlightBlock(const QString &text) override;
    
private:
    void setupProximaRules();
    void setupKeywordRules();
    void setupTypeRules();
    void setupFunctionRules();
    void setupCommentRules();
    void setupStringRules();
    void setupNumberRules();
    void setupOperatorRules();
    
    void applyModeHighlighting(const QString& text, int state);
    void applyTypeHighlighting(const QString& text);
    void applyDimensionHighlighting(const QString& text);
    void applyTimingHighlighting(const QString& text);
    void applyGitDiffHighlighting(const QString& text);
    
    QString filePath;
    DisplayMode displayMode;
    
    QVector<HighlightingRule> highlightingRules;
    
    // Formats
    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat preprocessorFormat;
    
    // Mode-specific formats
    QTextCharFormat typeHighlightFormat;
    QTextCharFormat dimensionHighlightFormat;
    QTextCharFormat timingHighlightFormat;
    QTextCharFormat frequencyHighlightFormat;
    QTextCharFormat gitAddFormat;
    QTextCharFormat gitRemoveFormat;
    QTextCharFormat gitModifyFormat;
    
    // Type information
    QMap<int, QString> typeInfo;
    QMap<int, QString> dimensionInfo;
    QMap<int, double> timingInfo;
    
    // Proxima keywords
    static const QStringList keywords;
    static const QStringList types;
    static const QStringList builtins;
};

} // namespace proxima

#endif // CENTAURI_SYNTAXHIGHLIGHTER_H