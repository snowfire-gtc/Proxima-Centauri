#include "ConsoleTab.h"
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPrinter>
#include <QPrintDialog>
#include <QDateTime>
#include <QToolTip>
#include <QCompleter>
#include <QRegularExpression>
#include <QBuffer>
#include <QTextCodec>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

ConsoleTab::ConsoleTab(ConsoleTabType type, QWidget *parent)
    : QWidget(parent)
    , tabType(type)
    , textEdit(nullptr)
    , statusLabel(nullptr)
    , statusWidget(nullptr)
    , maxLines(10000)
    , autoScroll(true)
    , lineWrap(false)
    , currentFilter("")
    , colorScheme("dark")
    , totalMessages(0)
    , modified(false)
    , lastModified(0)
    , contextMenu(nullptr)
    , searchIndex(-1) {
    
    // Настройка шрифта по умолчанию
    defaultFont = QFont("Consolas", 10);
    defaultFont.setStyleHint(QFont::Monospace);
    defaultFont.setFixedPitch(true);
    
    // Инициализация статистики
    resetStatistics();
    
    // Инициализация цветовых схем
    initializeColorSchemes();
    
    setupUI();
    setupConnections();
    setupContextMenu();
    applyColorScheme();
    
    LOG_DEBUG("ConsoleTab created, type: " + QString::number(static_cast<int>(type)));
}

ConsoleTab::~ConsoleTab() {
    LOG_DEBUG("ConsoleTab destroyed, type: " + QString::number(static_cast<int>(tabType)));
}

// ============================================================================
// Настройка UI
// ============================================================================

void ConsoleTab::setupUI() {
    // Главная компоновка
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    
    // Текстовое поле
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setLineWrapMode(lineWrap ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setFont(defaultFont);
    textEdit->setAcceptRichText(true);
    textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    textEdit->setUndoRedoEnabled(false);
    textEdit->setTabStopDistance(4 * QFontMetrics(defaultFont).horizontalAdvance(' '));
    
    // Стиль текстового поля
    textEdit->setStyleSheet(
        "QTextEdit { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  selection-background-color: #264f78; "
        "  selection-color: #ffffff; "
        "  border: none; "
        "  padding: 5px; "
        "}"
        "QScrollBar:vertical { "
        "  background-color: #2d2d2d; "
        "  width: 12px; "
        "  margin: 0px; "
        "}"
        "QScrollBar::handle:vertical { "
        "  background-color: #5a5a5a; "
        "  min-height: 20px; "
        "  border-radius: 6px; "
        "}"
        "QScrollBar::handle:vertical:hover { "
        "  background-color: #6a6a6a; "
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "  height: 0px; "
        "}"
        "QScrollBar:horizontal { "
        "  background-color: #2d2d2d; "
        "  height: 12px; "
        "  margin: 0px; "
        "}"
        "QScrollBar::handle:horizontal { "
        "  background-color: #5a5a5a; "
        "  min-width: 20px; "
        "  border-radius: 6px; "
        "}"
    );
    
    mainLayout->addWidget(textEdit, 1);
    
    // Строка состояния
    statusWidget = new QWidget(this);
    QHBoxLayout* statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(5, 2, 5, 2);
    statusLayout->setSpacing(10);
    
    statusLabel = new QLabel(this);
    statusLabel->setFont(QFont("Segoe UI", 9));
    statusLabel->setStyleSheet("color: #808080;");
    statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusLayout->addWidget(statusLabel, 1);
    
    statusWidget->setStyleSheet(
        "QWidget { "
        "  background-color: #2d2d2d; "
        "  border-top: 1px solid #3e3e3e; "
        "}"
    );
    statusWidget->setFixedHeight(24);
    
    mainLayout->addWidget(statusWidget);
    
    // Обновление строки состояния
    updateStatusBar();
}

void ConsoleTab::setupConnections() {
    // Изменение текста
    connect(textEdit->document(), &QTextDocument::contentsChanged, 
            this, &ConsoleTab::onTextChanged);
    
    // Изменение выделения
    connect(textEdit, &QTextEdit::copyAvailable, this, [this](bool available) {
        if (copyAction) copyAction->setEnabled(available);
        if (cutAction) cutAction->setEnabled(available);
    });
    
    // Контекстное меню
    connect(textEdit, &QTextEdit::customContextMenuRequested, this,
            [this](const QPoint& pos) {
        emit contextMenuRequested(pos);
        contextMenu->exec(textEdit->mapToGlobal(pos));
    });
    
    // Действия контекстного меню
    connect(copyAction, &QAction::triggered, this, &ConsoleTab::copy);
    connect(pasteAction, &QAction::triggered, this, &ConsoleTab::paste);
    connect(cutAction, &QAction::triggered, this, &ConsoleTab::cut);
    connect(selectAllAction, &QAction::triggered, this, &ConsoleTab::selectAll);
    connect(clearAction, &QAction::triggered, this, &ConsoleTab::clear);
    connect(saveAction, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this, "Сохранить вывод консоли", "", 
            "Text Files (*.txt);;HTML Files (*.html);;All Files (*)");
        if (!path.isEmpty()) {
            if (path.endsWith(".html", Qt::CaseInsensitive)) {
                exportToHtml(path);
            } else {
                saveToFile(path);
            }
        }
    });
    connect(findAction, &QAction::triggered, this, &ConsoleTab::onFindText);
    connect(clearFilterAction, &QAction::triggered, this, &ConsoleTab::clearFilter);
    connect(scrollToBottomAction, &QAction::triggered, this, &ConsoleTab::scrollToBottom);
    connect(scrollToTopAction, &QAction::triggered, this, &ConsoleTab::scrollToTop);
}

void ConsoleTab::setupContextMenu() {
    contextMenu = new QMenu(this);
    
    copyAction = contextMenu->addAction("Копировать", this, &ConsoleTab::copy);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(false);
    
    pasteAction = contextMenu->addAction("Вставить", this, &ConsoleTab::paste);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setEnabled(!textEdit->isReadOnly());
    
    cutAction = contextMenu->addAction("Вырезать", this, &ConsoleTab::cut);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setEnabled(false);
    
    contextMenu->addSeparator();
    
    selectAllAction = contextMenu->addAction("Выделить всё", this, &ConsoleTab::selectAll);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    
    contextMenu->addSeparator();
    
    clearAction = contextMenu->addAction("Очистить", this, &ConsoleTab::clear);
    clearAction->setShortcut(QKeySequence::Clear);
    
    saveAction = contextMenu->addAction("Сохранить...", this);
    saveAction->setShortcut(QKeySequence::Save);
    
    exportAction = contextMenu->addAction("Экспорт в HTML...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this, "Экспорт в HTML", "", "HTML Files (*.html)");
        if (!path.isEmpty()) {
            exportToHtml(path);
        }
    });
    
    contextMenu->addSeparator();
    
    findAction = contextMenu->addAction("Найти...", this);
    findAction->setShortcut(QKeySequence::Find);
    
    filterAction = contextMenu->addAction("Фильтр...", this, [this]() {
        bool ok;
        QString filter = QInputDialog::getText(this, "Фильтр", "Введите текст фильтра:",
                                              QLineEdit::Normal, currentFilter, &ok);
        if (ok) {
            setFilter(filter);
        }
    });
    
    clearFilterAction = contextMenu->addAction("Очистить фильтр", this);
    clearFilterAction->setEnabled(!currentFilter.isEmpty());
    
    contextMenu->addSeparator();
    
    scrollToBottomAction = contextMenu->addAction("В конец", this);
    scrollToBottomAction->setShortcut(QKeySequence::End);
    
    scrollToTopAction = contextMenu->addAction("В начало", this);
    scrollToTopAction->setShortcut(QKeySequence::Home);
}

// ============================================================================
// Работа с текстом
// ============================================================================

void ConsoleTab::appendText(const QString& text, MessageType type) {
    if (text.isEmpty()) return;
    
    // Применение фильтра
    if (!currentFilter.isEmpty() && !text.contains(currentFilter, Qt::CaseInsensitive)) {
        return;
    }
    
    // Ограничение количества строк
    limitLines();
    
    // Перемещение курсора в конец
    textEdit->moveCursor(QTextCursor::End);
    
    // Форматирование сообщения
    QTextCharFormat format = getMessageFormat(type);
    QTextCursor cursor = textEdit->textCursor();
    cursor.setCharFormat(format);
    textEdit->setTextCursor(cursor);
    
    // Вставка текста с временной меткой
    QString timestampedText = "[" + formatTimestamp() + "] " + text;
    if (!text.endsWith("\n")) {
        timestampedText += "\n";
    }
    
    textEdit->insertPlainText(timestampedText);
    
    // Обновление статистики
    messageStats[type]++;
    totalMessages++;
    modified = true;
    lastModified = QDateTime::currentMSecsSinceEpoch();
    
    // Автопрокрутка
    if (autoScroll) {
        scrollToBottom();
    }
    
    // Обновление строки состояния
    updateStatusBar();
    
    emit textChanged();
}

void ConsoleTab::appendHtml(const QString& html) {
    if (html.isEmpty()) return;
    
    limitLines();
    
    textEdit->moveCursor(QTextCursor::End);
    textEdit->insertHtml(html + "<br/>");
    
    modified = true;
    lastModified = QDateTime::currentMSecsSinceEpoch();
    
    if (autoScroll) {
        scrollToBottom();
    }
    
    updateStatusBar();
    emit textChanged();
}

void ConsoleTab::clear() {
    textEdit->clear();
    resetStatistics();
    modified = true;
    lastModified = QDateTime::currentMSecsSinceEpoch();
    updateStatusBar();
    emit textChanged();
}

void ConsoleTab::setPlaceholderText(const QString& text) {
    textEdit->setPlaceholderText(text);
}

QString ConsoleTab::getPlainText() const {
    return textEdit->toPlainText();
}

QString ConsoleTab::getHtml() const {
    return textEdit->toHtml();
}

int ConsoleTab::getLineCount() const {
    return textEdit->document()->blockCount();
}

int ConsoleTab::getCharacterCount() const {
    return textEdit->toPlainText().length();
}

// ============================================================================
// Прокрутка
// ============================================================================

void ConsoleTab::scrollToBottom() {
    QScrollBar* scrollbar = textEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void ConsoleTab::scrollToTop() {
    QScrollBar* scrollbar = textEdit->verticalScrollBar();
    scrollbar->setValue(scrollbar->minimum());
}

void ConsoleTab::scrollToLine(int line) {
    if (line < 1 || line > getLineCount()) return;
    
    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    for (int i = 1; i < line; i++) {
        cursor.movePosition(QTextCursor::Down);
    }
    textEdit->setTextCursor(cursor);
    textEdit->centerCursor();
}

bool ConsoleTab::isAtBottom() const {
    QScrollBar* scrollbar = textEdit->verticalScrollBar();
    return scrollbar->value() >= scrollbar->maximum();
}

// ============================================================================
// Конфигурация
// ============================================================================

void ConsoleTab::setMaxLines(int count) {
    maxLines = qMax(100, count);
    limitLines();
}

void ConsoleTab::setAutoScroll(bool enable) {
    autoScroll = enable;
}

void ConsoleTab::setReadOnly(bool readOnly) {
    textEdit->setReadOnly(readOnly);
    if (pasteAction) pasteAction->setEnabled(!readOnly);
}

bool ConsoleTab::isReadOnly() const {
    return textEdit->isReadOnly();
}

void ConsoleTab::setLineWrap(bool enable) {
    lineWrap = enable;
    textEdit->setLineWrapMode(enable ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
}

bool ConsoleTab::getLineWrap() const {
    return lineWrap;
}

// ============================================================================
// Фильтрация
// ============================================================================

void ConsoleTab::setFilter(const QString& filter) {
    currentFilter = filter;
    clearFilterAction->setEnabled(!filter.isEmpty());
    applyFilter();
    emit filterChanged(filter);
}

void ConsoleTab::clearFilter() {
    currentFilter.clear();
    clearFilterAction->setEnabled(false);
    applyFilter();
    emit filterChanged("");
}

void ConsoleTab::applyFilter() {
    // В полной реализации - скрытие/показ строк по фильтру
    // Для упрощения - просто перерисовка
    if (!currentFilter.isEmpty()) {
        textEdit->setStyleSheet(
            "QTextEdit { "
            "  background-color: #1e1e1e; "
            "  color: #d4d4d4; "
            "  border: 2px solid #007acc; "
            "}"
        );
    } else {
        textEdit->setStyleSheet(
            "QTextEdit { "
            "  background-color: #1e1e1e; "
            "  color: #d4d4d4; "
            "  border: none; "
            "}"
        );
    }
}

int ConsoleTab::getFilteredLineCount() const {
    if (currentFilter.isEmpty()) {
        return getLineCount();
    }
    
    int count = 0;
    QString text = getPlainText();
    QStringList lines = text.split("\n");
    for (const QString& line : lines) {
        if (line.contains(currentFilter, Qt::CaseInsensitive)) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// Форматирование
// ============================================================================

void ConsoleTab::setFont(const QFont& font) {
    defaultFont = font;
    textEdit->setFont(font);
}

QFont ConsoleTab::getFont() const {
    return textEdit->font();
}

void ConsoleTab::setFontSize(int size) {
    QFont font = defaultFont;
    font.setPointSize(size);
    setFont(font);
}

int ConsoleTab::getFontSize() const {
    return defaultFont.pointSize();
}

void ConsoleTab::setColorScheme(const QString& scheme) {
    colorScheme = scheme;
    applyColorScheme();
}

void ConsoleTab::initializeColorSchemes() {
    // Тёмная схема (по умолчанию)
    QMap<MessageType, QColor> darkScheme;
    darkScheme[MessageType::Info] = QColor(212, 212, 212);
    darkScheme[MessageType::Warning] = QColor(206, 167, 0);
    darkScheme[MessageType::Error] = QColor(244, 71, 71);
    darkScheme[MessageType::Success] = QColor(106, 153, 85);
    darkScheme[MessageType::Debug] = QColor(86, 156, 214);
    darkScheme[MessageType::Command] = QColor(197, 134, 192);
    darkScheme[MessageType::Output] = QColor(78, 201, 176);
    darkScheme[MessageType::Input] = QColor(220, 220, 220);
    darkScheme[MessageType::System] = QColor(150, 150, 150);
    colorSchemes["dark"] = darkScheme;
    
    // Светлая схема
    QMap<MessageType, QColor> lightScheme;
    lightScheme[MessageType::Info] = QColor(0, 0, 0);
    lightScheme[MessageType::Warning] = QColor(180, 140, 0);
    lightScheme[MessageType::Error] = QColor(200, 50, 50);
    lightScheme[MessageType::Success] = QColor(50, 120, 50);
    lightScheme[MessageType::Debug] = QColor(0, 100, 180);
    lightScheme[MessageType::Command] = QColor(150, 50, 150);
    lightScheme[MessageType::Output] = QColor(0, 120, 100);
    lightScheme[MessageType::Input] = QColor(50, 50, 50);
    lightScheme[MessageType::System] = QColor(100, 100, 100);
    colorSchemes["light"] = lightScheme;
}

void ConsoleTab::applyColorScheme() {
    if (!colorSchemes.contains(colorScheme)) {
        colorScheme = "dark";
    }
    
    // Применение цветовой схемы будет использоваться при выводе сообщений
}

// ============================================================================
// Выделение
// ============================================================================

void ConsoleTab::selectAll() {
    textEdit->selectAll();
}

void ConsoleTab::copy() {
    textEdit->copy();
}

void ConsoleTab::cut() {
    if (!textEdit->isReadOnly()) {
        textEdit->cut();
    }
}

void ConsoleTab::paste() {
    if (!textEdit->isReadOnly()) {
        textEdit->paste();
    }
}

bool ConsoleTab::hasSelection() const {
    return !textEdit->textCursor().selectedText().isEmpty();
}

QString ConsoleTab::getSelectedText() const {
    return textEdit->textCursor().selectedText();
}

// ============================================================================
// Поиск
// ============================================================================

int ConsoleTab::findText(const QString& text, bool caseSensitive, bool backward) {
    if (text.isEmpty()) return -1;
    
    QTextDocument::FindFlags flags;
    if (!caseSensitive) {
        flags |= QTextDocument::FindCaseInsensitively;
    }
    if (backward) {
        flags |= QTextDocument::FindBackward;
    }
    
    QTextCursor cursor = textEdit->textCursor();
    if (textEdit->find(text, flags)) {
        searchResults.clear();
        searchText = text;
        searchIndex = 0;
        
        // Поиск всех вхождений
        QTextCursor searchCursor = textEdit->document()->find(text, 0, flags);
        while (!searchCursor.isNull()) {
            searchResults.append(searchCursor.selectionStart());
            searchCursor = textEdit->document()->find(text, searchCursor, flags);
        }
        
        return searchCursor.selectionStart();
    }
    
    return -1;
}

int ConsoleTab::findTextNext(const QString& text) {
    if (text.isEmpty() || searchResults.isEmpty()) return -1;
    
    searchIndex = (searchIndex + 1) % searchResults.size();
    QTextCursor cursor = textEdit->textCursor();
    cursor.setPosition(searchResults[searchIndex]);
    cursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
    textEdit->setTextCursor(cursor);
    
    return searchResults[searchIndex];
}

void ConsoleTab::highlightText(const QString& text, const QColor& color) {
    // В полной реализации - подсветка всех вхождений текста
}

// ============================================================================
// Экспорт/Импорт
// ============================================================================

bool ConsoleTab::saveToFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << getPlainText();
    file.close();
    
    LOG_INFO("Console output saved to: " + path.toStdString());
    return true;
}

bool ConsoleTab::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    textEdit->setPlainText(in.readAll());
    file.close();
    
    modified = false;
    lastModified = QDateTime::currentMSecsSinceEpoch();
    
    LOG_INFO("Console output loaded from: " + path.toStdString());
    return true;
}

void ConsoleTab::exportToHtml(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // HTML заголовок
    out << "<!DOCTYPE html>\n";
    out << "<html>\n<head>\n";
    out << "<meta charset=\"UTF-8\">\n";
    out << "<title>Console Output</title>\n";
    out << "<style>\n";
    out << "body { font-family: Consolas, monospace; background: #1e1e1e; color: #d4d4d4; padding: 20px; }\n";
    out << ".info { color: #d4d4d4; }\n";
    out << ".warning { color: #cea700; }\n";
    out << ".error { color: #f44747; }\n";
    out << ".success { color: #6a9955; }\n";
    out << ".debug { color: #569cd6; }\n";
    out << "</style>\n";
    out << "</head>\n<body>\n";
    
    // Содержимое
    out << "<pre>\n";
    out << escapeHtml(getPlainText());
    out << "</pre>\n";
    
    out << "</body>\n</html>\n";
    file.close();
    
    LOG_INFO("Console output exported to HTML: " + path.toStdString());
}

void ConsoleTab::exportToPdf(const QString& path) {
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    
    QTextDocument* doc = textEdit->document();
    doc->print(&printer);
    
    LOG_INFO("Console output exported to PDF: " + path.toStdString());
}

// ============================================================================
// Статистика
// ============================================================================

int ConsoleTab::getMessageCount(MessageType type) const {
    return messageStats.value(type, 0);
}

QMap<MessageType, int> ConsoleTab::getMessageStatistics() const {
    return messageStats;
}

void ConsoleTab::resetStatistics() {
    messageStats.clear();
    messageStats[MessageType::Info] = 0;
    messageStats[MessageType::Warning] = 0;
    messageStats[MessageType::Error] = 0;
    messageStats[MessageType::Success] = 0;
    messageStats[MessageType::Debug] = 0;
    messageStats[MessageType::Command] = 0;
    messageStats[MessageType::Output] = 0;
    messageStats[MessageType::Input] = 0;
    messageStats[MessageType::System] = 0;
    totalMessages = 0;
}

// ============================================================================
// События
// ============================================================================

void ConsoleTab::contextMenuEvent(QContextMenuEvent* event) {
    contextMenu->exec(event->globalPos());
}

void ConsoleTab::keyPressEvent(QKeyEvent* event) {
    // Ctrl+A - выделение всего
    if (event->key() == Qt::Key_A && event->modifiers() & Qt::ControlModifier) {
        selectAll();
        event->accept();
        return;
    }
    
    // Ctrl+F - поиск
    if (event->key() == Qt::Key_F && event->modifiers() & Qt::ControlModifier) {
        onFindText();
        event->accept();
        return;
    }
    
    // Ctrl+L - очистка
    if (event->key() == Qt::Key_L && event->modifiers() & Qt::ControlModifier) {
        clear();
        event->accept();
        return;
    }
    
    // Базовая обработка
    QTextEdit::keyPressEvent(event);
}

void ConsoleTab::focusInEvent(QFocusEvent* event) {
    updateStatusBar();
    QWidget::focusInEvent(event);
}

void ConsoleTab::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateStatusBar();
}

void ConsoleTab::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
}

void ConsoleTab::onTextChanged() {
    updateStatusBar();
}

void ConsoleTab::onSelectionChanged() {
    // Обновление состояния действий
}

void ConsoleTab::onContextMenuAction() {
    // Обработка действий контекстного меню
}

void ConsoleTab::onFindText() {
    bool ok;
    QString text = QInputDialog::getText(this, "Найти", "Введите текст для поиска:",
                                         QLineEdit::Normal, searchText, &ok);
    if (ok && !text.isEmpty()) {
        findText(text);
    }
}

void ConsoleTab::onClearFilter() {
    clearFilter();
}

// ============================================================================
// Вспомогательные методы
// ============================================================================

void ConsoleTab::limitLines() {
    if (maxLines <= 0) return;
    
    while (textEdit->document()->blockCount() > maxLines) {
        QTextCursor cursor(textEdit->document());
        cursor.setPosition(0);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar(); // Удалить символ новой строки
    }
}

QColor ConsoleTab::getMessageColor(MessageType type) const {
    if (colorSchemes.contains(colorScheme)) {
        return colorSchemes[colorScheme].value(type, QColor(212, 212, 212));
    }
    return QColor(212, 212, 212);
}

QTextCharFormat ConsoleTab::getMessageFormat(MessageType type) const {
    QTextCharFormat format;
    format.setForeground(getMessageColor(type));
    format.setFont(defaultFont);
    
    // Для ошибок - дополнительный фон
    if (type == MessageType::Error) {
        format.setBackground(QColor(60, 30, 30));
    }
    
    return format;
}

QString ConsoleTab::formatTimestamp() const {
    return QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
}

int ConsoleTab::countLines(const QString& text) const {
    return text.count('\n') + 1;
}

QString ConsoleTab::escapeHtml(const QString& text) const {
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    return escaped;
}

void ConsoleTab::updateStatusBar() {
    if (!statusLabel) return;
    
    QString status = QString("Lines: %1 | Characters: %2 | Messages: %3")
        .arg(getLineCount())
        .arg(getCharacterCount())
        .arg(totalMessages);
    
    if (!currentFilter.isEmpty()) {
        status += QString(" | Filter: \"%1\"").arg(currentFilter);
    }
    
    statusLabel->setText(status);
}

} // namespace proxima