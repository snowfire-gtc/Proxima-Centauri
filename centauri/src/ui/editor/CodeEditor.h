#ifndef CENTAURI_CODEEDITOR_H
#define CENTAURI_CODEEDITOR_H

#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QVector>
#include "SyntaxHighlighter.h"
#include "core/Module.h"

namespace proxima {

enum class DisplayMode {
    Standard,           // Syntax highlighting only
    TypeHighlight,      // Highlight variable types
    DimensionHighlight, // Highlight array dimensions
    TimingHighlight,    // Highlight execution time
    FrequencyHighlight, // Highlight execution frequency
    GitDiff,           // Show changes vs repository
    AuthorHighlight,    // Show last editor
    AgeHighlight       // Show last edit age
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor *editor);
    
    QSize sizeHint() const override;
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    CodeEditor *editor;
};

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
    
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();
    
    // File operations
    bool loadFile(const QString& path);
    bool saveFile();
    QString getFilePath() const { return filePath; }
    bool isModified() const { return modified; }
    
    // Cursor position
    int getCurrentLine() const;
    int getCurrentColumn() const;
    void goToLine(int line);
    
    // Code operations
    QString getSelectedCode() const;
    QString getCurrentLineCode() const;
    int getSelectionStart() const;
    int getSelectionEnd() const;
    void formatCode();
    void insertSnippet(const QString& snippet);
    
    // Breakpoints
    void toggleBreakpoint(int line);
    bool hasBreakpoint(int line) const;
    void clearBreakpoints();
    QVector<int> getBreakpoints() const { return breakpoints; }
    
    // Display modes
    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode() const { return displayMode; }
    
    // Highlighting
    void applyTypeHighlighting(const QMap<int, QString>& lineTypes);
    void applyDimensionHighlighting(const QMap<int, QString>& lineDimensions);
    void applyTimingHighlighting(const QMap<int, double>& lineTimings);
    void applyFrequencyHighlighting(const QMap<int, int>& lineFrequencies);
    void applyGitDiff(const QString& diff);
    void applyAuthorHighlighting(const QMap<int, QString>& lineAuthors);
    void applyAgeHighlighting(const QMap<int, QDateTime>& lineAges);
    
    // Folding
    void toggleFold(int line);
    bool isFolded(int line) const;
    void foldAll();
    void unfoldAll();
    
    // Visual elements
    void setCurrentExecutionLine(int line);
    void clearExecutionLine();
    
signals:
    void cursorPositionChanged(int line, int column);
    void fileModified(bool modified);
    void breakpointToggled(int line, bool enabled);
    void foldToggled(int line, bool folded);
    
protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    
private:
    void setupEditor();
    void updateLineNumbers();
    void drawBreakpoints(QPainter& painter);
    void drawExecutionLine(QPainter& painter);
    void drawFoldMarkers(QPainter& painter);
    QString lineNumberAreaColor() const;
    
    LineNumberArea *lineNumberArea;
    SyntaxHighlighter *highlighter;
    
    QString filePath;
    bool modified;
    DisplayMode displayMode;
    
    // Breakpoints
    QVector<int> breakpoints;
    
    // Folding
    QVector<int> foldedLines;
    QMap<int, int> foldRanges; // start -> end
    
    // Execution
    int currentExecutionLine;
    
    // Highlighting data
    QMap<int, QString> typeHighlights;
    QMap<int, QString> dimensionHighlights;
    QMap<int, double> timingHighlights;
    QMap<int, int> frequencyHighlights;
    QMap<int, QString> authorHighlights;
    QMap<int, QDateTime> ageHighlights;
    
    // Git diff
    QString gitDiffData;
};

} // namespace proxima

#endif // CENTAURI_CODEEDITOR_H