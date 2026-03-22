#include "CodeEditor.h"
#include <QTextCursor>
#include <QTextBlock>
#include <QTextDocument>
#include <QScrollBar>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>
#include <QCompleter>
#include <QAbstractItemView>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QWheelEvent>
#include <QMimeData>
#include <QUrl>
#include <QBuffer>
#include <QByteArray>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QReadWriteLock>
#include <QAtomicInt>
#include <QAtomicPointer>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QExplicitlySharedDataPointer>
#include <QImplicitlySharedDataPointer>
#include <QDataStream>
#include <QDebug>
#include <QElapsedTimer>
#include <QQueue>
#include <QSet>
#include <QHash>
#include <QMultiHash>
#include <QMultiMap>
#include <QPair>
#include <QTuple>
#include <QTypeInfo>
#include <QMetaType>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaClassInfo>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QScriptable>
#include <QScriptContext>
#include <QScriptContextInfo>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextListFormat>
#include <QTextTableFormat>
#include <QTextFrameFormat>
#include <QTextImageFormat>
#include <QTextObject>
#include <QTextObjectInterface>
#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>
#include <QPdfWriter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QFontInfo>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QStyle>
#include <QStyleFactory>
#include <QStylePainter>
#include <QStyleOption>
#include <QVariantAnimation>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>

namespace proxima {

// ============================================================================
// LineNumberArea Implementation
// ============================================================================

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor)
    , editor(editor) {
}

QSize LineNumberArea::sizeHint() const {
    return QSize(editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    editor->lineNumberAreaPaintEvent(event);
}

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , lineNumberArea(nullptr)
    , highlighter(nullptr)
    , foldingManager(nullptr)
    , modified(false)
    , displayMode(DisplayMode::Standard)
    , currentExecutionLine(-1)
    , foldTimer(nullptr)
    , autoCompletionEnabled(true)
    , completionPopup(nullptr)
    , showLineNumbers(true)
    , showWhitespace(false)
    , showIndentGuides(true)
    , highlightCurrentLine(true)
    , wordWrap(false)
    , wordCount(0) {

    setupEditor();
    setupConnections();
    setupShortcuts();
    setupContextMenu();

    LOG_DEBUG("CodeEditor created");
}

CodeEditor::~CodeEditor() {
    LOG_DEBUG("CodeEditor destroyed");
}

// ============================================================================
// Настройка редактора
// ============================================================================

void CodeEditor::setupEditor() {
    // Настройка шрифта
    QFont font("Consolas", 11);
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    setFont(font);

    // Настройка табов (4 пробела)
    setTabStopDistance(4 * QFontMetrics(font).horizontalAdvance(' '));

    // Настройка переноса слов
    setLineWrapMode(wordWrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);

    // Создание области номеров строк
    lineNumberArea = new LineNumberArea(this);

    // Создание подсветки синтаксиса
    highlighter = new SyntaxHighlighter(document());

    // Создание менеджера сворачивания
    foldingManager = &CodeFoldingManager::getInstance();
    foldingManager->initialize(document());

    // Создание popup автодополнения
    completionPopup = new QListWidget(this);
    completionPopup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    completionPopup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    completionPopup->setSelectionBehavior(QAbstractItemView::SelectRows);
    completionPopup->setSelectionMode(QAbstractItemView::SingleSelection);
    completionPopup->setVisible(false);

    connect(completionPopup, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        insertCompletion(item->text());
    });

    // Таймер для анализа сворачивания
    foldTimer = new QTimer(this);
    foldTimer->setSingleShot(true);
    foldTimer->setInterval(500);
    connect(foldTimer, &QTimer::timeout, this, &CodeEditor::onFoldTimerTimeout);

    // Обновление ширины области номеров строк
    updateLineNumberAreaWidth(0);

    // Подсветка текущей строки
    highlightCurrentLine();

    // Стиль редактора
    setStyleSheet(
        "QPlainTextEdit { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  selection-background-color: #264f78; "
        "  selection-color: #ffffff; "
        "}"
    );
}

void CodeEditor::setupConnections() {
    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);
    connect(this, &CodeEditor::cursorPositionChanged, this, [this]() {
        emit cursorPositionChanged(getCurrentLine(), getCurrentColumn());
    });
    connect(this, &CodeEditor::modificationChanged, this, [this](bool m) {
        modified = m;
        emit fileModified(m);
    });
    connect(this, &CodeEditor::textChanged, this, [this]() {
        foldTimer->start();
        wordCount = 0; // Сброс для пересчёта
    });
}

void CodeEditor::setupShortcuts() {
    // F9 - переключение точки останова
    QShortcut* breakpointShortcut = new QShortcut(QKeySequence(Qt::Key_F9), this);
    connect(breakpointShortcut, &QShortcut::activated, this, [this]() {
        toggleBreakpoint(getCurrentLine());
    });

    // Ctrl+Space - автодополнение
    QShortcut* completionShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space), this);
    connect(completionShortcut, &QShortcut::activated, this, &CodeEditor::triggerAutoCompletion);

    // Ctrl+I - отступ
    QShortcut* indentShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), this);
    connect(indentShortcut, &QShortcut::activated, this, &CodeEditor::indentSelection);

    // Ctrl+U - убрать отступ
    QShortcut* unindentShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_U), this);
    connect(unindentShortcut, &QShortcut::activated, this, &CodeEditor::unindentSelection);

    // Ctrl+/ - закомментировать
    QShortcut* commentShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash), this);
    connect(commentShortcut, &QShortcut::activated, this, &CodeEditor::commentSelection);

    // Ctrl+Shift+/ - раскомментировать
    QShortcut* uncommentShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Slash), this);
    connect(uncommentShortcut, &QShortcut::activated, this, &CodeEditor::uncommentSelection);

    // Ctrl+D - дублировать строку
    QShortcut* duplicateShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this);
    connect(duplicateShortcut, &QShortcut::activated, this, &CodeEditor::duplicateLine);

    // Ctrl+L - удалить строку
    QShortcut* deleteLineShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(deleteLineShortcut, &QShortcut::activated, this, &CodeEditor::deleteLine);

    // Alt+Up - переместить строку вверх
    QShortcut* moveUpShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Up), this);
    connect(moveUpShortcut, &QShortcut::activated, this, &CodeEditor::moveLineUp);

    // Alt+Down - переместить строку вниз
    QShortcut* moveDownShortcut = new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Down), this);
    connect(moveDownShortcut, &QShortcut::activated, this, &CodeEditor::moveLineDown);

    // Ctrl+Shift+F - форматировать код
    QShortcut* formatShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), this);
    connect(formatShortcut, &QShortcut::activated, this, &CodeEditor::formatCode);

    // Ctrl+M - свернуть все
    QShortcut* foldAllShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_M), this);
    connect(foldAllShortcut, &QShortcut::activated, this, &CodeEditor::foldAll);

    // Ctrl+Shift+M - развернуть все
    QShortcut* unfoldAllShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M), this);
    connect(unfoldAllShortcut, &QShortcut::activated, this, &CodeEditor::unfoldAll);
}

void CodeEditor::setupContextMenu() {
    contextMenu = new QMenu(this);

    undoAction = contextMenu->addAction("Отменить", this, &CodeEditor::undo);
    undoAction->setShortcut(QKeySequence::Undo);

    redoAction = contextMenu->addAction("Повторить", this, &CodeEditor::redo);
    redoAction->setShortcut(QKeySequence::Redo);

    contextMenu->addSeparator();

    cutAction = contextMenu->addAction("Вырезать", this, &CodeEditor::cut);
    cutAction->setShortcut(QKeySequence::Cut);

    copyAction = contextMenu->addAction("Копировать", this, &CodeEditor::copy);
    copyAction->setShortcut(QKeySequence::Copy);

    pasteAction = contextMenu->addAction("Вставить", this, &CodeEditor::paste);
    pasteAction->setShortcut(QKeySequence::Paste);

    selectAllAction = contextMenu->addAction("Выделить всё", this, &CodeEditor::selectAll);
    selectAllAction->setShortcut(QKeySequence::SelectAll);

    contextMenu->addSeparator();

    commentAction = contextMenu->addAction("Закомментировать", this, &CodeEditor::commentSelection);
    uncommentAction = contextMenu->addAction("Раскомментировать", this, &CodeEditor::uncommentSelection);

    contextMenu->addSeparator();

    indentAction = contextMenu->addAction("Увеличить отступ", this, &CodeEditor::indentSelection);
    unindentAction = contextMenu->addAction("Уменьшить отступ", this, &CodeEditor::unindentSelection);

    contextMenu->addSeparator();

    duplicateLineAction = contextMenu->addAction("Дублировать строку", this, &CodeEditor::duplicateLine);
    deleteLineAction = contextMenu->addAction("Удалить строку", this, &CodeEditor::deleteLine);

    contextMenu->addSeparator();

    formatCodeAction = contextMenu->addAction("Форматировать код", this, &CodeEditor::formatCode);

    contextMenu->addSeparator();

    toggleBreakpointAction = contextMenu->addAction("Точка останова", this, [this]() {
        toggleBreakpoint(getCurrentLine());
    });

    contextMenu->addSeparator();

    foldAllAction = contextMenu->addAction("Свернуть всё", this, &CodeEditor::foldAll);
    unfoldAllAction = contextMenu->addAction("Развернуть всё", this, &CodeEditor::unfoldAll);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &CodeEditor::customContextMenuRequested, this, [this](const QPoint& pos) {
        contextMenu->exec(mapToGlobal(pos));
    });
}

// ============================================================================
// Операции с файлами
// ============================================================================

bool CodeEditor::loadFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot open file: " + path);
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    setPlainText(in.readAll());
    file.close();

    filePath = path;
    modified = false;
    setModified(false);

    // Обновление подсветки
    if (highlighter) {
        highlighter->setFilePath(path);
        highlighter->rehighlight();
    }

    // Анализ сворачивания
    analyzeFoldRegions();

    // Загрузка истории правок из StoryManager
    StoryManager& storyManager = StoryManager::getInstance();
    storyManager.loadStory(path);

    // Загрузка информации о возрасте для подсветки
    if (highlighter) {
        highlighter->loadAgeInfoFromStory(path);
    }

    LOG_INFO("File loaded: " + path.toStdString());
    return true;
}

bool CodeEditor::saveFile() {
    if (filePath.isEmpty()) {
        return saveFileAs("");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot save file: " + filePath);
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << toPlainText();
    file.close();

    modified = false;
    setModified(false);

    LOG_INFO("File saved: " + filePath.toStdString());
    return true;
}

bool CodeEditor::saveFileAs(const QString& path) {
    QString fileName = path;
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(this, "Save File", filePath,
                                                "Proxima Files (*.prx);;All Files (*)");
    }

    if (fileName.isEmpty()) {
        return false;
    }

    filePath = fileName;
    return saveFile();
}

void CodeEditor::setModified(bool m) {
    modified = m;
    emit fileModified(m);
}

// ============================================================================
// Позиция курсора
// ============================================================================

int CodeEditor::getCurrentLine() const {
    return textCursor().blockNumber() + 1;  // 1-based индексация
}

int CodeEditor::getCurrentColumn() const {
    return textCursor().columnNumber() + 1;  // 1-based индексация
}

int CodeEditor::getCurrentPosition() const {
    return textCursor().position();
}

void CodeEditor::goToLine(int line) {
    if (line < 1 || line > blockCount()) return;

    QTextCursor cursor(document()->findBlockByNumber(line - 1));
    setTextCursor(cursor);
    setFocus();
    centerCursor();
}

void CodeEditor::goToPosition(int position) {
    QTextCursor cursor(document());
    cursor.setPosition(position);
    setTextCursor(cursor);
    setFocus();
    centerCursor();
}

void CodeEditor::selectLine(int line) {
    if (line < 1 || line > blockCount()) return;

    QTextCursor cursor(document()->findBlockByNumber(line - 1));
    cursor.select(QTextCursor::BlockUnderCursor);
    setTextCursor(cursor);
}

void CodeEditor::selectRange(int startLine, int startCol, int endLine, int endCol) {
    QTextCursor cursor(document());
    cursor.setPosition(document()->findBlockByNumber(startLine - 1).position() + startCol - 1);
    cursor.setPosition(document()->findBlockByNumber(endLine - 1).position() + endCol - 1,
                      QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

// ============================================================================
// Операции с кодом
// ============================================================================

QString CodeEditor::getSelectedCode() const {
    return textCursor().selectedText().replace("\u2029", "\n");
}

QString CodeEditor::getCurrentLineCode() const {
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    return cursor.selectedText().replace("\u2029", "\n");
}

QString CodeEditor::getCodeInRange(int startLine, int endLine) const {
    if (startLine < 1 || endLine > blockCount() || startLine > endLine) {
        return "";
    }

    QTextCursor startCursor(document()->findBlockByNumber(startLine - 1));
    QTextCursor endCursor(document()->findBlockByNumber(endLine - 1));
    endCursor.select(QTextCursor::BlockUnderCursor);

    startCursor.setPosition(startCursor.position());
    startCursor.setPosition(endCursor.selectionEnd(), QTextCursor::KeepAnchor);

    return startCursor.selectedText().replace("\u2029", "\n");
}

int CodeEditor::getSelectionStart() const {
    return textCursor().selectionStart() + 1;  // 1-based
}

int CodeEditor::getSelectionEnd() const {
    return textCursor().selectionEnd() + 1;  // 1-based
}

void CodeEditor::formatCode() {
    // В полной реализации - форматирование кода
    // Для примера - просто переподсветка
    if (highlighter) {
        highlighter->rehighlight();
    }

    emit codeFormatted();
}

void CodeEditor::indentSelection() {
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.setPosition(start);
        cursor.beginEditBlock();
        while (cursor.position() < end) {
            cursor.insertText("    ");
            end += 4;
            cursor.setPosition(cursor.block().next().position());
        }
        cursor.endEditBlock();
    }
}

void CodeEditor::unindentSelection() {
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.setPosition(start);
        cursor.beginEditBlock();
        while (cursor.position() < end) {
            QString blockText = cursor.block().text();
            int spaces = 0;
            while (spaces < blockText.length() && blockText[spaces] == ' ' && spaces < 4) {
                spaces++;
            }
            if (spaces > 0) {
                cursor.deleteChar();
                end--;
            }
            cursor.setPosition(cursor.block().next().position());
        }
        cursor.endEditBlock();
    }
}

void CodeEditor::commentSelection() {
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.setPosition(start);
        cursor.beginEditBlock();
        while (cursor.position() < end) {
            cursor.insertText("// ");
            end += 3;
            cursor.setPosition(cursor.block().next().position());
        }
        cursor.endEditBlock();
    }
}

void CodeEditor::uncommentSelection() {
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.setPosition(start);
        cursor.beginEditBlock();
        while (cursor.position() < end) {
            QString blockText = cursor.block().text();
            if (blockText.trimmed().startsWith("//")) {
                int pos = cursor.block().position();
                int commentPos = blockText.indexOf("//");
                cursor.setPosition(pos + commentPos);
                cursor.deleteChar();
                cursor.deleteChar();
                end -= 2;
            }
            cursor.setPosition(cursor.block().next().position());
        }
        cursor.endEditBlock();
    }
}

void CodeEditor::duplicateLine() {
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString lineText = cursor.selectedText();
    cursor.setPosition(cursor.selectionEnd());
    cursor.insertBlock();
    cursor.insertText(lineText);
    cursor.endEditBlock();
}

void CodeEditor::deleteLine() {
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.removeSelectedText();
    cursor.deletePreviousChar();
    cursor.endEditBlock();
}

void CodeEditor::moveLineUp() {
    // Перемещение строки вверх
    int currentLine = getCurrentLine();
    if (currentLine > 1) {
        QString lineText = getCurrentLineCode();
        deleteLine();
        goToLine(currentLine - 1);
        QTextCursor cursor = textCursor();
        cursor.insertBlock();
        cursor.insertText(lineText);
        goToLine(currentLine - 1);
    }
}

void CodeEditor::moveLineDown() {
    // Перемещение строки вниз
    int currentLine = getCurrentLine();
    if (currentLine < blockCount()) {
        QString lineText = getCurrentLineCode();
        deleteLine();
        goToLine(currentLine + 1);
        QTextCursor cursor = textCursor();
        cursor.insertText(lineText);
        cursor.insertBlock();
        goToLine(currentLine + 1);
    }
}

void CodeEditor::insertSnippet(const QString& snippet) {
    QTextCursor cursor = textCursor();
    cursor.insertText(snippet);
    emit snippetInserted(snippet);
}

void CodeEditor::surroundWith(const QString& before, const QString& after) {
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection()) {
        QString selectedText = getSelectedCode();
        cursor.insertText(before + selectedText + after);
    }
}

// ============================================================================
// Точки останова
// ============================================================================

void CodeEditor::toggleBreakpoint(int line) {
    if (line < 1 || line > blockCount()) return;

    int index = breakpoints.indexOf(line);
    if (index >= 0) {
        breakpoints.removeAt(index);
        breakpointEnabled.remove(line);
        breakpointConditions.remove(line);
        emit breakpointToggled(line, false);
    } else {
        breakpoints.append(line);
        std::sort(breakpoints.begin(), breakpoints.end());
        breakpointEnabled[line] = true;
        emit breakpointToggled(line, true);
    }

    viewport()->update();
}

bool CodeEditor::hasBreakpoint(int line) const {
    return breakpoints.contains(line);
}

void CodeEditor::clearBreakpoints() {
    breakpoints.clear();
    breakpointEnabled.clear();
    breakpointConditions.clear();
    viewport()->update();
}

void CodeEditor::setBreakpoints(const QVector<int>& bps) {
    breakpoints = bps;
    std::sort(breakpoints.begin(), breakpoints.end());

    for (int bp : breakpoints) {
        breakpointEnabled[bp] = true;
    }

    viewport()->update();
}

void CodeEditor::setBreakpointEnabled(int line, bool enabled) {
    if (breakpoints.contains(line)) {
        breakpointEnabled[line] = enabled;
        viewport()->update();
    }
}

bool CodeEditor::isBreakpointEnabled(int line) const {
    return breakpointEnabled.value(line, false);
}

// ============================================================================
// Режимы отображения
// ============================================================================

void CodeEditor::setDisplayMode(DisplayMode mode) {
    if (displayMode != mode) {
        displayMode = mode;
        if (highlighter) {
            highlighter->setDisplayMode(mode);
            highlighter->rehighlight();
        }
        viewport()->update();
    }
}

void CodeEditor::applyTypeHighlighting(const QMap<int, QString>& lineTypes) {
    typeHighlights = lineTypes;
    if (displayMode == DisplayMode::TypeHighlight && highlighter) {
        highlighter->setTypeInfo(lineTypes);
        highlighter->rehighlight();
    }
    viewport()->update();
}

void CodeEditor::applyDimensionHighlighting(const QMap<int, QString>& lineDimensions) {
    dimensionHighlights = lineDimensions;
    if (displayMode == DisplayMode::DimensionHighlight && highlighter) {
        highlighter->setDimensionInfo(lineDimensions);
        highlighter->rehighlight();
    }
    viewport()->update();
}

void CodeEditor::applyTimingHighlighting(const QMap<int, double>& lineTimings) {
    timingHighlights = lineTimings;
    if (displayMode == DisplayMode::TimingHighlight && highlighter) {
        highlighter->setTimingInfo(lineTimings);
        highlighter->rehighlight();
    }
    viewport()->update();
}

void CodeEditor::applyFrequencyHighlighting(const QMap<int, int>& lineFrequencies) {
    frequencyHighlights = lineFrequencies;
    if (displayMode == DisplayMode::FrequencyHighlight && highlighter) {
        highlighter->setFrequencyInfo(lineFrequencies);
        highlighter->rehighlight();
    }
    viewport()->update();
}

void CodeEditor::applyGitDiff(const QString& diff) {
    gitDiffData = diff;
    if (displayMode == DisplayMode::GitDiff && highlighter) {
        highlighter->setGitDiffInfo(diff);
        highlighter->rehighlight();
    }
    viewport()->update();
}

void CodeEditor::applyAuthorHighlighting(const QMap<int, QString>& lineAuthors) {
    authorHighlights = lineAuthors;
    if (displayMode == DisplayMode::AuthorHighlight && highlighter) {
        highlighter->setAuthorInfo(lineAuthors);
        highlighter->rehighlight();
    }
    viewport()->update();
}

void CodeEditor::applyAgeHighlighting(const QMap<int, QDateTime>& lineAges) {
    ageHighlights = lineAges;
    if (displayMode == DisplayMode::AgeHighlight && highlighter) {
        highlighter->setAgeInfo(lineAges);
        highlighter->rehighlight();
    }
    viewport()->update();
}

// ============================================================================
// Сворачивание кода
// ============================================================================

void CodeEditor::toggleFold(int line) {
    foldingManager->toggleFold(line);
    updateFoldRegions();
    viewport()->update();
}

bool CodeEditor::isFolded(int line) const {
    return foldedLines.value(line, false);
}

void CodeEditor::foldAll() {
    foldingManager->foldAll();
    updateFoldRegions();
    viewport()->update();
}

void CodeEditor::unfoldAll() {
    foldingManager->unfoldAll();
    updateFoldRegions();
    viewport()->update();
}

void CodeEditor::foldLevel(int level) {
    foldingManager->foldLevel(level);
    updateFoldRegions();
    viewport()->update();
}

void CodeEditor::foldMethods() {
    foldingManager->foldMethods();
    updateFoldRegions();
    viewport()->update();
}

void CodeEditor::foldClasses() {
    foldingManager->foldClasses();
    updateFoldRegions();
    viewport()->update();
}

void CodeEditor::foldRegions() {
    foldingManager->foldRegions();
    updateFoldRegions();
    viewport()->update();
}

void CodeEditor::analyzeFoldRegions() {
    foldingManager->analyzeDocument();
    updateFoldRegions();
}

void CodeEditor::updateFoldRegions() {
    foldRegions = foldingManager->getFoldRegions();

    foldedLines.clear();
    for (const FoldRegion& region : foldRegions) {
        if (region.isFolded) {
            for (int line = region.startLine + 1; line < region.endLine; line++) {
                foldedLines[line] = true;
            }
        }
    }

    // Пересчёт видимых строк
    document()->documentLayout()->invalidate();
}

bool CodeEditor::isLineVisible(int line) const {
    return !foldedLines.value(line, false);
}

int CodeEditor::getFirstVisibleLine() const {
    for (int line = 1; line <= blockCount(); line++) {
        if (isLineVisible(line)) {
            return line;
        }
    }
    return 1;
}

int CodeEditor::getLastVisibleLine() const {
    for (int line = blockCount(); line >= 1; line--) {
        if (isLineVisible(line)) {
            return line;
        }
    }
    return blockCount();
}

// ============================================================================
// Выполнение и отладка
// ============================================================================

void CodeEditor::setCurrentExecutionLine(int line) {
    if (currentExecutionLine != line) {
        currentExecutionLine = line;
        emit executionLineChanged(line);
        viewport()->update();

        // Переход к строке выполнения
        if (line > 0) {
            goToLine(line);
        }
    }
}

void CodeEditor::clearExecutionLine() {
    currentExecutionLine = -1;
    emit executionLineChanged(-1);
    viewport()->update();
}

void CodeEditor::markErrorLine(int line, const QString& message) {
    if (!errorLines.contains(line)) {
        errorLines.append(line);
    }
    errorMessages[line] = message;
    viewport()->update();
}

void CodeEditor::clearErrorLines() {
    errorLines.clear();
    errorMessages.clear();
    viewport()->update();
}

void CodeEditor::markWarningLine(int line, const QString& message) {
    if (!warningLines.contains(line)) {
        warningLines.append(line);
    }
    warningMessages[line] = message;
    viewport()->update();
}

void CodeEditor::clearWarningLines() {
    warningLines.clear();
    warningMessages.clear();
    viewport()->update();
}

// ============================================================================
// Визуализация
// ============================================================================

void CodeEditor::showArrayVisualization(const QString& varName, const RuntimeValue& value) {
    emit visualizationRequested(varName);
    // В полной реализации - открытие VectorPlot
}

void CodeEditor::showMatrixVisualization(const QString& varName, const RuntimeValue& value) {
    emit visualizationRequested(varName);
    // В полной реализации - открытие MatrixView
}

void CodeEditor::showLayerVisualization(const QString& varName, const RuntimeValue& value) {
    emit visualizationRequested(varName);
    // В полной реализации - открытие Layer3DView
}

void CodeEditor::showCollectionVisualization(const QString& varName, const RuntimeValue& value) {
    emit visualizationRequested(varName);
    // В полной реализации - открытие CollectionTable
}

void CodeEditor::showObjectVisualization(const QString& varName, const RuntimeValue& value) {
    emit visualizationRequested(varName);
    // В полной реализации - открытие ObjectInspector
}

// ============================================================================
// Автодополнение
// ============================================================================

void CodeEditor::setAutoCompletionEnabled(bool enable) {
    autoCompletionEnabled = enable;
}

void CodeEditor::triggerAutoCompletion() {
    if (!autoCompletionEnabled) return;
    showCompletionPopup();
}

void CodeEditor::hideCompletionPopup() {
    if (completionPopup) {
        completionPopup->hide();
    }
}

void CodeEditor::setCompletionItems(const QStringList& items) {
    completionItems = items;
    updateCompletionPopup();
}

void CodeEditor::showCompletionPopup() {
    if (!autoCompletionEnabled || !completionPopup) return;

    QString prefix = getWordUnderCursor();
    QStringList suggestions = getCompletionSuggestions(prefix);

    if (suggestions.isEmpty()) {
        hideCompletionPopup();
        return;
    }

    completionPopup->clear();
    for (const QString& item : suggestions) {
        completionPopup->addItem(item);
    }

    updateCompletionPopup();
    completionPopup->show();
    completionPopup->setFocus();
}

void CodeEditor::updateCompletionPopup() {
    if (!completionPopup || completionPopup->currentRow() < 0) return;

    // Позиционирование popup у курсора
    QTextCursor cursor = textCursor();
    QRect cursorRect = this->cursorRect(cursor);
    QPoint pos = mapToGlobal(cursorRect.bottomLeft());

    completionPopup->move(pos);
}

QString CodeEditor::getWordUnderCursor() const {
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}

QStringList CodeEditor::getCompletionSuggestions(const QString& prefix) const {
    QStringList suggestions;

    // Ключевые слова Proxima
    QStringList keywords = {"if", "else", "elseif", "end", "for", "in", "while", "do",
                           "return", "break", "continue", "class", "interface", "function",
                           "vector", "matrix", "layer", "collection", "time", "string",
                           "int", "int32", "int64", "single", "double", "bool", "char"};

    for (const QString& kw : keywords) {
        if (kw.startsWith(prefix, Qt::CaseInsensitive)) {
            suggestions.append(kw);
        }
    }

    // Переменные из текущего контекста
    suggestions.append(completionItems);

    suggestions.removeDuplicates();
    suggestions.sort();

    return suggestions;
}

void CodeEditor::insertCompletion(const QString& completion) {
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.insertText(completion);
    setTextCursor(cursor);
    hideCompletionPopup();
}

// ============================================================================
// Настройки отображения
// ============================================================================

void CodeEditor::setShowLineNumbers(bool show) {
    showLineNumbers = show;
    updateLineNumberAreaWidth(0);
}

bool CodeEditor::getShowLineNumbers() const {
    return showLineNumbers;
}

void CodeEditor::setShowWhitespace(bool show) {
    showWhitespace = show;
    viewport()->update();
}

bool CodeEditor::getShowWhitespace() const {
    return showWhitespace;
}

void CodeEditor::setShowIndentGuides(bool show) {
    showIndentGuides = show;
    viewport()->update();
}

bool CodeEditor::getShowIndentGuides() const {
    return showIndentGuides;
}

void CodeEditor::setHighlightCurrentLine(bool highlight) {
    highlightCurrentLine = highlight;
    highlightCurrentLine();
}

bool CodeEditor::getHighlightCurrentLine() const {
    return highlightCurrentLine;
}

void CodeEditor::setWordWrap(bool wrap) {
    wordWrap = wrap;
    setLineWrapMode(wrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

bool CodeEditor::getWordWrap() const {
    return wordWrap;
}

// ============================================================================
// Статистика
// ============================================================================

int CodeEditor::getWordCount() const {
    if (wordCount > 0) return wordCount;

    QString text = toPlainText();
    wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    return wordCount;
}

// ============================================================================
// События
// ============================================================================

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                      lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    // Обработка popup автодополнения
    if (completionPopup && completionPopup->isVisible()) {
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
            QApplication::sendEvent(completionPopup, event);
            return;
        }
        if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Return) {
            if (completionPopup->currentRow() >= 0) {
                QListWidgetItem* item = completionPopup->currentItem();
                if (item) {
                    insertCompletion(item->text());
                }
            }
            return;
        }
        if (event->key() == Qt::Key_Escape) {
            hideCompletionPopup();
            return;
        }
    }

    // Обработка Tab
    if (event->key() == Qt::Key_Tab) {
        if (!(event->modifiers() & Qt::ControlModifier)) {
            insertPlainText("    ");
            event->accept();
            return;
        }
    }

    // Обработка Enter
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextCursor cursor = textCursor();
        QString currentLineText = cursor.block().text();

        // Автоматический отступ на новой строке
        int indent = 0;
        for (const QChar& ch : currentLineText) {
            if (ch == ' ') indent++;
            else if (ch == '\t') indent += 4;
            else break;
        }

        QPlainTextEdit::keyPressEvent(event);

        if (indent > 0) {
            cursor = textCursor();
            cursor.insertText(QString(indent, ' '));
        }

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

    // Дополнительная отрисовка
    QPainter painter(viewport());
    drawBreakpoints(painter);
    drawExecutionLine(painter);
    drawFoldMarkers(painter);
    drawErrorMarkers(painter);
    drawWarningMarkers(painter);

    if (showIndentGuides) {
        drawIndentGuides(painter);
    }

    if (showWhitespace) {
        drawWhitespace(painter);
    }
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event) {
    contextMenu->exec(event->globalPos());
}

void CodeEditor::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CodeEditor::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QString filePath = url.toLocalFile();
        if (filePath.endsWith(".prx")) {
            loadFile(filePath);
        }
    }
    event->acceptProposedAction();
}

void CodeEditor::wheelEvent(QWheelEvent *event) {
    // Zoom с Ctrl+колесо
    if (event->modifiers() & Qt::ControlModifier) {
        QFont font = this->font();
        int delta = event->angleDelta().y();
        int newSize = font.pointSize() + (delta > 0 ? 1 : -1);

        if (newSize >= 8 && newSize <= 72) {
            font.setPointSize(newSize);
            setFont(font);
            updateLineNumberAreaWidth(0);
        }
        event->accept();
    } else {
        QPlainTextEdit::wheelEvent(event);
    }
}

// ============================================================================
// Слоты
// ============================================================================

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    if (showLineNumbers) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    } else {
        setViewportMargins(0, 0, 0, 0);
    }
}

void CodeEditor::highlightCurrentLine() {
    if (!highlightCurrentLine) {
        setExtraSelections(QList<QTextEdit::ExtraSelection>());
        return;
    }

    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(40, 40, 40);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        lineNumberArea->scroll(0, dy);
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEditor::onCompletionItemActivated(const QString& item) {
    insertCompletion(item);
}

void CodeEditor::onFoldTimerTimeout() {
    analyzeFoldRegions();
}

// ============================================================================
// Методы отрисовки
// ============================================================================

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), lineNumberAreaColor());

    if (!showLineNumbers) return;

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            // 1-based нумерация строк
            QString number = QString::number(blockNumber + 1);

            // Подсветка строки с точкой останова
            int line = blockNumber + 1;
            if (breakpoints.contains(line)) {
                if (isBreakpointEnabled(line)) {
                    painter.setPen(QColor(255, 100, 100));
                    painter.setFont(QFont("Consolas", 10, QFont::Bold));
                } else {
                    painter.setPen(QColor(150, 150, 150));
                    painter.setFont(QFont("Consolas", 10));
                }
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
    if (!showLineNumbers) return 0;

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
    painter.setBrush(QColor(255, 100, 100));
    painter.setPen(Qt::NoPen);

    for (int line : breakpoints) {
        if (!isLineVisible(line)) continue;

        QTextBlock block = document()->findBlockByNumber(line - 1);
        if (block.isValid()) {
            int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
            int height = qRound(blockBoundingRect(block).height());

            // Рисуем кружок точки останова
            painter.drawEllipse(2, y + height/2 - 4, 8, 8);
        }
    }
}

void CodeEditor::drawExecutionLine(QPainter& painter) {
    if (currentExecutionLine < 1) return;

    QTextBlock block = document()->findBlockByNumber(currentExecutionLine - 1);
    if (!block.isValid()) return;

    int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int height = qRound(blockBoundingRect(block).height());

    // Рисуем стрелку выполнения
    painter.setBrush(QColor(100, 200, 100));
    painter.setPen(Qt::NoPen);

    QPolygon arrow;
    arrow << QPoint(0, y) << QPoint(10, y + height/2) << QPoint(0, y + height);
    painter.drawPolygon(arrow);

    // Подсветка строки выполнения
    painter.setBrush(QColor(50, 80, 50, 100));
    painter.drawRect(10, y, viewport()->width(), height);
}

void CodeEditor::drawFoldMarkers(QPainter& painter) {
    // Отрисовка маркеров сворачивания
    painter.setPen(getFoldMarkerColor());
    painter.setBrush(Qt::NoBrush);

    for (const FoldRegion& region : foldRegions) {
        if (!isLineVisible(region.foldLine)) continue;

        QTextBlock block = document()->findBlockByNumber(region.foldLine - 1);
        if (block.isValid()) {
            int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
            int height = qRound(blockBoundingRect(block).height());

            // Рисуем квадратик сворачивания
            int size = 8;
            int x = lineNumberAreaWidth() + 5;
            int posY = y + height/2 - size/2;

            painter.drawRect(x, posY, size, size);

            // Плюс или минус внутри
            painter.drawLine(x + 2, posY + size/2, x + size - 2, posY + size/2);
            if (!region.isFolded) {
                painter.drawLine(x + size/2, posY + 2, x + size/2, posY + size - 2);
            }
        }
    }
}

void CodeEditor::drawErrorMarkers(QPainter& painter) {
    painter.setBrush(QColor(255, 100, 100));
    painter.setPen(Qt::NoPen);

    for (int line : errorLines) {
        if (!isLineVisible(line)) continue;

        QTextBlock block = document()->findBlockByNumber(line - 1);
        if (block.isValid()) {
            int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
            int height = qRound(blockBoundingRect(block).height());

            // Рисуем маркер ошибки справа
            painter.drawRect(viewport()->width() - 5, y + height/2 - 2, 4, 4);
        }
    }
}

void CodeEditor::drawWarningMarkers(QPainter& painter) {
    painter.setBrush(QColor(255, 200, 100));
    painter.setPen(Qt::NoPen);

    for (int line : warningLines) {
        if (!isLineVisible(line)) continue;

        QTextBlock block = document()->findBlockByNumber(line - 1);
        if (block.isValid()) {
            int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
            int height = qRound(blockBoundingRect(block).height());

            // Рисуем маркер предупреждения справа
            painter.drawEllipse(viewport()->width() - 5, y + height/2 - 2, 4, 4);
        }
    }
}

void CodeEditor::drawIndentGuides(QPainter& painter) {
    painter.setPen(QColor(60, 60, 60));

    QTextBlock block = firstVisibleBlock();
    while (block.isValid()) {
        if (block.isVisible()) {
            int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
            int height = qRound(blockBoundingRect(block).height());

            QString text = block.text();
            int x = 0;
            for (int i = 0; i < text.length() && text[i] == ' '; i++) {
                if ((i + 1) % 4 == 0) {
                    x = fontMetrics().horizontalAdvance(text.left(i + 1));
                    painter.drawLine(x, y, x, y + height);
                }
            }
        }
        block = block.next();
    }
}

void CodeEditor::drawWhitespace(QPainter& painter) {
    // Отрисовка пробелов и табуляций
    painter.setPen(QColor(80, 80, 80));

    QTextBlock block = firstVisibleBlock();
    while (block.isValid()) {
        if (block.isVisible()) {
            int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
            QString text = block.text();

            for (int i = 0; i < text.length(); i++) {
                if (text[i] == ' ') {
                    int x = fontMetrics().horizontalAdvance(text.left(i));
                    painter.drawPoint(x + 3, y + fontMetrics().height()/2);
                } else if (text[i] == '\t') {
                    int x = fontMetrics().horizontalAdvance(text.left(i));
                    painter.drawLine(x, y + fontMetrics().height()/2,
                                   x + fontMetrics().horizontalAdvance("    "),
                                   y + fontMetrics().height()/2);
                }
            }
        }
        block = block.next();
    }
}

void CodeEditor::drawCurrentLineHighlight(QPainter& painter) {
    if (!highlightCurrentLine) return;

    QTextBlock block = textCursor().block();
    if (block.isValid()) {
        int y = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int height = qRound(blockBoundingRect(block).height());

        painter.fillRect(0, y, viewport()->width(), height, QColor(40, 40, 40));
    }
}

QString CodeEditor::lineNumberAreaColor() const {
    return QColor(30, 30, 30);
}

QColor CodeEditor::getBreakpointColor() const {
    return QColor(255, 100, 100);
}

QColor CodeEditor::getExecutionLineColor() const {
    return QColor(100, 200, 100);
}

QColor CodeEditor::getFoldMarkerColor() const {
    return QColor(150, 150, 150);
}

// ============================================================================
// Визуализация данных
// ============================================================================

RuntimeValue CodeEditor::parseArrayValue(const QString& code) const {
    // Парсинг значения массива из кода
    RuntimeValue value;
    // В полной реализации - парсинг runtime value
    return value;
}

RuntimeValue CodeEditor::parseMatrixValue(const QString& code) const {
    // Парсинг значения матрицы из кода
    RuntimeValue value;
    // В полной реализации - парсинг runtime value
    return value;
}

RuntimeValue CodeEditor::parseLayerValue(const QString& code) const {
    // Парсинг значения слоя из кода
    RuntimeValue value;
    // В полной реализации - парсинг runtime value
    return value;
}

// ============================================================================
// Информация об авторах и правках (НОВОЕ)
// ============================================================================

bool CodeEditor::registerAuthor(const QString& name, const QString& email) {
    if (name.isEmpty()) {
        return false;
    }

    // Проверка - уже зарегистрирован ли автор
    if (authors.contains(name)) {
        return true;  // Уже зарегистрирован
    }

    // Генерация уникального цвета для автора
    int order = nextAuthorOrder++;
    QColor color = generateUniqueAuthorColor(order);

    AuthorInfo info;
    info.name = name;
    info.email = email;
    info.color = color;
    info.registrationOrder = order;

    authors[name] = info;

    LOG_DEBUG("Author registered: " + name.toStdString() +
              " with color #" + color.name().toStdString());

    emit authorRegistered(name, color);

    return true;
}

AuthorInfo CodeEditor::getAuthorInfo(const QString& name) const {
    if (authors.contains(name)) {
        return authors[name];
    }
    return AuthorInfo();
}

QVector<AuthorInfo> CodeEditor::getAllAuthors() const {
    return authors.values();
}

void CodeEditor::setLineEditInfo(int line, const LineEditInfo& info) {
    if (line < 1 || line > blockCount()) return;

    lineEditInfo[line] = info;

    // Обновление информации для подсветки
    if (displayMode == DisplayMode::AuthorHighlight) {
        QMap<int, QString> authorMap;
        authorMap[line] = info.author;
        applyAuthorHighlighting(authorMap);
    }

    if (displayMode == DisplayMode::AgeHighlight) {
        QMap<int, QDateTime> ageMap;
        ageMap[line] = info.editTime;
        applyAgeHighlighting(ageMap);
    }

    emit lineEditInfoChanged(line, info);
}

LineEditInfo CodeEditor::getLineEditInfo(int line) const {
    if (lineEditInfo.contains(line)) {
        return lineEditInfo[line];
    }
    return LineEditInfo();
}

void CodeEditor::setAllLineEditInfo(const QMap<int, LineEditInfo>& lineInfoMap) {
    lineEditInfo = lineInfoMap;

    // Обновление подсветки для всех строк
    if (displayMode == DisplayMode::AuthorHighlight) {
        QMap<int, QString> authorMap;
        for (auto it = lineInfoMap.begin(); it != lineInfoMap.end(); ++it) {
            authorMap[it.key()] = it.value().author;
        }
        applyAuthorHighlighting(authorMap);
    }

    if (displayMode == DisplayMode::AgeHighlight) {
        QMap<int, QDateTime> ageMap;
        for (auto it = lineInfoMap.begin(); it != lineInfoMap.end(); ++it) {
            ageMap[it.key()] = it.value().editTime;
        }
        applyAgeHighlighting(ageMap);
    }
}

QMap<int, LineEditInfo> CodeEditor::getAllLineEditInfo() const {
    return lineEditInfo;
}

void CodeEditor::clearLineEditInfo() {
    lineEditInfo.clear();

    if (displayMode == DisplayMode::AuthorHighlight ||
        displayMode == DisplayMode::AgeHighlight) {
        setDisplayMode(DisplayMode::Standard);
    }
}

QColor CodeEditor::generateUniqueAuthorColor(int registrationOrder) {
    // Генерация уникального цвета на основе порядка регистрации
    // Используем золотое сечение для равномерного распределения цветов

    const double goldenRatio = 0.618033988749895;
    double hue = fmod(registrationOrder * goldenRatio, 1.0);

    // Преобразование HSV в RGB
    QColor color;
    color.setHsvF(hue, 0.7, 0.9);  // Насыщенность 70%, яркость 90%

    // Проверка на слишком тёмные/светлые цвета
    int brightness = (color.red() * 299 + color.green() * 587 + color.blue() * 114) / 1000;

    if (brightness < 100) {
        // Слишком тёмный - осветляем
        color = color.lighter(120);
    } else if (brightness > 200) {
        // Слишком светлый - затемняем
        color = color.darker(120);
    }

    return color;
}

QColor CodeEditor::getAgeColor(qint64 ageMinutes) {
    // Определение цвета на основе возраста правки в минутах

    if (ageMinutes < 5) {
        // Менее 5 минут - ярко-зелёный (очень свежая правка)
        return QColor(100, 255, 100);
    } else if (ageMinutes < 30) {
        // Менее 30 минут - зелёный (свежая правка)
        return QColor(50, 200, 50);
    } else if (ageMinutes < 60) {
        // Менее часа - светло-зелёный
        return QColor(50, 180, 50);
    } else if (ageMinutes < 120) {
        // Менее 2 часов - жёлто-зелёный
        return QColor(150, 200, 50);
    } else if (ageMinutes < 240) {
        // Менее 4 часов - жёлтый
        return QColor(200, 200, 50);
    } else if (ageMinutes < 480) {
        // Менее 8 часов - оранжево-жёлтый
        return QColor(220, 150, 50);
    } else if (ageMinutes < 1440) {
        // Менее дня - оранжевый
        return QColor(255, 120, 50);
    } else if (ageMinutes < 2880) {
        // Менее 2 дней - оранжево-красный
        return QColor(255, 100, 50);
    } else if (ageMinutes < 4320) {
        // Менее 3 дней - красный
        return QColor(255, 80, 50);
    } else if (ageMinutes < 10080) {
        // Менее недели - тёмно-красный
        return QColor(200, 50, 50);
    } else if (ageMinutes < 43200) {
        // Менее месяца - бордовый
        return QColor(150, 50, 50);
    } else {
        // Старше месяца - тёмно-бордовый (очень старая правка)
        return QColor(100, 40, 40);
    }
}

QString CodeEditor::getAgeString(qint64 ageMinutes) {
    if (ageMinutes < 1) {
        return "Только что";
    } else if (ageMinutes < 60) {
        return QString("%1 мин.").arg(ageMinutes);
    } else if (ageMinutes < 1440) {
        int hours = ageMinutes / 60;
        return QString("%1 ч.").arg(hours);
    } else if (ageMinutes < 10080) {
        int days = ageMinutes / 1440;
        return QString("%1 дн.").arg(days);
    } else if (ageMinutes < 43200) {
        int weeks = ageMinutes / 10080;
        return QString("%1 нед.").arg(weeks);
    } else {
        int months = ageMinutes / 43200;
        return QString("%1 мес.").arg(months);
    }
}

int CodeEditor::findAvailableColorSlot() const {
    // Поиск доступного слота для цвета (для будущей реализации)
    return authors.size();
}

bool CodeEditor::isColorUsed(const QColor& color) const {
    // Проверка - используется ли уже такой цвет
    for (auto it = authors.begin(); it != authors.end(); ++it) {
        if (it.value().color == color) {
            return true;
        }
    }
    return false;
}

void CodeEditor::updateAuthorColors() {
    // Обновление цветов авторов (при изменении темы и т.п.)
    int order = 0;
    for (auto it = authors.begin(); it != authors.end(); ++it) {
        it->color = generateUniqueAuthorColor(order);
        it->registrationOrder = order;
        order++;
    }
}

void CodeEditor::trackTextEdit(int position, int charsRemoved, int charsAdded) {
    QTextDocument* doc = document();
    if (!doc) return;
    
    // Определение изменённых строк
    QTextBlock startBlock = doc->findBlock(position);
    QTextBlock endBlock = doc->findBlock(position + charsAdded);
    
    int startLine = startBlock.blockNumber() + 1;
    int endLine = endBlock.blockNumber() + 1;
    
    // Извлечение изменённого текста
    QTextCursor cursor(doc);
    cursor.setPosition(position);
    cursor.setPosition(position + charsAdded, QTextCursor::KeepAnchor);
    QString changedText = cursor.selectedText();
    
    // Добавление записи в историю
    StoryManager& storyManager = StoryManager::getInstance();
    
    if (charsAdded > 0) {
        storyManager.addEditEntry(filePath, EditOperation::Add, 
                                 changedText, startLine, endLine);
    }
    
    if (charsRemoved > 0) {
        // Для удаления нужно сохранить удалённый текст
        // В полной реализации - отслеживание удалённого текста
        storyManager.addEditEntry(filePath, EditOperation::Remove, 
                                 "", startLine, endLine);
    }
}

// Добавить метод для отмены последней правки:

bool CodeEditor::undoLastEdit() {
    StoryManager& storyManager = StoryManager::getInstance();
    return storyManager.undoLastEdit(filePath);
}

// Добавить метод для возврата правки:

bool CodeEditor::redoLastEdit() {
    StoryManager& storyManager = StoryManager::getInstance();
    return storyManager.redoLastEdit(filePath);
}

} // namespace proxima
