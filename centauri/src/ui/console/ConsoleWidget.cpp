#include "ConsoleWidget.h"
#include <QDateTime>
#include <QTextCursor>
#include <QTextBlock>
#include <QClipboard>
#include <QApplication>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

namespace proxima {

// ============================================================================
// ConsoleTab Implementation
// ============================================================================

ConsoleTab::ConsoleTab(ConsoleTabType type, QWidget *parent)
    : QWidget(parent)
    , tabType(type)
    , maxLines(10000)
    , currentFilter("") {
    
    setupUI();
}

ConsoleTab::~ConsoleTab() {
}

void ConsoleTab::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Моноширинный шрифт для консоли
    QFont consoleFont("Consolas", 10);
    consoleFont.setStyleHint(QFont::Monospace);
    textEdit->setFont(consoleFont);
    
    // Цветовая схема
    textEdit->setStyleSheet(
        "QTextEdit { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  selection-background-color: #264f78; "
        "}"
    );
    
    layout->addWidget(textEdit);
}

void ConsoleTab::appendText(const QString& text, MessageType type) {
    if (!textEdit) return;
    
    // Применение фильтра
    if (!currentFilter.isEmpty() && !text.contains(currentFilter, Qt::CaseInsensitive)) {
        return;
    }
    
    // Ограничение количества строк
    if (maxLines > 0 && textEdit->document()->blockCount() > maxLines) {
        QTextCursor cursor(textEdit->document());
        cursor.setPosition(0);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar(); // Удалить символ новой строки
    }
    
    // Перемещение курсора в конец
    textEdit->moveCursor(QTextCursor::End);
    
    // Форматирование сообщения
    QTextCharFormat format;
    format.setForeground(getMessageColor(type));
    
    // Вставка текста с форматированием
    QTextCursor cursor = textEdit->textCursor();
    cursor.setCharFormat(format);
    textEdit->setTextCursor(cursor);
    textEdit->append(text);
    
    // Автопрокрутка
    if (autoScroll) {
        textEdit->verticalScrollBar()->setValue(
            textEdit->verticalScrollBar()->maximum()
        );
    }
}

void ConsoleTab::clear() {
    if (textEdit) {
        textEdit->clear();
    }
}

void ConsoleTab::setPlaceholderText(const QString& text) {
    if (textEdit) {
        textEdit->setPlaceholderText(text);
    }
}

QString ConsoleTab::getPlainText() const {
    return textEdit ? textEdit->toPlainText() : "";
}

int ConsoleTab::getLineCount() const {
    return textEdit ? textEdit->document()->blockCount() : 0;
}

void ConsoleTab::scrollToBottom() {
    if (textEdit) {
        textEdit->verticalScrollBar()->setValue(
            textEdit->verticalScrollBar()->maximum()
        );
    }
}

void ConsoleTab::setMaxLines(int count) {
    maxLines = count;
}

void ConsoleTab::setFilter(const QString& filter) {
    currentFilter = filter;
    applyFilter();
}

void ConsoleTab::applyFilter() {
    // В полной реализации - фильтрация видимых строк
    // Для упрощения - просто перерисовка
    if (textEdit) {
        textEdit->update();
    }
}

QColor ConsoleTab::getMessageColor(MessageType type) const {
    switch (type) {
        case MessageType::Info:
            return QColor("#d4d4d4");  // Светло-серый
        case MessageType::Warning:
            return QColor("#cca700");  // Жёлтый
        case MessageType::Error:
            return QColor("#f44747");  // Красный
        case MessageType::Success:
            return QColor("#6a9955");  // Зелёный
        case MessageType::Debug:
            return QColor("#569cd6");  // Синий
        case MessageType::Command:
            return QColor("#c586c0");  // Фиолетовый
        case MessageType::Output:
            return QColor("#4ec9b0");  // Бирюзовый
        default:
            return QColor("#d4d4d4");
    }
}

// ============================================================================
// REPLConsoleTab Implementation
// ============================================================================

REPLConsoleTab::REPLConsoleTab(QWidget *parent)
    : ConsoleTab(ConsoleTabType::REPL, parent)
    , repl(nullptr)
    , inputLine(nullptr)
    , prompt(">> ")
    , historyIndex(-1)
    , historyEnabled(true) {
    
    setupUI();
    setupConnections();
}

REPLConsoleTab::~REPLConsoleTab() {
    if (repl) {
        saveHistory(QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation) + "/repl_history.prx");
    }
}

void REPLConsoleTab::setupUI() {
    // Основная компоновка
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Текстовая область вывода
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Моноширинный шрифт
    QFont consoleFont("Consolas", 10);
    consoleFont.setStyleHint(QFont::Monospace);
    textEdit->setFont(consoleFont);
    
    // Цветовая схема REPL
    textEdit->setStyleSheet(
        "QTextEdit { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  selection-background-color: #264f78; "
        "}"
    );
    
    mainLayout->addWidget(textEdit, 1);
    
    // Строка ввода
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(5, 5, 5, 5);
    inputLayout->setSpacing(5);
    
    // Метка prompt
    QLabel* promptLabel = new QLabel(prompt, this);
    promptLabel->setFont(consoleFont);
    promptLabel->setStyleSheet("color: #c586c0; font-weight: bold;");
    inputLayout->addWidget(promptLabel);
    
    // Поле ввода
    inputLine = new QLineEdit(this);
    inputLine->setFont(consoleFont);
    inputLine->setStyleSheet(
        "QLineEdit { "
        "  background-color: #2d2d2d; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  padding: 3px; "
        "}"
        "QLineEdit:focus { "
        "  border: 1px solid #007acc; "
        "}"
    );
    inputLine->setPlaceholderText("Введите команду Proxima или 'help' для справки...");
    inputLayout->addWidget(inputLine, 1);
    
    mainLayout->addLayout(inputLayout);
    
    // Контекстное меню
    setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Контекстное меню для inputLine
    inputLine->setContextMenuPolicy(Qt::CustomContextMenu);
}

void REPLConsoleTab::setupConnections() {
    // Ввод команды
    connect(inputLine, &QLineEdit::returnPressed, this, [this]() {
        QString input = inputLine->text().trimmed();
        if (!input.isEmpty()) {
            onInputSubmitted(input);
            inputLine->clear();
            historyIndex = -1;
            currentInput.clear();
        }
    });
    
    // Навигация по истории
    connect(inputLine, &QLineEdit::textEdited, this, [this](const QString& text) {
        currentInput = text;
        if (historyIndex >= 0) {
            historyIndex = -1;
        }
    });
    
    // Контекстное меню
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu(this);
        
        menu.addAction("Очистить консоль", this, &REPLConsoleTab::clear);
        menu.addAction("Сохранить историю", this, [this]() {
            QString path = QFileDialog::getSaveFileName(
                this, "Сохранить историю REPL", "", "Text Files (*.txt);;All Files (*)");
            if (!path.isEmpty()) {
                saveHistory(path);
            }
        });
        menu.addAction("Загрузить историю", this, [this]() {
            QString path = QFileDialog::getOpenFileName(
                this, "Загрузить историю REPL", "", "Text Files (*.txt);;All Files (*)");
            if (!path.isEmpty()) {
                loadHistory(path);
            }
        });
        menu.addSeparator();
        menu.addAction("Очистить историю", this, &REPLConsoleTab::clearHistory);
        menu.addSeparator();
        menu.addAction("Копировать", this, [this]() {
            textEdit->copy();
        });
        menu.addAction("Вставить", this, [this]() {
            inputLine->paste();
        });
        
        menu.exec(mapToGlobal(pos));
    });
}

void REPLConsoleTab::initialize(REPL* r) {
    repl = r;
    
    if (repl) {
        setupConnections();
        
        // Загрузка истории
        QString historyPath = QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation) + "/repl_history.prx";
        loadHistory(historyPath);
        
        // Приветственное сообщение
        appendOutput("Proxima REPL v1.0.0\n");
        appendOutput("Type 'help' for available commands.\n");
        appendOutput("Type 'exit' to quit.\n\n");
        
        displayPrompt();
    }
}

void REPLConsoleTab::setPrompt(const QString& p) {
    prompt = p;
    if (repl) {
        repl->setPrompt(p);
    }
}

QString REPLConsoleTab::getPrompt() const {
    return prompt;
}

void REPLConsoleTab::executeCommand(const QString& command) {
    if (repl) {
        QString result = repl->execute(command);
        if (!result.isEmpty()) {
            appendOutput(result);
        }
        displayPrompt();
    }
}

void REPLConsoleTab::setHistoryEnabled(bool enable) {
    historyEnabled = enable;
}

bool REPLConsoleTab::isHistoryEnabled() const {
    return historyEnabled;
}

void REPLConsoleTab::clearHistory() {
    commandHistory.clear();
    historyIndex = -1;
    appendOutput("История команд очищена.\n");
}

int REPLConsoleTab::getHistorySize() const {
    return commandHistory.size();
}

void REPLConsoleTab::saveHistory(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        appendOutput("Ошибка: Не удалось сохранить историю.\n");
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

void REPLConsoleTab::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Up) {
        navigateHistory(1);
        event->accept();
        return;
    }
    
    if (event->key() == Qt::Key_Down) {
        navigateHistory(-1);
        event->accept();
        return;
    }
    
    if (event->key() == Qt::Key_Tab) {
        // Автодополнение
        if (repl) {
            QString prefix = inputLine->text();
            QStringList completions = repl->getCompletions(prefix);
            if (!completions.isEmpty()) {
                // В полной реализации - показ popup с вариантами
                inputLine->setText(completions.first());
                inputLine->setCursorPosition(inputLine->text().length());
            }
        }
        event->accept();
        return;
    }
    
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        // Ctrl+C - прерывание
        appendOutput("\n^C\n");
        displayPrompt();
        event->accept();
        return;
    }
    
    if (event->key() == Qt::Key_L && event->modifiers() & Qt::ControlModifier) {
        // Ctrl+L - очистка консоли
        clear();
        displayPrompt();
        event->accept();
        return;
    }
    
    ConsoleTab::keyPressEvent(event);
}

void REPLConsoleTab::onInputSubmitted(const QString& input) {
    if (!repl) return;
    
    // Отображение ввода
    appendInput(prompt + input);
    
    // Добавление в историю
    if (historyEnabled && !input.trimmed().isEmpty()) {
        if (commandHistory.isEmpty() || commandHistory.last() != input) {
            commandHistory.append(input);
            
            // Ограничение размера истории
            while (commandHistory.size() > 1000) {
                commandHistory.removeFirst();
            }
        }
    }
    
    // Выполнение команды
    QString result = repl->execute(input);
    
    // Отображение результата
    if (!result.isEmpty()) {
        appendOutput(result);
    }
    
    // Новый prompt
    displayPrompt();
    
    emit replCommandExecuted(input, result);
}

void REPLConsoleTab::onOutputReceived(const QString& output) {
    appendOutput(output);
}

void REPLConsoleTab::onErrorReceived(const QString& error) {
    appendText(error, MessageType::Error);
}

void REPLConsoleTab::onPromptDisplayed() {
    displayPrompt();
    inputLine->setFocus();
}

void REPLConsoleTab::displayPrompt() {
    textEdit->moveCursor(QTextCursor::End);
    textEdit->append(prompt);
    inputLine->setFocus();
}

void REPLConsoleTab::appendInput(const QString& input) {
    QTextCharFormat format;
    format.setForeground(QColor("#d4d4d4"));
    
    textEdit->moveCursor(QTextCursor::End);
    QTextCursor cursor = textEdit->textCursor();
    cursor.setCharFormat(format);
    textEdit->setTextCursor(cursor);
    textEdit->append(input);
}

void REPLConsoleTab::appendOutput(const QString& output) {
    QTextCharFormat format;
    format.setForeground(QColor("#4ec9b0"));
    
    textEdit->moveCursor(QTextCursor::End);
    QTextCursor cursor = textEdit->textCursor();
    cursor.setCharFormat(format);
    textEdit->setTextCursor(cursor);
    textEdit->append(output);
}

void REPLConsoleTab::navigateHistory(int direction) {
    if (!historyEnabled || commandHistory.isEmpty()) return;
    
    if (direction > 0) {
        // Вверх по истории
        if (historyIndex < commandHistory.size() - 1) {
            historyIndex++;
            inputLine->setText(commandHistory[historyIndex]);
            inputLine->setCursorPosition(inputLine->text().length());
        }
    } else {
        // Вниз по истории
        if (historyIndex > 0) {
            historyIndex--;
            inputLine->setText(commandHistory[historyIndex]);
            inputLine->setCursorPosition(inputLine->text().length());
        } else if (historyIndex == 0) {
            historyIndex = -1;
            inputLine->setText(currentInput);
            inputLine->setCursorPosition(inputLine->text().length());
        }
    }
}

QString REPLConsoleTab::getCurrentInput() const {
    return inputLine ? inputLine->text() : "";
}

void REPLConsoleTab::setCurrentInput(const QString& input) {
    if (inputLine) {
        inputLine->setText(input);
        inputLine->setCursorPosition(inputLine->text().length());
    }
}

// ============================================================================
// ConsoleWidget Implementation
// ============================================================================

ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QWidget(parent)
    , replTab(nullptr)
    , repl(nullptr)
    , replEnabled(true)
    , verboseLevel(2)
    , autoScroll(true)
    , maxLines(10000)
    , currentTabType(ConsoleTabType::Build) {
    
    setupUI();
    setupConnections();
    createTabs();
}

ConsoleWidget::~ConsoleWidget() {
}

void ConsoleWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Tab widget
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(false);
    tabWidget->setTabsClosable(false);
    
    // Стиль табов
    tabWidget->setStyleSheet(
        "QTabWidget::pane { "
        "  border: 1px solid #3e3e3e; "
        "  border-top: none; "
        "} "
        "QTabBar::tab { "
        "  background-color: #2d2d2d; "
        "  color: #d4d4d4; "
        "  padding: 5px 15px; "
        "  border: 1px solid #3e3e3e; "
        "  border-bottom: none; "
        "} "
        "QTabBar::tab:selected { "
        "  background-color: #1e1e1e; "
        "  border-top: 2px solid #007acc; "
        "} "
        "QTabBar::tab:hover { "
        "  background-color: #3e3e3e; "
        "}"
    );
    
    mainLayout->addWidget(tabWidget);
    
    // Контекстное меню
    contextMenu = new QMenu(this);
    clearAction = contextMenu->addAction("Clear");
    saveAction = contextMenu->addAction("Save...");
    copyAction = contextMenu->addAction("Copy");
    contextMenu->addSeparator();
    filterAction = contextMenu->addAction("Filter...");
    contextMenu->addSeparator();
    showREPLAction = contextMenu->addAction("Show REPL Tab");
    hideREPLAction = contextMenu->addAction("Hide REPL Tab");
    
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void ConsoleWidget::setupConnections() {
    connect(tabWidget, &QTabWidget::currentChanged, this, &ConsoleWidget::onTabChanged);
    connect(this, &QWidget::customContextMenuRequested, this, &ConsoleWidget::onContextMenu);
    
    connect(clearAction, &QAction::triggered, this, &ConsoleWidget::onClearTab);
    connect(saveAction, &QAction::triggered, this, &ConsoleWidget::onSaveTab);
    connect(copyAction, &QAction::triggered, this, &ConsoleWidget::onCopyTab);
    connect(filterAction, &QAction::triggered, this, [this]() {
        bool ok;
        QString filter = QInputDialog::getText(this, "Filter", "Enter filter:",
                                              QLineEdit::Normal, "", &ok);
        if (ok) {
            onFilterChanged(filter);
        }
    });
    
    connect(showREPLAction, &QAction::triggered, this, [this]() {
        if (replTab) {
            showTab(ConsoleTabType::REPL);
        }
    });
    
    connect(hideREPLAction, &QAction::triggered, this, [this]() {
        hideTab(ConsoleTabType::REPL);
    });
}

void ConsoleWidget::createTabs() {
    // Вкладка сборки
    ConsoleTab* buildTab = new ConsoleTab(ConsoleTabType::Build, this);
    buildTab->setPlaceholderText("Build process output...");
    tabs[ConsoleTabType::Build] = buildTab;
    tabWidget->addTab(buildTab, QIcon(":/icons/build.svg"), "Build");
    
    // Вкладка ошибок
    ConsoleTab* errorsTab = new ConsoleTab(ConsoleTabType::Errors, this);
    errorsTab->setPlaceholderText("Errors will appear here...");
    tabs[ConsoleTabType::Errors] = errorsTab;
    tabWidget->addTab(errorsTab, QIcon(":/icons/error.svg"), "Errors");
    
    // Вкладка вывода
    ConsoleTab* outputTab = new ConsoleTab(ConsoleTabType::Output, this);
    outputTab->setPlaceholderText("Program output...");
    tabs[ConsoleTabType::Output] = outputTab;
    tabWidget->addTab(outputTab, QIcon(":/icons/output.svg"), "Output");
    
    // Вкладка Call Stack
    ConsoleTab* callStackTab = new ConsoleTab(ConsoleTabType::CallStack, this);
    callStackTab->setPlaceholderText("Call stack (available during debug)...");
    tabs[ConsoleTabType::CallStack] = callStackTab;
    tabWidget->addTab(callStackTab, QIcon(":/icons/stack.svg"), "Call Stack");
    
    // Вкладка REPL
    replTab = new REPLConsoleTab(this);
    replTab->setPlaceholderText("Proxima REPL - Type 'help' for commands...");
    tabs[ConsoleTabType::REPL] = replTab;
    tabWidget->addTab(replTab, QIcon(":/icons/repl.svg"), "REPL");
    
    // Обновление иконок
    updateTabIcons();
}

void ConsoleWidget::updateTabIcons() {
    // В полной реализации - загрузка иконок из ресурсов
    // Для примера используем стандартные
}

ConsoleTab* ConsoleWidget::getTab(ConsoleTabType type) const {
    return tabs.value(type, nullptr);
}

void ConsoleWidget::showTab(ConsoleTabType type) {
    ConsoleTab* tab = tabs.value(type);
    if (tab) {
        int index = tabWidget->indexOf(tab);
        if (index >= 0) {
            tabWidget->tabBar()->show();
            tabWidget->setCurrentIndex(index);
        }
    }
}

void ConsoleWidget::hideTab(ConsoleTabType type) {
    ConsoleTab* tab = tabs.value(type);
    if (tab) {
        int index = tabWidget->indexOf(tab);
        if (index >= 0) {
            tabWidget->tabBar()->hideTab(index);
        }
    }
}

bool ConsoleWidget::isTabVisible(ConsoleTabType type) const {
    ConsoleTab* tab = tabs.value(type);
    if (tab) {
        int index = tabWidget->indexOf(tab);
        return index >= 0 && !tabWidget->tabBar()->isTabHidden(index);
    }
    return false;
}

void ConsoleWidget::addBuildMessage(const QString& message, MessageType type) {
    ConsoleTab* tab = tabs.value(ConsoleTabType::Build);
    if (tab) {
        tab->appendText("[" + formatTimestamp() + "] " + message, type);
        if (autoScroll) {
            tab->scrollToBottom();
        }
    }
    
    // Дублирование в output для важных сообщений
    if (type == MessageType::Error || type == MessageType::Success) {
        addOutputMessage(message);
    }
}

void ConsoleWidget::addErrorMessage(const QString& message, const QString& file, int line) {
    ConsoleTab* tab = tabs.value(ConsoleTabType::Errors);
    if (tab) {
        QString formattedMsg = "[" + formatTimestamp() + "] ";
        if (!file.isEmpty()) {
            formattedMsg += file + ":" + QString::number(line) + " - ";
        }
        formattedMsg += message;
        
        tab->appendText(formattedMsg, MessageType::Error);
        if (autoScroll) {
            tab->scrollToBottom();
        }
    }
    
    // Дублирование в build tab
    addBuildMessage(message, MessageType::Error);
    
    // Переключение на вкладку ошибок
    showTab(ConsoleTabType::Errors);
}

void ConsoleWidget::addOutputMessage(const QString& message) {
    ConsoleTab* tab = tabs.value(ConsoleTabType::Output);
    if (tab) {
        tab->appendText(message, MessageType::Output);
        if (autoScroll) {
            tab->scrollToBottom();
        }
    }
}

void ConsoleWidget::addDebugMessage(const QString& message) {
    if (verboseLevel < 3) return;
    
    ConsoleTab* tab = tabs.value(ConsoleTabType::Output);
    if (tab) {
        tab->appendText("[DEBUG] " + message, MessageType::Debug);
        if (autoScroll) {
            tab->scrollToBottom();
        }
    }
}

void ConsoleWidget::addCallStack(const QVector<StackFrame>& frames) {
    ConsoleTab* tab = tabs.value(ConsoleTabType::CallStack);
    if (!tab) return;
    
    tab->clear();
    
    QString html = "<html><body>";
    html += "<h3 style='color: #569cd6;'>Call Stack</h3>";
    html += "<table border='1' cellpadding='5' style='border-collapse: collapse; border-color: #3e3e3e;'>";
    html += "<tr style='background-color: #2d2d2d; color: #d4d4d4;'>";
    html += "<th>#</th><th>Function</th><th>File</th><th>Line</th></tr>";
    
    for (int i = 0; i < frames.size(); i++) {
        const StackFrame& frame = frames[i];
        html += "<tr style='color: #d4d4d4;'>";
        html += "<td>" + QString::number(i) + "</td>";
        html += "<td style='color: #dcdcaa;'>" + frame.function + "</td>";
        html += "<td style='color: #4ec9b0;'>" + frame.file + "</td>";
        html += "<td>" + QString::number(frame.line) + "</td>";
        html += "</tr>";
    }
    
    html += "</table></body></html>";
    
    tab->appendText(html, MessageType::Info);
}

void ConsoleWidget::initializeREPL(REPL* r) {
    repl = r;
    
    if (replTab) {
        replTab->initialize(repl);
        replEnabled = true;
        
        connect(repl, &REPL::outputReceived, replTab, &REPLConsoleTab::onOutputReceived);
        connect(repl, &REPL::errorReceived, replTab, &REPLConsoleTab::onErrorReceived);
        connect(repl, &REPL::promptDisplayed, replTab, &REPLConsoleTab::onPromptDisplayed);
        connect(replTab, &REPLConsoleTab::replCommandExecuted, 
                this, &ConsoleWidget::onREPLCommandExecuted);
        
        emit replInitialized();
        
        addBuildMessage("REPL initialized\n");
    }
}

void ConsoleWidget::executeREPLCommand(const QString& command) {
    if (replTab && replEnabled) {
        replTab->executeCommand(command);
    }
}

void ConsoleWidget::setREPLEnabled(bool enable) {
    replEnabled = enable;
    
    if (enable && replTab) {
        showTab(ConsoleTabType::REPL);
    }
}

bool ConsoleWidget::isREPLEnabled() const {
    return replEnabled;
}

void ConsoleWidget::setVerboseLevel(int level) {
    verboseLevel = level;
    emit verboseLevelChanged(level);
}

void ConsoleWidget::setAutoScroll(bool enable) {
    autoScroll = enable;
}

bool ConsoleWidget::getAutoScroll() const {
    return autoScroll;
}

void ConsoleWidget::setMaxLines(int count) {
    maxLines = count;
    
    for (auto it = tabs.begin(); it != tabs.end(); ++it) {
        it.value()->setMaxLines(count);
    }
}

int ConsoleWidget::getMaxLines() const {
    return maxLines;
}

void ConsoleWidget::saveOutput(const QString& path) {
    ConsoleTab* tab = tabs.value(currentTabType);
    if (!tab) return;
    
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << tab->getPlainText();
        file.close();
        
        addBuildMessage("Output saved to: " + path + "\n");
    }
}

void ConsoleWidget::saveAllTabs(const QString& dir) {
    QDir directory(dir);
    if (!directory.exists()) {
        directory.mkpath(".");
    }
    
    for (auto it = tabs.begin(); it != tabs.end(); ++it) {
        QString fileName;
        switch (it.key()) {
            case ConsoleTabType::Build:
                fileName = "build_log.txt";
                break;
            case ConsoleTabType::Errors:
                fileName = "errors_log.txt";
                break;
            case ConsoleTabType::Output:
                fileName = "output_log.txt";
                break;
            case ConsoleTabType::CallStack:
                fileName = "callstack_log.txt";
                break;
            case ConsoleTabType::REPL:
                fileName = "repl_history.txt";
                break;
        }
        
        if (!fileName.isEmpty()) {
            QString filePath = directory.filePath(fileName);
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << it.value()->getPlainText();
                file.close();
            }
        }
    }
    
    addBuildMessage("All tabs saved to: " + dir + "\n");
}

void ConsoleWidget::copyOutput() {
    ConsoleTab* tab = tabs.value(currentTabType);
    if (!tab) return;
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(tab->getPlainText());
    
    addBuildMessage("Output copied to clipboard\n");
}

void ConsoleWidget::clearAll() {
    for (auto it = tabs.begin(); it != tabs.end(); ++it) {
        it.value()->clear();
    }
    
    addBuildMessage("All tabs cleared\n");
}

void ConsoleWidget::clearTab(ConsoleTabType type) {
    ConsoleTab* tab = tabs.value(type);
    if (tab) {
        tab->clear();
    }
}

void ConsoleWidget::onTabChanged(int index) {
    QWidget* widget = tabWidget->widget(index);
    
    for (auto it = tabs.begin(); it != tabs.end(); ++it) {
        if (it.value() == widget) {
            currentTabType = it.key();
            break;
        }
    }
}

void ConsoleWidget::onContextMenu(const QPoint& pos) {
    contextMenu->exec(mapToGlobal(pos));
}

void ConsoleWidget::onClearTab() {
    clearTab(currentTabType);
}

void ConsoleWidget::onSaveTab() {
    QString path = QFileDialog::getSaveFileName(this, "Save Output",
                         "", "Text Files (*.txt);;All Files (*)");
    if (!path.isEmpty()) {
        saveOutput(path);
    }
}

void ConsoleWidget::onCopyTab() {
    copyOutput();
}

void ConsoleWidget::onFilterChanged(const QString& filter) {
    ConsoleTab* tab = tabs.value(currentTabType);
    if (tab) {
        tab->setFilter(filter);
    }
}

void ConsoleWidget::onREPLCommandExecuted(const QString& command, const QString& result) {
    emit replCommandExecuted(command, result);
}

void ConsoleWidget::onREPLInitialized() {
    emit replInitialized();
}

void ConsoleWidget::styleMessage(QTextEdit* editor, MessageType type) {
    // В полной реализации - стилизация сообщений
}

QString ConsoleWidget::formatTimestamp() const {
    return QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
}

} // namespace proxima