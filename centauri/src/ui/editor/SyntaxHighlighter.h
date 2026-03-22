#ifndef CENTAURI_SYNTAXHIGHLIGHTER_H
#define CENTAURI_SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>
#include <QMap>
#include <QColor>
#include <QFont>
#include "editor/CodeEditor.h"

namespace proxima {

/**
 * @brief Правило подсветки
 */
struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
    QString description;  // Описание для tooltip
};

/**
 * @brief Класс подсветки синтаксиса для языка Proxima
 * 
 * Согласно требованию ide.txt пункт 11е:
 * - стандартный с подсветкой синтаксиса
 * - подсветка типов переменных
 * - подсветка размерности переменных
 * - подсветка времени выполнения
 * - подсветка частоты выполнения
 * - отображение изменений vs репозиторий
 * - автор правки
 * - старость правки
 */
class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    
public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    ~SyntaxHighlighter();
    
    // Настройка
    void setFilePath(const QString& path);
    QString getFilePath() const { return filePath; }
    
    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode() const { return displayMode; }
    
    // Информация о типах (для подсветки типов)
    void setTypeInfo(const QMap<int, QString>& types);
    void setDimensionInfo(const QMap<int, QString>& dimensions);
    void setTimingInfo(const QMap<int, double>& timings);
    void setFrequencyInfo(const QMap<int, int>& frequencies);
    void setGitDiffInfo(const QString& diff);
    void setAuthorInfo(const QMap<int, QString>& authors);
    void setAgeInfo(const QMap<int, QDateTime>& ages);
    
    // Переподсветка
    void rehighlightBlock(QTextBlock block);
    void rehighlightRange(int start, int end);
    
    // Статистика
    int getHighlightedKeywords() const { return keywordCount; }
    int getHighlightedTypes() const { return typeCount; }
    int getHighlightedFunctions() const { return functionCount; }
    
protected:
    void highlightBlock(const QString &text) override;
    
private:
    // Инициализация правил
    void setupProximaRules();
    void setupKeywordRules();
    void setupTypeRules();
    void setupFunctionRules();
    void setupCommentRules();
    void setupStringRules();
    void setupNumberRules();
    void setupOperatorRules();
    void setupDirectiveRules();
    void setupAnnotationRules();
    
    // Режимы подсветки
    void applyModeHighlighting(const QString& text, int blockState);
    void applyTypeHighlighting(const QString& text);
    void applyDimensionHighlighting(const QString& text);
    void applyTimingHighlighting(const QString& text);
    void applyFrequencyHighlighting(const QString& text);
    void applyGitDiffHighlighting(const QString& text);
    void applyAuthorHighlighting(const QString& text);
    void applyAgeHighlighting(const QString& text);
    
    // Утилиты
    bool isInString(int pos, const QString& text) const;
    bool isInComment(int pos, const QString& text) const;
    bool isInCharLiteral(int pos, const QString& text) const;
    QColor blendColors(const QColor& base, const QColor& overlay, qreal alpha) const;
    
    // Форматы
    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat docCommentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat charFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat directiveFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat annotationFormat;
    
    // Форматы режимов
    QTextCharFormat typeHighlightFormat;
    QTextCharFormat dimensionHighlightFormat;
    QTextCharFormat timingHighlightFormat;
    QTextCharFormat frequencyHighlightFormat;
    QTextCharFormat gitAddFormat;
    QTextCharFormat gitRemoveFormat;
    QTextCharFormat gitModifyFormat;
    QTextCharFormat authorHighlightFormat;
    QTextCharFormat ageHighlightFormat;
    
    void loadAgeInfoFromStory(const QString& filePath);
    StoryManager* storyManager;
    
    // Правила
    QVector<HighlightingRule> highlightingRules;
    
    // Информация для режимов
    QString filePath;
    DisplayMode displayMode;
    QMap<int, QString> typeInfo;
    QMap<int, QString> dimensionInfo;
    QMap<int, double> timingInfo;
    QMap<int, int> frequencyInfo;
    QString gitDiffInfo;
    QMap<int, QString> authorInfo;
    QMap<int, QDateTime> ageInfo;
    
    // Счётчики
    int keywordCount;
    int typeCount;
    int functionCount;
    
    // Ключевые слова Proxima (согласно language.txt)
    static const QStringList keywords;
    static const QStringList types;
    static const QStringList builtins;
    static const QStringList operators;
    static const QStringList directives;
};

} // namespace proxima

#endif // CENTAURI_SYNTAXHIGHLIGHTER_H
