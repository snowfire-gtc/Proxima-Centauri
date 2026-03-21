#include "REPLConsoleTab.h"
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QToolTip>
#include <QCompleter>
#include <QAbstractItemView>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

REPLConsoleTab::REPLConsoleTab(QWidget *parent)
    : QWidget(parent)
    , repl(nullptr)
    , outputDisplay(nullptr)
    , inputLine(nullptr)
    , promptLabel(nullptr)
    , inputContainer(nullptr)
    , syntaxHighlighter(nullptr)
    , prompt(">> ")
    , promptColor(197, 134, 192)  // #C586C0
    , historyIndex(-1)
    , historyEnabled(true)
    , maxHistorySize(1000)
    , autoCompletionEnabled(true)
    , completionPopup(nullptr)
    , completionList(nullptr)
    , selectedCompletionIndex(-1)
    , autoScroll(true)
    , syntaxHighlightingEnabled(true)
    , busy(false)
    , contextMenu(nullptr)
    , commandCount(0)
    , outputLineCount(0) {
    
    // Настройка шрифта консоли
    consoleFont = QFont("Consolas", 10);
    consoleFont.setStyleHint(QFont::Monospace);
    consoleFont.setFixedPitch(true);
    
    // Настройка цветов сообщений
    messageColors[MessageType::Info] = QColor(212, 212, 212);      // #D4D4D4
    messageColors[MessageType::Warning] = QColor(206, 167, 0);     // #CEA700
    messageColors[MessageType::Error] = QColor(244, 71, 71);       // #F44747
    messageColors[MessageType::Success] = QColor(106, 153, 85);    // #6A9955
    messageColors[MessageType::Debug] = QColor(86, 156, 214);      // #569CD6
    messageColors[MessageType::Command] = QColor(197, 134, 192);   // #C586C0
    messageColors[MessageType::Output] = QColor(78, 201, 176);     // #4EC9B0
    
    setupUI();
    setupConnections();
    setupContextMenu();
    setupCompletionPopup();
    
    LOG_INFO("REPLConsoleTab created");
}

REPLConsoleTab::~REPLConsoleTab() {
    if (repl && historyEnabled) {
        saveHistory(QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation) + "/repl_history.prx");
    }
    
    LOG_INFO("REPLConsoleTab destroyed");
}

// ============================================================================
// Настройка UI
// ============================================================================

void REPLConsoleTab::setupUI() {
    // Главная компоновка
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    
    // Область вывода
    outputDisplay = new QTextEdit(this);
    outputDisplay->setReadOnly(true);
    outputDisplay->setLineWrapMode(QTextEdit::NoWrap);
    outputDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    outputDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    outputDisplay->setFont(consoleFont);
    outputDisplay->setAcceptRichText(false);
    outputDisplay->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Стиль области вывода
    outputDisplay->setStyleSheet(
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
    );
    
    mainLayout->addWidget(outputDisplay, 1);
    
    // Контейнер для ввода
    inputContainer = new QWidget(this);
    QHBoxLayout* inputLayout = new QHBoxLayout(inputContainer);
    inputLayout->setContentsMargins(5, 5, 5, 5);
    inputLayout->setSpacing(5);
    
    // Метка prompt
    promptLabel = new QLabel(prompt, this);
    promptLabel->setFont(consoleFont);
    promptLabel->setStyleSheet(
        "QLabel { "
        "  color: #c586c0; "
        "  font-weight: bold; "
        "  padding: 2px; "
        "}"
    );
    promptLabel->setFixedWidth(30);
    inputLayout->addWidget(promptLabel);
    
    // Поле ввода (многострочное)
    inputLine = new QPlainTextEdit(this);
    inputLine->setFont(consoleFont);
    inputLine->setMaximumBlockCount(1);  // Одна строка
    inputLine->setLineWrapMode(QPlainTextEdit::NoWrap);
    inputLine->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    inputLine->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    inputLine->setAcceptRichText(false);
    inputLine->setContextMenuPolicy(Qt::CustomContextMenu);
    inputLine->setPlaceholderText("Введите команду Proxima или 'help' для справки...");
    
    // Стиль поля ввода
    inputLine->setStyleSheet(
        "QPlainTextEdit { "
        "  background-color: #2d2d2d; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 3px; "
        "  selection-background-color: #264f78; "
        "  selection-color: #ffffff; "
        "}"
        "QPlainTextEdit:focus { "
        "  border: 1px solid #007acc; "
        "}"
        "QPlainTextEdit:hover { "
        "  border: 1px solid #5a5a5a; "
        "}"
    );
    
    inputLayout->addWidget(inputLine, 1);
    
    mainLayout->addWidget(inputContainer);
    
    // Подсветка синтаксиса
    if (syntaxHighlightingEnabled) {
        syntaxHighlighter = new REPLSyntaxHighlighter(inputLine->document());
        syntaxHighlighter->setPrompt(prompt);
    }
    
    // Установка фокуса на поле ввода
    inputLine->setFocus();
}

void REPLConsoleTab::setupConnections() {
    // Ввод команды
    connect(inputLine, &QPlainTextEdit::blockCountChanged, this, [this]() {
        // Ограничение одной строкой
        if (inputLine->blockCount() > 1) {
            QTextCursor cursor = inputLine->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.select(QTextCursor::BlockUnderCursor);
            cursor.removeSelectedText();
        }
    });
    
    // Обработка Enter
    connect(inputLine, &QPlainTextEdit::blockCountChanged, this, [this]() {
        // Проверка на нажатие Enter обрабатывается в keyPressEvent
    });
    
    // Изменение текста
    connect(inputLine, &QPlainTextEdit::textChanged, this, 
            &REPLConsoleTab::onInputTextChanged);
    
    // Контекстное меню для outputDisplay
    connect(outputDisplay, &QTextEdit::customContextMenuRequested, this,
            [this](const QPoint& pos) {
        contextMenu->exec(outputDisplay->mapToGlobal(pos));
    });
    
    // Контекстное меню для inputLine
    connect(inputLine, &QPlainTextEdit::customContextMenuRequested, this,
            [this](const QPoint& pos) {
        contextMenu->exec(inputLine->mapToGlobal(pos));
    });
}

void REPLConsoleTab::setupContextMenu() {
    contextMenu = new QMenu(this);
    
    clearAction = contextMenu->addAction("Очистить консоль", this, 
                                          &REPLConsoleTab::onClearTriggered);
    clearAction->setShortcut(QKeySequence::Clear);
    
    saveAction = contextMenu->addAction("Сохранить историю...", this,
                                         &REPLConsoleTab::onSaveHistoryTriggered);
    saveAction->setShortcut(QKeySequence::Save);
    
    loadAction = contextMenu->addAction("Загрузить историю...", this,
                                         &REPLConsoleTab::onLoadHistoryTriggered);
    loadAction->setShortcut(QKeySequence::Open);
    
    contextMenu->addSeparator();
    
    copyAction = contextMenu->addAction("Копировать", this,
                                         &REPLConsoleTab::onCopyTriggered);
    copyAction->setShortcut(QKeySequence::Copy);
    
    pasteAction = contextMenu->addAction("Вставить", this,
                                          &REPLConsoleTab::onPasteTriggered);
    pasteAction->setShortcut(QKeySequence::Paste);
    
    selectAllAction = contextMenu->addAction("Выделить всё", this,
                                              &REPLConsoleTab::onSelectAllTriggered);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    
    contextMenu->addSeparator();
    
    historyAction = contextMenu->addAction("Очистить историю", this,
                                            &REPLConsoleTab::clearHistory);
}

void REPLConsoleTab::setupCompletionPopup() {
    if (!autoCompletionEnabled) return;
    
    completionPopup = new QPopupWidget(this);
    completionPopup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    
    completionList = new QListWidget(completionPopup);
    completionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    completionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    completionList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    completionList->setSelectionBehavior(QAbstractItemView::SelectRows);
    completionList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Стиль popup
    completionList->setStyleSheet(
        "QListWidget { "
        "  background-color: #252526; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  outline: none; "
        "}"
        "QListWidget::item { "
        "  padding: 4px 8px; "
        "  border-bottom: 1px solid #3e3e3e; "
        "}"
        "QListWidget::item:selected { "
        "  background-color: #094771; "
        "  color: #ffffff; "
        "}"
        "QListWidget::item:hover { "
        "  background-color: #2a2d2e; "
        "}"
    );
    
    QVBoxLayout* popupLayout = new QVBoxLayout(completionPopup);
    popupLayout->setContentsMargins(0, 0, 0, 0);
    popupLayout->addWidget(completionList);
    
    completionPopup->setLayout(popupLayout);
    
    // Соединения
    connect(completionList, &QListWidget::itemClicked, this,
            [this](QListWidgetItem* item) {
        onCompletionItemSelected(item->text());
    });
    
    connect(completionList, &QListWidget::itemActivated, this,
            [this](QListWidgetItem* item) {
        onCompletionItemSelected(item->text());
    });
}

// ============================================================================
// Инициализация
// ============================================================================

void REPLConsoleTab::initialize(REPL* r) {
    repl = r;
    
    if (repl) {
        // Подключение к сигналам REPL
        connect(repl, &REPL::outputReceived, this, &REPLConsoleTab::onOutputReceived);
        connect(repl, &REPL::errorReceived, this, &REPLConsoleTab::onErrorReceived);
        connect(repl, &REPL::promptDisplayed, this, &REPLConsoleTab::onPromptDisplayed);
        
        // Загрузка истории
        QString historyPath = QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation) + "/repl_history.prx";
        loadHistory(historyPath);
        
        // Приветственное сообщение
        appendOutput("Proxima REPL v1.0.0\n");
        appendOutput("Type 'help' for available commands.\n");
        appendOutput("Type 'exit' to quit.\n\n");
        
        // Отображение prompt
        appendPrompt();
        
        emit initializationComplete();
        
        LOG_INFO("REPLConsoleTab initialized");
    }
}

void REPLConsoleTab::setPrompt(const QString& p) {
    prompt = p;
    promptLabel->setText(prompt);
    
    if (syntaxHighlighter) {
        syntaxHighlighter->setPrompt(p);
    }
}

// ============================================================================
// Выполнение команд
// ============================================================================

void REPLConsoleTab::executeCommand(const QString& command) {
    if (!repl || busy) return;
    
    busy = true;
    inputLine->setEnabled(false);
    
    // Отображение ввода
    appendInput(prompt + command);
    
    // Добавление в историю
    if (historyEnabled && !command.trimmed().isEmpty()) {
        if (commandHistory.isEmpty() || commandHistory.last() != command) {
            commandHistory.append(command);
            
            // Ограничение размера истории
            while (commandHistory.size() > maxHistorySize) {
                commandHistory.removeFirst();
            }
            
            historyIndex = -1;
            emit historyChanged();
        }
    }
    
    // Выполнение команды
    QString result = repl->execute(command);
    
    // Отображение результата
    if (!result.isEmpty()) {
        if (result.contains("Error", Qt::CaseInsensitive)) {
            appendError(result);
        } else if (result.contains("Warning", Qt::CaseInsensitive)) {
            appendWarning(result);
        } else {
            appendOutput(result);
        }
    }
    
    // Новый prompt
    appendPrompt();
    
    busy = false;
    inputLine->setEnabled(true);
    inputLine->setFocus();
    
    emit commandExecuted(command, result);
    commandCount++;
}

void REPLConsoleTab::executeCurrentInput() {
    QString input = getCurrentInput().trimmed();
    if (!input.isEmpty()) {
        executeCommand(input);
        inputLine->clear();
    }
}

void REPLConsoleTab::onInputSubmitted() {
    executeCurrentInput();
}

void REPLConsoleTab::onInputTextChanged(const QString& text) {
    if (autoCompletionEnabled && !text.isEmpty()) {
        updateCompletionPopup();
    } else {
        hideCompletionPopup();
    }
}

// ============================================================================
// Обработка ввода с клавиатуры
// ============================================================================

void REPLConsoleTab::keyPressEvent(QKeyEvent* event) {
    // Обработка автодополнения
    if (completionPopup && completionPopup->isVisible()) {
        if (event->key() == Qt::Key_Up) {
            int currentRow = completionList->currentRow();
            if (currentRow > 0) {
                completionList->setCurrentRow(currentRow - 1);
            }
            event->accept();
            return;
        }
        
        if (event->key() == Qt::Key_Down) {
            int currentRow = completionList->currentRow();
            if (currentRow < completionList->count() - 1) {
                completionList->setCurrentRow(currentRow + 1);
            }
            event->accept();
            return;
        }
        
        if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Return) {
            if (completionList->currentRow() >= 0) {
                QListWidgetItem* item = completionList->currentItem();
                if (item) {
                    onCompletionItemSelected(item->text());
                }
            }
            event->accept();
            return;
        }
        
        if (event->key() == Qt::Key_Escape) {
            hideCompletionPopup();
            event->accept();
            return;
        }
    }
    
    // Обработка Enter
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            executeCurrentInput();
            event->accept();
            return;
        }
    }
    
    // Навигация по истории
    if (event->key() == Qt::Key_Up) {
        if (inputLine->textCursor().position() == 0 || 
            inputLine->toPlainText().isEmpty()) {
            navigateHistory(1);
            event->accept();
            return;
        }
    }
    
    if (event->key() == Qt::Key_Down) {
        if (inputLine->textCursor().position() == inputLine->toPlainText().length() ||
            inputLine->toPlainText().isEmpty()) {
            navigateHistory(-1);
            event->accept();
            return;
        }
    }
    
    // Ctrl+C - прерывание
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        appendOutput("\n^C\n");
        appendPrompt();
        event->accept();
        return;
    }
    
    // Ctrl+L - очистка консоли
    if (event->key() == Qt::Key_L && event->modifiers() & Qt::ControlModifier) {
        clear();
        appendPrompt();
        event->accept();
        return;
    }
    
    // Ctrl+A - выделение всего в inputLine
    if (event->key() == Qt::Key_A && event->modifiers() & Qt::ControlModifier) {
        inputLine->selectAll();
        event->accept();
        return;
    }
    
    // Tab - автодополнение
    if (event->key() == Qt::Key_Tab) {
        if (autoCompletionEnabled) {
            showCompletionPopup();
            event->accept();
            return;
        }
    }
    
    // Базовая обработка
    QPlainTextEdit::keyPressEvent(event);
}

void REPLConsoleTab::focusInEvent(QFocusEvent* event) {
    inputLine->setFocus();
    QWidget::focusInEvent(event);
}

void REPLConsoleTab::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // Пересчёт позиции popup автодополнения
    if (completionPopup && completionPopup->isVisible()) {
        updateCompletionPopup();
    }
}

void REPLConsoleTab::contextMenuEvent(QContextMenuEvent* event) {
    contextMenu->exec(event->globalPos());
}

// ============================================================================
// История команд
// ============================================================================

void REPLConsoleTab::navigateHistory(int direction) {
    if (!historyEnabled || commandHistory.isEmpty()) return;
    
    if (direction > 0) {
        // Вверх по истории (предыдущие команды)
        if (historyIndex < commandHistory.size() - 1) {
            if (historyIndex == -1) {
                savedInput = getCurrentInput();
            }
            historyIndex++;
            setCurrentInput(commandHistory[historyIndex]);
        }
    } else {
        // Вниз по истории (следующие команды)
        if (historyIndex > 0) {
            historyIndex--;
            setCurrentInput(commandHistory[historyIndex]);
        } else if (historyIndex == 0) {
            historyIndex = -1;
            setCurrentInput(savedInput);
        }
    }
    
    // Курсор в конец строки
    QTextCursor cursor = inputLine->textCursor();
    cursor.movePosition(QTextCursor::End);
    inputLine->setTextCursor(cursor);
}

void REPLConsoleTab::clearHistory() {
    commandHistory.clear();
    historyIndex = -1;
    appendOutput("История команд очищена.\n");
    emit historyChanged();
}

void REPLConsoleTab::saveHistory(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        appendError("Ошибка: Не удалось сохранить историю.\n");
        return;
    }
    
    QTextStream out(&file);
    for (const QString& cmd : commandHistory) {
        out << cmd << "\n";
    }
    
    file.close();
    appendOutput(QString("История сохранена в %1 (%2 команд).\n")
        .arg(path).arg(commandHistory.size()));
}

void REPLConsoleTab::loadHistory(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // Файл может не существовать при первом запуске
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.trimmed().isEmpty()) {
            commandHistory.append(line);
        }
    }
    
    file.close();
    
    if (!commandHistory.isEmpty()) {
        appendOutput(QString("Загружено %1 команд из истории.\n")
            .arg(commandHistory.size()));
    }
}

// ============================================================================
// Автодополнение
// ============================================================================

void REPLConsoleTab::setAutoCompletionEnabled(bool enable) {
    autoCompletionEnabled = enable;
    if (!enable) {
        hideCompletionPopup();
    }
}

void REPLConsoleTab::showCompletionPopup() {
    if (!autoCompletionEnabled || !repl) return;
    
    QString prefix = getWordUnderCursor();
    if (prefix.isEmpty()) {
        hideCompletionPopup();
        return;
    }
    
    currentCompletions = getCompletions(prefix);
    
    if (currentCompletions.isEmpty()) {
        hideCompletionPopup();
        return;
    }
    
    // Заполнение списка
    completionList->clear();
    for (const QString& completion : currentCompletions) {
        QListWidgetItem* item = new QListWidgetItem(completion);
        
        // Иконка в зависимости от типа
        if (completion.startsWith("!")) {
            item->setIcon(QIcon(":/icons/command.svg"));
        } else if (repl->hasVariable(completion)) {
            item->setIcon(QIcon(":/icons/variable.svg"));
        } else {
            item->setIcon(QIcon(":/icons/keyword.svg"));
        }
        
        completionList->addItem(item);
    }
    
    completionList->setCurrentRow(0);
    selectedCompletionIndex = 0;
    
    // Позиционирование popup
    updateCompletionPopup();
    completionPopup->show();
}

void REPLConsoleTab::hideCompletionPopup() {
    if (completionPopup) {
        completionPopup->hide();
    }
    selectedCompletionIndex = -1;
    currentCompletions.clear();
}

void REPLConsoleTab::updateCompletionPopup() {
    if (!completionPopup || !completionPopup->isVisible()) return;
    
    // Позиционирование рядом с курсором
    QTextCursor cursor = inputLine->textCursor();
    QRect cursorRect = inputLine->cursorRect(cursor);
    QPoint pos = inputLine->mapToGlobal(cursorRect.bottomLeft());
    
    completionPopup->move(pos);
    
    // Автоподстановка при одном варианте
    if (currentCompletions.size() == 1) {
        insertCompletion(currentCompletions.first());
        hideCompletionPopup();
    }
}

void REPLConsoleTab::onCompletionItemSelected(const QString& item) {
    insertCompletion(item);
    hideCompletionPopup();
    inputLine->setFocus();
}

QStringList REPLConsoleTab::getCompletions(const QString& prefix) const {
    QStringList completions;
    
    if (!repl) return completions;
    
    // Автодополнение команд REPL
    if (prefix.startsWith("!")) {
        QStringList commands = {"!help", "!whois", "!whos", "!show", "!type", 
                               "!size", "!clear", "!history", "!load", "!save",
                               "!exit", "!version", "!config", "!reset"};
        for (const QString& cmd : commands) {
            if (cmd.startsWith(prefix, Qt::CaseInsensitive)) {
                completions.append(cmd);
            }
        }
    } else {
        // Автодополнение переменных
        QMap<QString, REPLVariable> variables = repl->getVariables();
        for (auto it = variables.begin(); it != variables.end(); ++it) {
            if (it.key().startsWith(prefix, Qt::CaseInsensitive)) {
                completions.append(it.key());
            }
        }
        
        // Автодополнение ключевых слов Proxima
        QStringList keywords = {"if", "else", "elseif", "end", "for", "in", 
                               "while", "do", "return", "break", "continue",
                               "class", "interface", "function", "vector", 
                               "matrix", "collection", "time"};
        for (const QString& kw : keywords) {
            if (kw.startsWith(prefix, Qt::CaseInsensitive)) {
                completions.append(kw);
            }
        }
    }
    
    completions.removeDuplicates();
    completions.sort();
    
    return completions;
}

QString REPLConsoleTab::getWordUnderCursor() const {
    QTextCursor cursor = inputLine->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}

void REPLConsoleTab::insertCompletion(const QString& completion) {
    QTextCursor cursor = inputLine->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.insertText(completion);
    inputLine->setTextCursor(cursor);
}

// ============================================================================
// Вывод сообщений
// ============================================================================

void REPLConsoleTab::appendPrompt() {
    outputDisplay->moveCursor(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(promptColor);
    format.setFont(consoleFont);
    format.setFontWeight(QFont::Bold);
    
    QTextCursor cursor = outputDisplay->textCursor();
    cursor.setCharFormat(format);
    outputDisplay->setTextCursor(cursor);
    outputDisplay->append(prompt);
    
    scrollToBottom();
    
    emit promptDisplayed();
}

void REPLConsoleTab::appendInput(const QString& input) {
    outputDisplay->moveCursor(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(messageColors[MessageType::Input]);
    format.setFont(consoleFont);
    
    QTextCursor cursor = outputDisplay->textCursor();
    cursor.setCharFormat(format);
    outputDisplay->setTextCursor(cursor);
    outputDisplay->append(input);
    
    scrollToBottom();
    outputLineCount++;
}

void REPLConsoleTab::appendOutput(const QString& output) {
    formatOutput(output, MessageType::Output);
}

void REPLConsoleTab::appendError(const QString& error) {
    formatOutput(error, MessageType::Error);
}

void REPLConsoleTab::appendWarning(const QString& warning) {
    formatOutput(warning, MessageType::Warning);
}

void REPLConsoleTab::appendDebug(const QString& debug) {
    formatOutput(debug, MessageType::Debug);
}

void REPLConsoleTab::formatOutput(const QString& text, MessageType type) {
    outputDisplay->moveCursor(QTextCursor::End);
    
    QTextCharFormat format;
    format.setForeground(getMessageColor(type));
    format.setFont(consoleFont);
    
    // Для ошибок - дополнительный фон
    if (type == MessageType::Error) {
        format.setBackground(QColor(60, 30, 30));
    }
    
    QTextCursor cursor = outputDisplay->textCursor();
    cursor.setCharFormat(format);
    outputDisplay->setTextCursor(cursor);
    
    // Разбиение на строки
    QStringList lines = text.split("\n");
    for (const QString& line : lines) {
        if (!line.isEmpty()) {
            outputDisplay->append(line);
            outputLineCount++;
        }
    }
    
    scrollToBottom();
}

QColor REPLConsoleTab::getMessageColor(MessageType type) const {
    return messageColors.value(type, messageColors[MessageType::Info]);
}

void REPLConsoleTab::scrollToBottom() {
    if (autoScroll) {
        QScrollBar* scrollbar = outputDisplay->verticalScrollBar();
        scrollbar->setValue(scrollbar->maximum());
    }
}

// ============================================================================
// Конфигурация
// ============================================================================

void REPLConsoleTab::setMaxHistorySize(int size) {
    maxHistorySize = qMax(10, size);
    
    while (commandHistory.size() > maxHistorySize) {
        commandHistory.removeFirst();
    }
}

void REPLConsoleTab::setAutoScroll(bool enable) {
    autoScroll = enable;
}

void REPLConsoleTab::setSyntaxHighlightingEnabled(bool enable) {
    syntaxHighlightingEnabled = enable;
    if (syntaxHighlighter) {
        syntaxHighlighter->setDocument(enable ? inputLine->document() : nullptr);
    }
}

// ============================================================================
// Очистка
// ============================================================================

void REPLConsoleTab::clear() {
    outputDisplay->clear();
    outputLineCount = 0;
    appendPrompt();
}

void REPLConsoleTab::clearOutput() {
    outputDisplay->clear();
    outputLineCount = 0;
}

// ============================================================================
// Слоты контекстного меню
// ============================================================================

void REPLConsoleTab::onClearTriggered() {
    clear();
}

void REPLConsoleTab::onSaveHistoryTriggered() {
    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить историю REPL", "", 
        "Text Files (*.txt);;All Files (*)");
    if (!path.isEmpty()) {
        saveHistory(path);
    }
}

void REPLConsoleTab::onLoadHistoryTriggered() {
    QString path = QFileDialog::getOpenFileName(
        this, "Загрузить историю REPL", "", 
        "Text Files (*.txt);;All Files (*)");
    if (!path.isEmpty()) {
        loadHistory(path);
    }
}

void REPLConsoleTab::onCopyTriggered() {
    if (outputDisplay->hasFocus()) {
        outputDisplay->copy();
    } else if (inputLine->hasFocus()) {
        inputLine->copy();
    }
}

void REPLConsoleTab::onPasteTriggered() {
    if (inputLine->hasFocus()) {
        inputLine->paste();
    }
}

void REPLConsoleTab::onSelectAllTriggered() {
    if (outputDisplay->hasFocus()) {
        outputDisplay->selectAll();
    } else if (inputLine->hasFocus()) {
        inputLine->selectAll();
    }
}

// ============================================================================
// Вспомогательные методы
// ============================================================================

QString REPLConsoleTab::getCurrentInput() const {
    return inputLine->toPlainText();
}

void REPLConsoleTab::setCurrentInput(const QString& input) {
    inputLine->setPlainText(input);
    QTextCursor cursor = inputLine->textCursor();
    cursor.movePosition(QTextCursor::End);
    inputLine->setTextCursor(cursor);
}

int REPLConsoleTab::getInputCursorPosition() const {
    return inputLine->textCursor().position();
}

void REPLConsoleTab::setInputCursorPosition(int pos) {
    QTextCursor cursor = inputLine->textCursor();
    cursor.setPosition(pos);
    inputLine->setTextCursor(cursor);
}

void REPLConsoleTab::setFocusToInput() {
    inputLine->setFocus();
}

// ============================================================================
// Обработка сигналов REPL
// ============================================================================

void REPLConsoleTab::onOutputReceived(const QString& output) {
    appendOutput(output);
}

void REPLConsoleTab::onErrorReceived(const QString& error) {
    appendError(error);
}

void REPLConsoleTab::onPromptDisplayed() {
    appendPrompt();
    inputLine->setFocus();
}

} // namespace proxima