#include "SearchWidget.h"
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace proxima {

// ============================================================================
// SearchWidget Implementation
// ============================================================================

SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent)
    , textEditor(nullptr)
    , matchCount(0)
    , currentMatchIndex(-1)
    , maxHistorySize(50)
    , caseSensitive(false)
    , wholeWord(false)
    , useRegex(false) {
    
    setupUI();
    setupConnections();
    setupShortcuts();
    loadSettings();
    
    // Таймер для отложенного поиска
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300);
    connect(searchTimer, &QTimer::timeout, this, &SearchWidget::updateMatchCount);
    
    LOG_DEBUG("SearchWidget created");
}

SearchWidget::~SearchWidget() {
    saveSettings();
    clearHighlights();
    LOG_DEBUG("SearchWidget destroyed");
}

void SearchWidget::setupUI() {
    // Главная компоновка
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // Стиль виджета
    setStyleSheet(
        "QWidget { "
        "  background-color: #252526; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "}"
        "QLineEdit { "
        "  background-color: #3c3c3c; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 3px; "
        "}"
        "QLineEdit:focus { "
        "  border: 1px solid #007acc; "
        "}"
        "QCheckBox { "
        "  color: #d4d4d4; "
        "  spacing: 3px; "
        "}"
        "QPushButton { "
        "  background-color: #3c3c3c; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 3px 8px; "
        "}"
        "QPushButton:hover { "
        "  background-color: #4c4c4c; "
        "  border: 1px solid #007acc; "
        "}"
        "QToolButton { "
        "  background-color: transparent; "
        "  color: #d4d4d4; "
        "  border: none; "
        "  border-radius: 3px; "
        "  padding: 3px; "
        "}"
        "QToolButton:hover { "
        "  background-color: #3c3c3c; "
        "}"
        "QLabel { "
        "  color: #808080; "
        "}"
    );
    
    // Поле поиска
    QHBoxLayout* findLayout = new QHBoxLayout();
    findLayout->setSpacing(5);
    
    findEdit = new QLineEdit(this);
    findEdit->setPlaceholderText("Найти...");
    findEdit->setClearButtonEnabled(true);
    findEdit->setFixedHeight(24);
    findLayout->addWidget(findEdit, 1);
    
    // Кнопки навигации
    findPrevButton = new QPushButton("↑", this);
    findPrevButton->setToolTip("Найти предыдущее");
    findPrevButton->setFixedWidth(30);
    findLayout->addWidget(findPrevButton);
    
    findNextButton = new QPushButton("↓", this);
    findNextButton->setToolTip("Найти следующее");
    findNextButton->setFixedWidth(30);
    findLayout->addWidget(findNextButton);
    
    // Счётчик совпадений
    matchCountLabel = new QLabel("0 из 0", this);
    matchCountLabel->setFixedWidth(60);
    matchCountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    findLayout->addWidget(matchCountLabel);
    
    // Кнопка истории
    historyButton = new QToolButton(this);
    historyButton->setText("🕐");
    historyButton->setToolTip("История поиска");
    historyButton->setPopupMode(QToolButton::InstantPopup);
    historyButton->setFixedWidth(30);
    findLayout->addWidget(historyButton);
    
    // Меню истории
    historyMenu = new QMenu(this);
    historyButton->setMenu(historyMenu);
    
    connect(historyMenu, &QMenu::aboutToShow, this, &SearchWidget::onShowHistory);
    
    // Кнопка закрытия
    closeButton = new QToolButton(this);
    closeButton->setText("✕");
    closeButton->setToolTip("Закрыть (Esc)");
    closeButton->setFixedWidth(30);
    findLayout->addWidget(closeButton);
    
    mainLayout->addLayout(findLayout);
    
    // Опции поиска
    QHBoxLayout* optionsLayout = new QHBoxLayout();
    optionsLayout->setSpacing(10);
    
    caseSensitiveCheck = new QCheckBox("Aa", this);
    caseSensitiveCheck->setToolTip("С учётом регистра");
    optionsLayout->addWidget(caseSensitiveCheck);
    
    wholeWordCheck = new QCheckBox("␣", this);
    wholeWordCheck->setToolTip("Целые слова");
    optionsLayout->addWidget(wholeWordCheck);
    
    regexCheck = new QCheckBox(".*", this);
    regexCheck->setToolTip("Регулярные выражения");
    optionsLayout->addWidget(regexCheck);
    
    optionsLayout->addStretch();
    
    mainLayout->addLayout(optionsLayout);
    
    // Виджет замены
    replaceWidget = new QWidget(this);
    QHBoxLayout* replaceLayout = new QHBoxLayout(replaceWidget);
    replaceLayout->setContentsMargins(0, 5, 0, 0);
    replaceLayout->setSpacing(5);
    
    replaceEdit = new QLineEdit(this);
    replaceEdit->setPlaceholderText("Заменить на...");
    replaceEdit->setClearButtonEnabled(true);
    replaceEdit->setFixedHeight(24);
    replaceLayout->addWidget(replaceEdit, 1);
    
    replaceButton = new QPushButton("Заменить", this);
    replaceButton->setToolTip("Заменить текущее совпадение");
    replaceLayout->addWidget(replaceButton);
    
    replaceAllButton = new QPushButton("Все", this);
    replaceAllButton->setToolTip("Заменить все совпадения");
    replaceLayout->addWidget(replaceAllButton);
    
    QToolButton* toggleReplaceButton = new QToolButton(this);
    toggleReplaceButton->setText("⚙");
    toggleReplaceButton->setToolTip("Показать/скрыть замену");
    connect(toggleReplaceButton, &QToolButton::clicked, this, &SearchWidget::onToggleReplace);
    replaceLayout->addWidget(toggleReplaceButton);
    
    replaceWidget->setVisible(false);
    mainLayout->addWidget(replaceWidget);
}

void SearchWidget::setupConnections() {
    connect(findEdit, &QLineEdit::textChanged, this, &SearchWidget::onFindTextChanged);
    connect(replaceEdit, &QLineEdit::textChanged, this, &SearchWidget::onReplaceTextChanged);
    connect(findNextButton, &QPushButton::clicked, this, &SearchWidget::onFindNext);
    connect(findPrevButton, &QPushButton::clicked, this, &SearchWidget::onFindPrevious);
    connect(replaceButton, &QPushButton::clicked, this, &SearchWidget::onReplaceCurrent);
    connect(replaceAllButton, &QPushButton::clicked, this, &SearchWidget::onReplaceAll);
    connect(caseSensitiveCheck, &QCheckBox::toggled, this, &SearchWidget::onCaseSensitiveToggled);
    connect(wholeWordCheck, &QCheckBox::toggled, this, &SearchWidget::onWholeWordToggled);
    connect(regexCheck, &QCheckBox::toggled, this, &SearchWidget::onRegularExpressionToggled);
    connect(closeButton, &QToolButton::clicked, this, &SearchWidget::onCloseClicked);
}

void SearchWidget::setupShortcuts() {
    // F3 - Найти далее
    QShortcut* findNextShortcut = new QShortcut(QKeySequence::FindNext, this);
    connect(findNextShortcut, &QShortcut::activated, this, &SearchWidget::onFindNext);
    
    // Shift+F3 - Найти ранее
    QShortcut* findPrevShortcut = new QShortcut(QKeySequence::FindPrevious, this);
    connect(findPrevShortcut, &QShortcut::activated, this, &SearchWidget::onFindPrevious);
    
    // Ctrl+H - Показать замену
    QShortcut* replaceShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this);
    connect(replaceShortcut, &QShortcut::activated, this, [this]() {
        replaceWidget->setVisible(!replaceWidget->isVisible());
        if (replaceWidget->isVisible()) {
            replaceEdit->setFocus();
        }
    });
    
    // Ctrl+Shift+H - Заменить все
    QShortcut* replaceAllShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H), this);
    connect(replaceAllShortcut, &QShortcut::activated, this, &SearchWidget::onReplaceAll);
    
    // Esc - Закрыть
    QShortcut* closeShortcut = new QShortcut(QKeySequence::Cancel, this);
    connect(closeShortcut, &QShortcut::activated, this, &SearchWidget::onCloseClicked);
}

void SearchWidget::setEditor(QTextEdit* editor) {
    if (textEditor) {
        disconnect(textEditor, &QTextEdit::textChanged, this, &SearchWidget::updateMatchCount);
    }
    
    textEditor = editor;
    
    if (textEditor) {
        connect(textEditor, &QTextEdit::textChanged, this, &SearchWidget::updateMatchCount);
    }
}

void SearchWidget::findText(const QString& text) {
    if (text.isEmpty()) {
        clearHighlights();
        matchCount = 0;
        currentMatchIndex = -1;
        updateMatchCount();
        return;
    }
    
    findEdit->setText(text);
    addToHistory(text);
    
    emit searchStarted(text);
    
    // Отложенный поиск для производительности
    searchTimer->start();
}

void SearchWidget::findNext() {
    if (!textEditor || matchCount == 0) return;
    
    currentMatchIndex = (currentMatchIndex + 1) % matchCount;
    
    if (currentMatchIndex >= 0 && currentMatchIndex < matchPositions.size()) {
        int pos = matchPositions[currentMatchIndex];
        
        QTextCursor cursor = textEditor->textCursor();
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 
                           findEdit->text().length());
        textEditor->setTextCursor(cursor);
        textEditor->ensureCursorVisible();
        
        emit matchFound(currentMatchIndex + 1, matchCount);
    }
}

void SearchWidget::findPrevious() {
    if (!textEditor || matchCount == 0) return;
    
    currentMatchIndex--;
    if (currentMatchIndex < 0) {
        currentMatchIndex = matchCount - 1;
    }
    
    if (currentMatchIndex >= 0 && currentMatchIndex < matchPositions.size()) {
        int pos = matchPositions[currentMatchIndex];
        
        QTextCursor cursor = textEditor->textCursor();
        cursor.setPosition(pos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 
                           findEdit->text().length());
        textEditor->setTextCursor(cursor);
        textEditor->ensureCursorVisible();
        
        emit matchFound(currentMatchIndex + 1, matchCount);
    }
}

void SearchWidget::replaceCurrent() {
    if (!textEditor || currentMatchIndex < 0) return;
    
    QTextCursor cursor = textEditor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText(replaceEdit->text());
        updateMatchCount();
        findNext();
        
        emit replacePerformed(1);
    }
}

void SearchWidget::replaceAll() {
    if (!textEditor || matchCount == 0) return;
    
    int replaceCount = 0;
    QString searchText = findEdit->text();
    QString replaceText = replaceEdit->text();
    
    QTextCursor cursor = textEditor->textCursor();
    cursor.beginEditBlock();
    
    // Поиск с конца, чтобы позиции не смещались
    for (int i = matchPositions.size() - 1; i >= 0; i--) {
        int pos = matchPositions[i];
        cursor.setPosition(pos);
        cursor.setPosition(pos + searchText.length(), QTextCursor::KeepAnchor);
        cursor.insertText(replaceText);
        replaceCount++;
    }
    
    cursor.endEditBlock();
    
    clearHighlights();
    updateMatchCount();
    
    QMessageBox::information(this, "Завершено", 
                            QString("Заменено %1 совпадений.").arg(replaceCount));
    
    emit replacePerformed(replaceCount);
}

void SearchWidget::setCaseSensitive(bool sensitive) {
    caseSensitiveCheck->setChecked(sensitive);
}

void SearchWidget::setWholeWord(bool whole) {
    wholeWordCheck->setChecked(whole);
}

void SearchWidget::setRegularExpression(bool regex) {
    regexCheck->setChecked(regex);
}

void SearchWidget::setSearchText(const QString& text) {
    findEdit->setText(text);
}

void SearchWidget::setReplaceText(const QString& text) {
    replaceEdit->setText(text);
}

void SearchWidget::showFind() {
    show();
    findEdit->setFocus();
    findEdit->selectAll();
    emit visibilityChanged(true);
}

void SearchWidget::showReplace() {
    show();
    replaceWidget->setVisible(true);
    replaceEdit->setFocus();
    replaceEdit->selectAll();
    emit visibilityChanged(true);
}

void SearchWidget::hideWidget() {
    hide();
    clearHighlights();
    emit visibilityChanged(false);
}

void SearchWidget::addToHistory(const QString& text) {
    if (text.isEmpty()) return;
    
    // Удаляем дубликаты
    searchHistory.removeAll(text);
    
    // Добавляем в начало
    searchHistory.prepend(text);
    
    // Ограничиваем размер
    while (searchHistory.size() > maxHistorySize) {
        searchHistory.removeLast();
    }
    
    saveSettings();
}

void SearchWidget::clearHistory() {
    searchHistory.clear();
    historyMenu->clear();
    saveSettings();
}

void SearchWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (event->modifiers() & Qt::ShiftModifier) {
            findPrevious();
        } else {
            findNext();
        }
        event->accept();
        return;
    }
    
    if (event->key() == Qt::Key_Escape) {
        hideWidget();
        event->accept();
        return;
    }
    
    QWidget::keyPressEvent(event);
}

void SearchWidget::showEvent(QShowEvent* event) {
    findEdit->setFocus();
    findEdit->selectAll();
    updateMatchCount();
    QWidget::showEvent(event);
}

void SearchWidget::hideEvent(QHideEvent* event) {
    clearHighlights();
    QWidget::hideEvent(event);
}

void SearchWidget::onFindTextChanged(const QString& text) {
    searchTimer->start();
}

void SearchWidget::onReplaceTextChanged(const QString& text) {
    // Обновление при изменении текста замены
}

void SearchWidget::onFindNext() {
    findNext();
}

void SearchWidget::onFindPrevious() {
    findPrevious();
}

void SearchWidget::onReplaceCurrent() {
    replaceCurrent();
}

void SearchWidget::onReplaceAll() {
    replaceAll();
}

void SearchWidget::onCaseSensitiveToggled(bool checked) {
    caseSensitive = checked;
    updateMatchCount();
    saveSettings();
}

void SearchWidget::onWholeWordToggled(bool checked) {
    wholeWord = checked;
    updateMatchCount();
    saveSettings();
}

void SearchWidget::onRegularExpressionToggled(bool checked) {
    useRegex = checked;
    updateMatchCount();
    saveSettings();
}

void SearchWidget::onHistorySelected(const QString& text) {
    findEdit->setText(text);
    findEdit->setFocus();
    findEdit->selectAll();
}

void SearchWidget::onShowHistory() {
    historyMenu->clear();
    
    if (searchHistory.isEmpty()) {
        historyMenu->addAction("История пуста");
        historyMenu->actions().last()->setEnabled(false);
    } else {
        for (const QString& item : searchHistory) {
            QAction* action = historyMenu->addAction(item);
            connect(action, &QAction::triggered, this, [this, item]() {
                onHistorySelected(item);
            });
        }
        
        historyMenu->addSeparator();
        historyMenu->addAction("Очистить историю", this, &SearchWidget::onClearHistory);
    }
}

void SearchWidget::onClearHistory() {
    clearHistory();
}

void SearchWidget::onCloseClicked() {
    hideWidget();
}

void SearchWidget::onToggleReplace() {
    replaceWidget->setVisible(!replaceWidget->isVisible());
    if (replaceWidget->isVisible()) {
        replaceEdit->setFocus();
    }
}

void SearchWidget::updateMatchCount() {
    if (!textEditor) {
        matchCount = 0;
        matchCountLabel->setText("0 из 0");
        return;
    }
    
    matchCount = countMatches();
    matchCountLabel->setText(QString("%1 из %2")
        .arg(currentMatchIndex + 1, 1)
        .arg(matchCount, 1));
    
    highlightMatches();
    
    if (matchCount == 0 && !findEdit->text().isEmpty()) {
        emit noMatchFound();
    } else {
        emit searchFinished(matchCount);
    }
}

void SearchWidget::highlightMatches() {
    clearHighlights();
    
    if (!textEditor || matchCount == 0) return;
    
    matchSelections = createMatchSelections();
    
    if (!matchSelections.isEmpty()) {
        textEditor->setExtraSelections(matchSelections);
    }
}

void SearchWidget::clearHighlights() {
    if (textEditor) {
        textEditor->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    }
    matchSelections.clear();
}

void SearchWidget::loadSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    searchHistory = settings.value("search/history", QStringList()).toStringList();
    caseSensitive = settings.value("search/caseSensitive", false).toBool();
    wholeWord = settings.value("search/wholeWord", false).toBool();
    useRegex = settings.value("search/regex", false).toBool();
    
    caseSensitiveCheck->setChecked(caseSensitive);
    wholeWordCheck->setChecked(wholeWord);
    regexCheck->setChecked(useRegex);
}

void SearchWidget::saveSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    settings.setValue("search/history", searchHistory);
    settings.setValue("search/caseSensitive", caseSensitive);
    settings.setValue("search/wholeWord", wholeWord);
    settings.setValue("search/regex", useRegex);
}

QString SearchWidget::getSettingsPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + 
           "/search_settings.ini";
}

QString SearchWidget::escapeRegExp(const QString& text) const {
    return QRegularExpression::escape(text);
}

QRegularExpression SearchWidget::createRegExp() const {
    QString pattern = findEdit->text();
    
    if (pattern.isEmpty()) {
        return QRegularExpression();
    }
    
    if (!useRegex) {
        pattern = escapeRegExp(pattern);
    }
    
    if (wholeWord) {
        pattern = "\\b" + pattern + "\\b";
    }
    
    QRegularExpression re(pattern);
    
    if (!caseSensitive) {
        re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    
    return re;
}

QVector<QTextEdit::ExtraSelection> SearchWidget::createMatchSelections() const {
    QVector<QTextEdit::ExtraSelection> selections;
    
    if (!textEditor || findEdit->text().isEmpty()) {
        return selections;
    }
    
    QRegularExpression re = createRegExp();
    if (!re.isValid()) {
        return selections;
    }
    
    QTextDocument* doc = textEditor->document();
    QString text = doc->toPlainText();
    
    matchPositions.clear();
    
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        
        QTextEdit::ExtraSelection selection;
        selection.cursor = textEditor->textCursor();
        selection.cursor.setPosition(match.capturedStart());
        selection.cursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);
        
        // Цвет выделения
        selection.format.setBackground(QColor(0, 122, 204, 100));  // Синий полупрозрачный
        selection.format.setForeground(QColor(255, 255, 255));
        
        selections.append(selection);
        matchPositions.append(match.capturedStart());
    }
    
    return selections;
}

int SearchWidget::countMatches() const {
    if (!textEditor || findEdit->text().isEmpty()) {
        return 0;
    }
    
    QRegularExpression re = createRegExp();
    if (!re.isValid()) {
        return 0;
    }
    
    QTextDocument* doc = textEditor->document();
    QString text = doc->toPlainText();
    
    int count = 0;
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        it.next();
        count++;
    }
    
    return count;
}

// ============================================================================
// ProjectSearchWidget Implementation
// ============================================================================

ProjectSearchWidget::ProjectSearchWidget(QWidget *parent)
    : QWidget(parent)
    , projectPath("")
    , isSearchRunning(false)
    , searchCancelled(false)
    , searchThread(nullptr)
    , searchWorker(nullptr) {
    
    setupUI();
    setupConnections();
    setupOptions();
    loadSettings();
}

ProjectSearchWidget::~ProjectSearchWidget() {
    if (isSearchRunning) {
        stopSearch();
    }
    
    saveSettings();
    
    if (searchThread) {
        delete searchThread;
    }
    if (searchWorker) {
        delete searchWorker;
    }
}

void ProjectSearchWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Поле поиска
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(5);
    
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Поиск по проекту...");
    searchEdit->setClearButtonEnabled(true);
    searchLayout->addWidget(searchEdit, 1);
    
    searchButton = new QPushButton("🔍 Поиск", this);
    searchButton->setFixedWidth(100);
    searchLayout->addWidget(searchButton);
    
    stopButton = new QPushButton("⏹ Стоп", this);
    stopButton->setFixedWidth(100);
    stopButton->setEnabled(false);
    searchLayout->addWidget(stopButton);
    
    mainLayout->addLayout(searchLayout);
    
    // Опции
    QGroupBox* optionsGroup = new QGroupBox("Опции", this);
    QHBoxLayout* optionsLayout = new QHBoxLayout(optionsGroup);
    optionsLayout->setSpacing(15);
    
    caseSensitiveCheck = new QCheckBox("С учётом регистра", this);
    optionsLayout->addWidget(caseSensitiveCheck);
    
    wholeWordCheck = new QCheckBox("Целые слова", this);
    optionsLayout->addWidget(wholeWordCheck);
    
    regexCheck = new QCheckBox("Регулярные выражения", this);
    optionsLayout->addWidget(regexCheck);
    
    includeHiddenCheck = new QCheckBox("Включая скрытые", this);
    optionsLayout->addWidget(includeHiddenCheck);
    
    groupByFileCheck = new QCheckBox("Группировать по файлам", this);
    groupByFileCheck->setChecked(true);
    optionsLayout->addWidget(groupByFileCheck);
    
    optionsLayout->addStretch();
    
    mainLayout->addWidget(optionsGroup);
    
    // Фильтр файлов
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Фильтр файлов:", this));
    
    fileFilterCombo = new QComboBox(this);
    fileFilterCombo->addItem("Все файлы (*.*)", "*");
    fileFilterCombo->addItem("Proxima (*.prx)", "*.prx");
    fileFilterCombo->addItem("Заголовки (*.h, *.hpp)", "*.h *.hpp");
    fileFilterCombo->addItem("Исходники (*.cpp, *.c)", "*.cpp *.c");
    fileFilterCombo->addItem("Текст (*.txt, *.md)", "*.txt *.md");
    fileFilterCombo->setEditable(true);
    filterLayout->addWidget(fileFilterCombo, 1);
    
    mainLayout->addLayout(filterLayout);
    
    // Фильтр результатов
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Фильтр результатов...");
    filterEdit->setClearButtonEnabled(true);
    mainLayout->addWidget(filterEdit);
    
    // Список результатов
    resultsList = new QListWidget(this);
    resultsList->setAlternatingRowColors(true);
    resultsList->setStyleSheet(
        "QListWidget { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "}"
        "QListWidget::item { "
        "  padding: 5px; "
        "  border-bottom: 1px solid #2d2d2d; "
        "}"
        "QListWidget::item:selected { "
        "  background-color: #094771; "
        "  color: #ffffff; "
        "}"
        "QListWidget::item:hover { "
        "  background-color: #2a2d2e; "
        "}"
    );
    mainLayout->addWidget(resultsList, 1);
    
    // Статус и прогресс
    QHBoxLayout* statusLayout = new QHBoxLayout();
    
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setVisible(false);
    progressBar->setFixedHeight(20);
    statusLayout->addWidget(progressBar, 1);
    
    statusLabel = new QLabel("Готов к поиску", this);
    statusLabel->setStyleSheet("color: #808080;");
    statusLayout->addWidget(statusLabel);
    
    mainLayout->addLayout(statusLayout);
    
    // Кнопки действий
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->addStretch();
    
    exportButton = new QPushButton("Экспорт", this);
    actionLayout->addWidget(exportButton);
    
    clearButton = new QPushButton("Очистить", this);
    actionLayout->addWidget(clearButton);
    
    mainLayout->addLayout(actionLayout);
}

void ProjectSearchWidget::setupConnections() {
    connect(searchEdit, &QLineEdit::returnPressed, this, &ProjectSearchWidget::onSearchButtonClicked);
    connect(searchButton, &QPushButton::clicked, this, &ProjectSearchWidget::onSearchButtonClicked);
    connect(stopButton, &QPushButton::clicked, this, &ProjectSearchWidget::onStopButtonClicked);
    connect(resultsList, &QListWidget::itemClicked, this, &ProjectSearchWidget::onResultItemClicked);
    connect(resultsList, &QListWidget::itemDoubleClicked, this, &ProjectSearchWidget::onResultItemDoubleClicked);
    connect(filterEdit, &QLineEdit::textChanged, this, &ProjectSearchWidget::onFilterTextChanged);
    connect(groupByFileCheck, &QCheckBox::toggled, this, &ProjectSearchWidget::onGroupByFileToggled);
    connect(exportButton, &QPushButton::clicked, this, &ProjectSearchWidget::onExportResults);
    connect(clearButton, &QPushButton::clicked, this, &ProjectSearchWidget::onClearResults);
}

void ProjectSearchWidget::setupOptions() {
    // Настройка опций поиска
}

void ProjectSearchWidget::setProjectPath(const QString& path) {
    projectPath = path;
}

void ProjectSearchWidget::setSearchText(const QString& text) {
    searchEdit->setText(text);
}

void ProjectSearchWidget::startSearch() {
    if (projectPath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Project path not set");
        return;
    }
    
    QString searchText = searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Search text is empty");
        return;
    }
    
    if (isSearchRunning) {
        stopSearch();
    }
    
    isSearchRunning = true;
    searchCancelled = false;
    searchResults.clear();
    resultsList->clear();
    
    searchButton->setEnabled(false);
    stopButton->setEnabled(true);
    progressBar->setVisible(true);
    statusLabel->setText("Поиск...");
    
    emit searchStarted(searchText, projectPath);
    
    // Создание worker для поиска в отдельном потоке
    searchThread = new QThread();
    searchWorker = new SearchWorker();
    searchWorker->moveToThread(searchThread);
    
    // Настройка worker
    searchWorker->setSearchText(searchText);
    searchWorker->setProjectPath(projectPath);
    searchWorker->setCaseSensitive(caseSensitiveCheck->isChecked());
    searchWorker->setWholeWord(wholeWordCheck->isChecked());
    searchWorker->setRegularExpression(regexCheck->isChecked());
    searchWorker->setFileFilter(fileFilterCombo->currentData().toString());
    searchWorker->setIncludeHidden(includeHiddenCheck->isChecked());
    
    // Соединения
    connect(searchThread, &QThread::started, searchWorker, &SearchWorker::startSearch);
    connect(searchWorker, &SearchWorker::progress, this, &ProjectSearchWidget::onSearchProgress);
    connect(searchWorker, &SearchWorker::finished, this, &ProjectSearchWidget::onSearchFinished);
    connect(searchWorker, &SearchWorker::error, this, &ProjectSearchWidget::onSearchError);
    connect(this, &ProjectSearchWidget::searchCancelled, searchWorker, &SearchWorker::cancel);
    
    searchThread->start();
}

void ProjectSearchWidget::stopSearch() {
    if (!isSearchRunning) return;
    
    searchCancelled = true;
    emit searchCancelled();
    
    if (searchThread) {
        searchThread->quit();
        searchThread->wait(3000);
        delete searchThread;
        searchThread = nullptr;
    }
    
    if (searchWorker) {
        delete searchWorker;
        searchWorker = nullptr;
    }
    
    isSearchRunning = false;
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);
    statusLabel->setText("Поиск остановлен");
    
    emit searchCancelled();
}

void ProjectSearchWidget::clearResults() {
    searchResults.clear();
    resultsList->clear();
    statusLabel->setText("Результаты очищены");
}

void ProjectSearchWidget::filterResults(const QString& filter) {
    // Фильтрация отображаемых результатов
}

void ProjectSearchWidget::groupByFile(bool group) {
    displayResults(searchResults);
}

void ProjectSearchWidget::exportResults(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Cannot create file");
        return;
    }
    
    QTextStream out(&file);
    
    out << "Search Results\n";
    out << "==============\n\n";
    out << "Search text: " << searchEdit->text() << "\n";
    out << "Project path: " << projectPath << "\n";
    out << "Total matches: " << searchResults.size() << "\n\n";
    
    QMap<QString, QVector<SearchResult>> grouped;
    for (const SearchResult& result : searchResults) {
        grouped[result.filePath].append(result);
    }
    
    for (auto it = grouped.begin(); it != grouped.end(); ++it) {
        out << "File: " << it.key() << "\n";
        out << "Matches: " << it.value().size() << "\n\n";
        
        for (const SearchResult& result : it.value()) {
            out << "  Line " << result.lineNumber << ": " << result.lineText.trimmed() << "\n";
        }
        out << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Export", "Results exported to " + path);
}

void ProjectSearchWidget::onSearchTextChanged(const QString& text) {
    // Обновление при изменении текста поиска
}

void ProjectSearchWidget::onSearchButtonClicked() {
    startSearch();
}

void ProjectSearchWidget::onStopButtonClicked() {
    stopSearch();
}

void ProjectSearchWidget::onResultItemClicked(QListWidgetItem* item) {
    // Выделение результата
}

void ProjectSearchWidget::onResultItemDoubleClicked(QListWidgetItem* item) {
    // Открытие файла в редакторе
    SearchResult result = item->data(Qt::UserRole).value<SearchResult>();
    emit resultSelected(result);
}

void ProjectSearchWidget::onSearchProgress(int filesSearched, int totalFiles, int matchesFound) {
    statusLabel->setText(QString("Поиск: %1/%2 файлов, %3 совпадений")
        .arg(filesSearched)
        .arg(totalFiles)
        .arg(matchesFound));
}

void ProjectSearchWidget::onSearchFinished(const QVector<SearchResult>& results) {
    isSearchRunning = false;
    searchResults = results;
    
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);
    
    displayResults(results);
    
    statusLabel->setText(QString("Найдено %1 совпадений в %2 файлах")
        .arg(results.size())
        .arg(results.size()));  // Упрощённо
    
    emit searchFinished(results.size(), results.size());
    
    // Очистка worker
    if (searchThread) {
        searchThread->quit();
        searchThread->wait();
        delete searchThread;
        searchThread = nullptr;
    }
    
    if (searchWorker) {
        delete searchWorker;
        searchWorker = nullptr;
    }
}

void ProjectSearchWidget::onSearchError(const QString& error) {
    isSearchRunning = false;
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);
    statusLabel->setText("Ошибка: " + error);
    
    emit errorOccurred(error);
}

void ProjectSearchWidget::onExportResults() {
    QString path = QFileDialog::getSaveFileName(
        this, "Export Results", "", 
        "Text Files (*.txt);;All Files (*)");
    
    if (!path.isEmpty()) {
        exportResults(path);
    }
}

void ProjectSearchWidget::onClearResults() {
    clearResults();
}

void ProjectSearchWidget::onFilterTextChanged(const QString& text) {
    filterResults(text);
}

void ProjectSearchWidget::onGroupByFileToggled(bool checked) {
    displayResults(searchResults);
}

void ProjectSearchWidget::displayResults(const QVector<SearchResult>& results) {
    resultsList->clear();
    
    if (groupByFileCheck->isChecked()) {
        // Группировка по файлам
        QMap<QString, QVector<SearchResult>> grouped;
        for (const SearchResult& result : results) {
            grouped[result.filePath].append(result);
        }
        
        for (auto it = grouped.begin(); it != grouped.end(); ++it) {
            // Заголовок файла
            QListWidgetItem* fileItem = new QListWidgetItem("📄 " + it.key());
            fileItem->setForeground(QColor(78, 201, 176));
            fileItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
            resultsList->addItem(fileItem);
            
            // Результаты в файле
            for (const SearchResult& result : it.value()) {
                QListWidgetItem* item = new QListWidgetItem(
                    QString("  %1: %2").arg(result.lineNumber).arg(result.lineText.left(80)));
                item->setData(Qt::UserRole, QVariant::fromValue(result));
                item->setForeground(QColor(212, 212, 212));
                resultsList->addItem(item);
            }
        }
    } else {
        // Плоский список
        for (const SearchResult& result : results) {
            QListWidgetItem* item = new QListWidgetItem(
                QString("%1 (%2): %3")
                    .arg(result.filePath)
                    .arg(result.lineNumber)
                    .arg(result.lineText.left(60)));
            item->setData(Qt::UserRole, QVariant::fromValue(result));
            resultsList->addItem(item);
        }
    }
}

void ProjectSearchWidget::loadSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    caseSensitiveCheck->setChecked(settings.value("projectSearch/caseSensitive", false).toBool());
    wholeWordCheck->setChecked(settings.value("projectSearch/wholeWord", false).toBool());
    regexCheck->setChecked(settings.value("projectSearch/regex", false).toBool());
    includeHiddenCheck->setChecked(settings.value("projectSearch/includeHidden", false).toBool());
    groupByFileCheck->setChecked(settings.value("projectSearch/groupByFile", true).toBool());
}

void ProjectSearchWidget::saveSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    settings.setValue("projectSearch/caseSensitive", caseSensitiveCheck->isChecked());
    settings.setValue("projectSearch/wholeWord", wholeWordCheck->isChecked());
    settings.setValue("projectSearch/regex", regexCheck->isChecked());
    settings.setValue("projectSearch/includeHidden", includeHiddenCheck->isChecked());
    settings.setValue("projectSearch/groupByFile", groupByFileCheck->isChecked());
}

QString ProjectSearchWidget::getSettingsPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + 
           "/project_search_settings.ini";
}

// ============================================================================
// SearchWorker Implementation
// ============================================================================

SearchWorker::SearchWorker(QObject *parent)
    : QObject(parent)
    , caseSensitive(false)
    , wholeWord(false)
    , useRegex(false)
    , includeHidden(false)
    , cancelled(false)
    , filesSearched(0)
    , totalFiles(0)
    , matchesFound(0) {
}

void SearchWorker::setSearchText(const QString& text) {
    searchText = text;
}

void SearchWorker::setProjectPath(const QString& path) {
    projectPath = path;
}

void SearchWorker::setCaseSensitive(bool sensitive) {
    caseSensitive = sensitive;
}

void SearchWorker::setWholeWord(bool whole) {
    wholeWord = whole;
}

void SearchWorker::setRegularExpression(bool regex) {
    useRegex = regex;
}

void SearchWorker::setFileFilter(const QString& filter) {
    fileFilter = filter;
}

void SearchWorker::setIncludeHidden(bool include) {
    includeHidden = include;
}

void SearchWorker::cancel() {
    cancelled = true;
}

void SearchWorker::startSearch() {
    cancelled = false;
    results.clear();
    filesSearched = 0;
    matchesFound = 0;
    
    searchDirectory(projectPath);
    
    emit finished(results);
}

void SearchWorker::searchDirectory(const QString& dirPath) {
    if (cancelled) return;
    
    QDir dir(dirPath);
    if (!dir.exists()) return;
    
    // Получение файлов
    QStringList files = getFilesToSearch(dirPath);
    totalFiles += files.size();
    
    // Поиск в файлах
    for (const QString& file : files) {
        if (cancelled) break;
        
        searchFile(file);
        filesSearched++;
        
        emit progress(filesSearched, totalFiles, matchesFound);
    }
    
    // Рекурсивный поиск в поддиректориях
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& subdir : subdirs) {
        if (cancelled) break;
        
        if (includeHidden || !subdir.fileName().startsWith(".")) {
            searchDirectory(subdir.absoluteFilePath());
        }
    }
}

void SearchWorker::searchFile(const QString& filePath) {
    if (cancelled) return;
    
    if (shouldSkipFile(filePath)) return;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();
    
    QVector<SearchResult> fileMatches = findMatches(filePath, content);
    
    if (!fileMatches.isEmpty()) {
        results.append(fileMatches);
        matchesFound += fileMatches.size();
    }
}

QVector<SearchResult> SearchWorker::findMatches(const QString& filePath, const QString& content) {
    QVector<SearchResult> matches;
    
    QRegularExpression re;
    if (useRegex) {
        re.setPattern(searchText);
    } else {
        re.setPattern(QRegularExpression::escape(searchText));
    }
    
    if (wholeWord) {
        re.setPattern("\\b" + re.pattern() + "\\b");
    }
    
    if (!caseSensitive) {
        re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    
    if (!re.isValid()) return matches;
    
    QStringList lines = content.split("\n");
    
    for (int i = 0; i < lines.size(); i++) {
        if (cancelled) break;
        
        QString line = lines[i];
        QRegularExpressionMatchIterator it = re.globalMatch(line);
        
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            
            SearchResult result;
            result.filePath = filePath;
            result.lineNumber = i + 1;
            result.columnNumber = match.capturedStart() + 1;
            result.matchLength = match.capturedLength();
            result.lineText = line;
            result.matchedText = match.captured();
            
            matches.append(result);
        }
    }
    
    return matches;
}

QStringList SearchWorker::getFilesToSearch(const QString& dirPath) {
    QStringList files;
    
    QDir dir(dirPath);
    
    // Фильтр файлов
    QStringList filters;
    if (fileFilter == "*" || fileFilter.isEmpty()) {
        filters << "*";
    } else {
        filters = fileFilter.split(" ", Qt::SkipEmptyParts);
    }
    
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files);
    
    if (!includeHidden) {
        dir.setFilter(dir.filter() | QDir::NoHidden);
    }
    
    QFileInfoList fileInfoList = dir.entryInfoList();
    
    for (const QFileInfo& fileInfo : fileInfoList) {
        files.append(fileInfo.absoluteFilePath());
    }
    
    return files;
}

bool SearchWorker::shouldSkipFile(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    
    // Пропуск бинарных файлов
    QStringList binaryExtensions = {".bin", ".exe", ".dll", ".so", ".dylib", 
                                    ".pdf", ".jpg", ".png", ".gif", ".ico"};
    
    for (const QString& ext : binaryExtensions) {
        if (fileInfo.suffix().toLower() == ext.mid(1)) {
            return true;
        }
    }
    
    return false;
}

} // namespace proxima