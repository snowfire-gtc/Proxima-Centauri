#include "CodeEditor.h"
#include <QTextBlock>
#include <QPainter>
#include <QKeyEvent>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>

namespace proxima {

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor), editor(editor) {}

QSize LineNumberArea::sizeHint() const {
    return QSize(editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    editor->lineNumberAreaPaintEvent(event);
}

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , modified(false)
    , displayMode(DisplayMode::Standard)
    , currentExecutionLine(-1) {
    
    setupEditor();
    
    lineNumberArea = new LineNumberArea(this);
    
    connect(this, &CodeEditor::blockCountChanged, 
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, 
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, 
            this, &CodeEditor::highlightCurrentLine);
    
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

CodeEditor::~CodeEditor() {}

void CodeEditor::setupEditor() {
    // Set font
    QFont font("Consolas", 11);
    font.setStyleHint(QFont::Monospace);
    setFont(font);
    
    // Set tab width
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
    
    // Enable line wrapping option
    // setLineWrapMode(LineWrapMode::NoWrap);
    
    // Create syntax highlighter
    highlighter = new SyntaxHighlighter(document());
    
    // Set line number area color
    setStyleSheet("QPlainTextEdit { background-color: #1e1e1e; color: #d4d4d4; }");
}

bool CodeEditor::loadFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot open file: " + path);
        return false;
    }
    
    QTextStream in(&file);
    setPlainText(in.readAll());
    file.close();
    
    filePath = path;
    modified = false;
    clearBreakpoints();
    
    // Update highlighter with file path for context
    highlighter->setFilePath(path);
    
    return true;
}

bool CodeEditor::saveFile() {
    if (filePath.isEmpty()) {
        // Save as
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot save file: " + filePath);
        return false;
    }
    
    QTextStream out(&file);
    out << toPlainText();
    file.close();
    
    modified = false;
    emit fileModified(false);
    
    return true;
}

int CodeEditor::getCurrentLine() const {
    return textCursor().blockNumber() + 1;
}

int CodeEditor::getCurrentColumn() const {
    return textCursor().columnNumber();
}

void CodeEditor::goToLine(int line) {
    if (line < 1 || line > blockCount()) return;
    
    QTextCursor cursor(document()->findBlockByNumber(line - 1));
    setTextCursor(cursor);
    setFocus();
}

QString CodeEditor::getSelectedCode() const {
    return textCursor().selectedText();
}

QString CodeEditor::getCurrentLineCode() const {
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    return cursor.selectedText();
}

int CodeEditor::getSelectionStart() const {
    return textCursor().selectionStart();
}

int CodeEditor::getSelectionEnd() const {
    return textCursor().selectionEnd();
}

void CodeEditor::formatCode() {
    // TODO: Implement code formatting
    // Would use compiler's formatter service
    highlighter->rehighlight();
}

void CodeEditor::insertSnippet(const QString& snippet) {
    textCursor().insertText(snippet);
}

void CodeEditor::toggleBreakpoint(int line) {
    if (line < 1 || line > blockCount()) return;
    
    int index = breakpoints.indexOf(line);
    if (index >= 0) {
        breakpoints.removeAt(index);
        emit breakpointToggled(line, false);
    } else {
        breakpoints.append(line);
        std::sort(breakpoints.begin(), breakpoints.end());
        emit breakpointToggled(line, true);
    }
    
    viewport()->update();
}

bool CodeEditor::hasBreakpoint(int line) const {
    return breakpoints.contains(line);
}

void CodeEditor::clearBreakpoints() {
    breakpoints.clear();
    viewport()->update();
}

void CodeEditor::setDisplayMode(DisplayMode mode) {
    displayMode = mode;
    highlighter->setDisplayMode(mode);
    highlighter->rehighlight();
    viewport()->update();
}

void CodeEditor::applyTypeHighlighting(const QMap<int, QString>& lineTypes) {
    typeHighlights = lineTypes;
    if (displayMode == DisplayMode::TypeHighlight) {
        highlighter->rehighlight();
    }
}

void CodeEditor::applyDimensionHighlighting(const QMap<int, QString>& lineDimensions) {
    dimensionHighlights = lineDimensions;
    if (displayMode == DisplayMode::DimensionHighlight) {
        highlighter->rehighlight();
    }
}

void CodeEditor::applyTimingHighlighting(const QMap<int, double>& lineTimings) {
    timingHighlights = lineTimings;
    if (displayMode == DisplayMode::TimingHighlight) {
        highlighter->rehighlight();
    }
}

void CodeEditor::applyFrequencyHighlighting(const QMap<int, int>& lineFrequencies) {
    frequencyHighlights = lineFrequencies;
    if (displayMode == DisplayMode::FrequencyHighlight) {
        highlighter->rehighlight();
    }
}

void CodeEditor::applyGitDiff(const QString& diff) {
    gitDiffData = diff;
    if (displayMode == DisplayMode::GitDiff) {
        highlighter->rehighlight();
    }
}

void CodeEditor::applyAuthorHighlighting(const QMap<int, QString>& lineAuthors) {
    authorHighlights = lineAuthors;
    if (displayMode == DisplayMode::AuthorHighlight) {
        highlighter->rehighlight();
    }
}

void CodeEditor::applyAgeHighlighting(const QMap<int, QDateTime>& lineAges) {
    ageHighlights = lineAges;
    if (displayMode == DisplayMode::AgeHighlight) {
        highlighter->rehighlight();
    }
}

void CodeEditor::toggleFold(int line) {
    // TODO: Implement code folding
    // Would need to parse code structure
}

bool CodeEditor::isFolded(int line) const {
    return foldedLines.contains(line);
}

void CodeEditor::foldAll() {
    // TODO: Implement fold all
}

void CodeEditor::unfoldAll() {
    foldedLines.clear();
    viewport()->update();
}

void CodeEditor::setCurrentExecutionLine(int line) {
    currentExecutionLine = line;
    goToLine(line);
    viewport()->update();
}

void CodeEditor::clearExecutionLine() {
    currentExecutionLine = -1;
    viewport()->update();
}

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), 
                                      lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    // Handle special keys
    if (event->key() == Qt::Key_F9) {
        // Toggle breakpoint
        toggleBreakpoint(getCurrentLine());
        return;
    }
    
    if (event->key() == Qt::Key_Tab) {
        // Insert spaces instead of tab
        insertPlainText("    ");
        return;
    }
    
    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::focusInEvent(QFocusEvent *event) {
    QPlainTextEdit::focusInEvent(event);
    highlightCurrentLine();
}

void CodeEditor::paintEvent(QPaintEvent *event) {
    QPlainTextEdit::paintEvent(event);
    
    // Custom painting for execution line, etc.
    QPainter painter(viewport());
    drawExecutionLine(painter);
    drawBreakpoints(painter);
    drawFoldMarkers(painter);
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        
        QColor lineColor = QColor(40, 40, 40);
        
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    
    setExtraSelections(extraSelections);
    
    // Emit cursor position
    emit cursorPositionChanged(getCurrentLine(), getCurrentColumn());
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), lineNumberAreaColor());
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            
            // Highlight line with breakpoint
            if (breakpoints.contains(blockNumber + 1)) {
                painter.setPen(QColor(255, 100, 100));
                painter.setFont(QFont("Consolas", 10, QFont::Bold));
            } else {
                painter.setPen(QColor(120, 120, 120));
                painter.setFont(QFont("Consolas", 10));
            }
            
            painter.drawText(0, top, lineNumberArea->width() - 5, 
                           fontMetrics().height(), Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    int space = 13 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::drawBreakpoints(QPainter& painter) {
    // Draw breakpoint indicators in the left margin
    painter.setBrush(QColor(255, 100, 100));
    painter.setPen(Qt::NoPen);
    
    for (int line : breakpoints) {
        QTextBlock block = document()->findBlockByNumber(line - 1);
        if (block.isValid()) {
            int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
            int height = qRound(blockBoundingRect(block).height());
            
            painter.drawEllipse(2, y + height/2 - 4, 8, 8);
        }
    }
}

void CodeEditor::drawExecutionLine(QPainter& painter) {
    if (currentExecutionLine < 1) return;
    
    QTextBlock block = document()->findBlockByNumber(currentExecutionLine - 1);
    if (block.isValid()) {
        int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int height = qRound(blockBoundingRect(block).height());
        
        // Draw arrow indicator
        painter.setBrush(QColor(100, 200, 100));
        painter.setPen(Qt::NoPen);
        
        QPolygon arrow;
        arrow << QPoint(0, y) << QPoint(10, y + height/2) << QPoint(0, y + height);
        painter.drawPolygon(arrow);
        
        // Highlight line background
        painter.setBrush(QColor(50, 80, 50, 100));
        painter.drawRect(10, y, viewport()->width(), height);
    }
}

void CodeEditor::drawFoldMarkers(QPainter& painter) {
    // TODO: Implement fold marker drawing
}

QString CodeEditor::lineNumberAreaColor() const {
    return QColor(30, 30, 30);
}

} // namespace proxima