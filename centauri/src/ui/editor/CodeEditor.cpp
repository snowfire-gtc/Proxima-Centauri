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

    // Connect to folding manager
    CodeFoldingManager::getInstance().connectToEditor(this);
    CodeFoldingManager::getInstance().initialize(document());

    // Connect folding signals
    connect(&CodeFoldingManager::getInstance(), &CodeFoldingManager::foldStateChanged,
            this, &CodeEditor::onFoldStateChanged);
    connect(&CodeFoldingManager::getInstance(), &CodeFoldingManager::visibilityChanged,
            this, &CodeEditor::onVisibilityChanged);

    // Connect to snippet manager
    SnippetManager::getInstance().setDocument(document());

    // Connect snippet signals
    connect(&SnippetManager::getInstance(), &SnippetManager::sessionStarted,
            this, &CodeEditor::onSnippetSessionStarted);
    connect(&SnippetManager::getInstance(), &SnippetManager::sessionEnded,
            this, &CodeEditor::onSnippetSessionEnded);
    connect(&SnippetManager::getInstance(), &SnippetManager::placeholderNavigated,
            this, &CodeEditor::onPlaceholderNavigated);
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

    // Load fold state after loading file
    CodeFoldingManager::getInstance().loadFoldState(path);

    return true;
}

bool CodeEditor::saveFile() {
    if (filePath.isEmpty()) {
        // Save as
        return false;
    }

    CodeFoldingManager::getInstance().saveFoldState(filePath);

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

bool CodeEditor::checkAndInsertSnippet() {
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    QString blockText = block.text();

    // Get word before cursor
    int cursorPosInBlock = cursor.position() - block.position();
    int wordStart = cursorPosInBlock;

    while (wordStart > 0 &&
           (blockText[wordStart - 1].isLetterOrNumber() ||
            blockText[wordStart - 1] == '.' ||
            blockText[wordStart - 1] == '_')) {
        wordStart--;
    }

    QString trigger = blockText.mid(wordStart, cursorPosInBlock - wordStart);

    if (trigger.isEmpty()) {
        return false;
    }

    // Check if trigger matches a snippet
    SnippetDefinition snippet = SnippetManager::getInstance().getSnippet(trigger);

    if (snippet.trigger.isEmpty()) {
        return false;
    }

    // Insert snippet
    cursor.setPosition(block.position() + wordStart);
    cursor.setPosition(block.position() + cursorPosInBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    int insertPosition = cursor.position();

    SnippetManager::getInstance().startSnippetSession(
        document(),
        snippet.content,
        insertPosition
    );

    LOG_INFO("Inserted snippet: " + snippet.trigger.toStdString());
    return true;
}

void CodeEditor::insertSnippet(const QString& snippetName) {
    SnippetDefinition snippet = SnippetManager::getInstance().getSnippet(snippetName);

    if (snippet.trigger.isEmpty()) {
        LOG_WARNING("Snippet not found: " + snippetName.toStdString());
        return;
    }

    QTextCursor cursor = textCursor();
    int insertPosition = cursor.position();

    SnippetManager::getInstance().startSnippetSession(
        document(),
        snippet.content,
        insertPosition
    );
}

void CodeEditor::onSnippetSessionStarted(const SnippetSession& session) {
    // Update UI to show snippet mode
    statusBar->setMessage(QString("Snippet mode: %1 placeholders").arg(session.placeholders.size()));

    // Show placeholder info
    if (!session.placeholders.isEmpty()) {
        QString info = QString("Placeholder %1/%2")
            .arg(session.currentPlaceholderIndex + 1)
            .arg(session.placeholders.size());

        QToolTip::showText(QCursor::pos(), info, this);
    }
}

void CodeEditor::onSnippetSessionEnded() {
    // Clear snippet mode UI
    statusBar->clearMessage();
    QToolTip::hideText();
}

void CodeEditor::onPlaceholderNavigated(int currentIndex, int total) {
    // Update UI with current placeholder info
    QString info = QString("Placeholder %1/%2")
        .arg(currentIndex + 1)
        .arg(total);

    statusBar->setMessage(info);

    // Show tooltip with placeholder info
    if (currentIndex >= 0 && currentIndex < SnippetManager::getInstance().getCurrentSession().placeholders.size()) {
        const SnippetPlaceholder& placeholder =
            SnippetManager::getInstance().getCurrentSession().placeholders[currentIndex];

        QString tooltip = QString("Placeholder #%1\nDefault: %2")
            .arg(placeholder.id)
            .arg(placeholder.defaultText);

        QRect rect = SnippetManager::getInstance().getPlaceholderRect(currentIndex);
        if (rect.isValid()) {
            QToolTip::showText(mapToGlobal(rect.center()), tooltip, this);
        }
    }
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

    // Snippet navigation
    if (SnippetManager::getInstance().isSessionActive()) {
        if (event->key() == Qt::Key_Tab) {
            if (event->modifiers() & Qt::ShiftModifier) {
                // Shift+Tab - previous placeholder
                SnippetManager::getInstance().previousPlaceholder();
            } else {
                // Tab - next placeholder
                SnippetManager::getInstance().nextPlaceholder();
            }
            event->accept();
            return;
        }

        if (event->key() == Qt::Key_Escape) {
            // Escape - end snippet session
            SnippetManager::getInstance().endSnippetSession();
            event->accept();
            return;
        }
    }

   // Check for snippet trigger on Enter/Space
    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Space) {
        if (checkAndInsertSnippet()) {
            event->accept();
            return;
        }
    }

    if (event->key() == Qt::Key_Tab) {
        // Insert spaces instead of tab
        insertPlainText("    ");
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}


void CodeEditor::mousePressEvent(QMouseEvent *event) {
    // Check for fold marker click
    if (event->button() == Qt::LeftButton) {
        int line = CodeFoldingManager::getInstance().getLineFromMousePos(event->pos());
        if (line > 0 && CodeFoldingManager::getInstance().isMouseOverFoldMarker(event->pos())) {
            CodeFoldingManager::getInstance().toggleFold(line);
            event->accept();
            return;
        }
    }

    QPlainTextEdit::mousePressEvent(event);
}

void CodeEditor::mouseMoveEvent(QMouseEvent *event) {
    // Update hover state for fold markers
    bool wasOverMarker = mouseOverFoldMarker;
    mouseOverFoldMarker = CodeFoldingManager::getInstance().isMouseOverFoldMarker(event->pos());

    if (wasOverMarker != mouseOverFoldMarker) {
        viewport()->update();

        if (mouseOverFoldMarker) {
            int line = CodeFoldingManager::getInstance().getLineFromMousePos(event->pos());
            QVector<FoldRegion> regions = CodeFoldingManager::getInstance().getFoldRegionsAtLine(line);

            if (!regions.isEmpty()) {
                QString tooltip = regions[0].name;
                if (!regions[0].preview.isEmpty()) {
                    tooltip += "\n" + regions[0].preview;
                }
                QToolTip::showText(event->globalPos(), tooltip, this);
            }
        } else {
            QToolTip::hideText();
        }
    }

    QPlainTextEdit::mouseMoveEvent(event);
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    // Folding shortcuts
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
            case Qt::Key_Minus:
                CodeFoldingManager::getInstance().foldRegion(
                    getCurrentFoldRegionId());
                event->accept();
                return;
            case Qt::Key_Plus:
                CodeFoldingManager::getInstance().unfoldRegion(
                    getCurrentFoldRegionId());
                event->accept();
                return;
            case Qt::Key_0:
                CodeFoldingManager::getInstance().foldAll();
                event->accept();
                return;
            case Qt::Key_1:
                CodeFoldingManager::getInstance().unfoldAll();
                event->accept();
                return;
            case Qt::Key_2:
                CodeFoldingManager::getInstance().foldLevel(1);
                event->accept();
                return;
            case Qt::Key_3:
                CodeFoldingManager::getInstance().foldLevel(2);
                event->accept();
                return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::onFoldStateChanged(int regionId, bool isFolded) {
    // Update editor state
    viewport()->update();

    // Save fold state
    if (!filePath.isEmpty()) {
        CodeFoldingManager::getInstance().saveFoldState(filePath);
    }
}

void CodeEditor::onVisibilityChanged(int startLine, int endLine, bool visible) {
    // Hide/show lines in editor
    // This would require custom block visibility implementation
}

int CodeEditor::getCurrentFoldRegionId() const {
    int line = getCurrentLine();
    QVector<FoldRegion> regions = CodeFoldingManager::getInstance().getFoldRegionsAtLine(line);

    if (!regions.isEmpty()) {
        return regions[0].id;
    }
    return -1;
}

void CodeEditor::toggleFold(int line) {
    CodeFoldingManager::getInstance().toggleFold(line);
}

bool CodeEditor::isFolded(int line) const {
    return CodeFoldingManager::getInstance().isLineFolded(line);
}

void CodeEditor::foldAll() {
    CodeFoldingManager::getInstance().foldAll();
}

void CodeEditor::unfoldAll() {
    CodeFoldingManager::getInstance().unfoldAll();
}

void CodeEditor::focusInEvent(QFocusEvent *event) {
    QPlainTextEdit::focusInEvent(event);
    highlightCurrentLine();
}

void CodeEditor::paintEvent(QPaintEvent *event) {
    QPlainTextEdit::paintEvent(event);

    // Custom painting for execution line, etc.
    QPainter painter(viewport());
    drawFoldMarkers(painter); // Draw fold markers
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
    if (!CodeFoldingManager::getInstance().getConfig().enableFolding) {
        return;
    }

    QVector<FoldRegion> regions = CodeFoldingManager::getInstance().getFoldRegions();

    for (const FoldRegion& region : regions) {
        QRect markerRect = CodeFoldingManager::getInstance().getFoldMarkerRect(region.foldLine);

        if (markerRect.isValid() && markerRect.intersects(viewport()->rect())) {
            // Draw fold marker background
            bool isHover = CodeFoldingManager::getInstance().isMouseOverFoldMarker(
                mapFromGlobal(QCursor::pos()));

            QColor bgColor = isHover ?
                QColor(CodeFoldingManager::getInstance().getConfig().foldMarkerHoverColor) :
                QColor(CodeFoldingManager::getInstance().getConfig().foldMarkerColor);

            painter.setBrush(bgColor);
            painter.setPen(Qt::NoPen);

            // Draw fold control (minus/plus box)
            if (region.isFolded) {
                // Plus sign (folded)
                painter.drawRect(markerRect.adjusted(2, 2, -2, -2));
                painter.setPen(QColor(255, 255, 255));
                painter.drawLine(markerRect.center().x() - 4, markerRect.center().y(),
                               markerRect.center().x() + 4, markerRect.center().y());
                painter.drawLine(markerRect.center().x(), markerRect.center().y() - 4,
                               markerRect.center().x(), markerRect.center().y() + 4);
            } else {
                // Minus sign (unfolded)
                painter.drawRect(markerRect.adjusted(2, 2, -2, -2));
                painter.setPen(QColor(255, 255, 255));
                painter.drawLine(markerRect.center().x() - 4, markerRect.center().y(),
                               markerRect.center().x() + 4, markerRect.center().y());
            }

            // Draw type icon (optional)
            // QString icon = CodeFoldingManager::getInstance().getFoldIcon(region.type);
        }
    }
}

QString CodeEditor::lineNumberAreaColor() const {
    return QColor(30, 30, 30);
}

// Добавляем интеграцию с CompletionProvider

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    // Обработка автодополнения
    if (event->key() == Qt::Key_Tab ||
        (event->key() == Qt::Key_Space && event->modifiers() & Qt::ControlModifier)) {

        if (completionPopup && completionPopup->isVisible()) {
            // Выбор элемента из popup
            completionPopup->selectCurrent();
            return;
        } else {
            // Показать автодополнение
            showCompletionPopup();
            return;
        }
    }

    if (event->key() == Qt::Key_Dot) {
        // После "." показываем автодополнение методов
        QPlainTextEdit::keyPressEvent(event);
        QTimer::singleShot(50, this, &CodeEditor::showCompletionPopup);
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        if (completionPopup && completionPopup->isVisible()) {
            completionPopup->hide();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::showCompletionPopup() {
    if (!completionProvider) {
        completionProvider = &CompletionProvider::getInstance();
    }

    // Анализируем контекст
    QString text = toPlainText();
    int cursorPos = textCursor().position();

    CompletionContext context = completionProvider->analyzeContext(text, cursorPos);

    // Получаем подсказки
    QVector<CompletionItem> completions = completionProvider->getCompletions(context);

    if (completions.isEmpty()) {
        if (completionPopup) {
            completionPopup->hide();
        }
        return;
    }

    // Показываем popup
    if (!completionPopup) {
        completionPopup = new CompletionPopup(this);
    }

    completionPopup->setCompletions(completions);
    completionPopup->setContext(context);

    // Позиционируем popup у курсора
    QRect cursorRect = cursorRect(textCursor());
    QPoint popupPos = mapToGlobal(QPoint(cursorRect.right(), cursorRect.bottom()));
    completionPopup->move(popupPos);
    completionPopup->show();

    // Показываем документацию в tooltip
    if (!completions.isEmpty()) {
        QString docText = completions[0].description;
        if (!docText.isEmpty()) {
            QToolTip::showText(popupPos + QPoint(0, 30), docText, this);
        }
    }
}

void CodeEditor::applyCompletion(const CompletionItem& item) {
    QTextCursor cursor = textCursor();
    CompletionContext context = completionProvider->analyzeContext(toPlainText(), cursor.position());

    // Заменяем текущее слово на выбранное
    int wordStart = cursor.position() - context.currentWord.length();
    cursor.setPosition(wordStart, QTextCursor::MoveAnchor);
    cursor.setPosition(cursor.position() + context.currentWord.length(), QTextCursor::KeepAnchor);

    if (item.type == "snippet") {
        // Insert snippet with placeholder navigation
        SnippetManager::getInstance().startSnippetSession(
            editor->document(),
            item.detail,  // Snippet content with placeholders
            editor->textCursor().position()
        );
    } else {
        // Вставка метода/свойства
        QString text = item.text;
        if (item.type == "method") {
            text += "()";
            cursor.insertText(text);
            // Перемещаем курсор внутрь скобок
            cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
        } else {
            cursor.insertText(text);
        }
    }

    // Показываем обучающую подсказку
    if (!item.description.isEmpty()) {
        QString tip = completionProvider->getExampleUsage(
            context.className.isEmpty() ? context.objectType : context.className,
            item.text);

        if (!tip.isEmpty()) {
            QToolTip::showText(QCursor::pos(), tip, this, QRect(), 5000);
        }
    }
}

} // namespace proxima
