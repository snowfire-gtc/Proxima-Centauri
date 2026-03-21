#ifndef CENTAURI_REPLCONSOLETAB_H
#define CENTAURI_REPLCONSOLETAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QScrollBar>
#include <QFont>
#include <QColor>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QInputDialog>
#include <QCompleter>
#include <QListWidget>
#include <QPopupWidget>
#include "runtime/REPL.h"
#include "editor/REPLSyntaxHighlighter.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Вкладка REPL консоли для интерактивного выполнения кода Proxima
 * 
 * Согласно требованию language.txt пункт 4:
 * Язык интерпретируемый, с возможностью компиляции.
 * 
 * Согласно требованию ide.txt пункт 12:
 * В IDE есть консоль, в которой отображаются в отдельных табах различные типы вывода.
 */
class REPLConsoleTab : public QWidget {
    Q_OBJECT
    
public:
    explicit REPLConsoleTab(QWidget *parent = nullptr);
    ~REPLConsoleTab();
    
    // Инициализация
    void initialize(REPL* repl);
    bool isInitialized() const { return repl != nullptr; }
    
    // Prompt
    void setPrompt(const QString& prompt);
    QString getPrompt() const { return prompt; }
    
    // Выполнение команд
    void executeCommand(const QString& command);
    void executeCurrentInput();
    
    // История команд
    void setHistoryEnabled(bool enable);
    bool isHistoryEnabled() const { return historyEnabled; }
    void clearHistory();
    int getHistorySize() const { return commandHistory.size(); }
    void saveHistory(const QString& path);
    void loadHistory(const QString& path);
    void navigateHistory(int direction);
    
    // Автодополнение
    void setAutoCompletionEnabled(bool enable);
    bool isAutoCompletionEnabled() const { return autoCompletionEnabled; }
    void showCompletionPopup();
    void hideCompletionPopup();
    
    // Конфигурация
    void setMaxHistorySize(int size);
    int getMaxHistorySize() const { return maxHistorySize; }
    void setAutoScroll(bool enable);
    bool getAutoScroll() const { return autoScroll; }
    void setSyntaxHighlightingEnabled(bool enable);
    bool isSyntaxHighlightingEnabled() const { return syntaxHighlightingEnabled; }
    
    // Очистка
    void clear();
    void clearOutput();
    
    // Вывод
    void appendPrompt();
    void appendInput(const QString& input);
    void appendOutput(const QString& output);
    void appendError(const QString& error);
    void appendWarning(const QString& warning);
    void appendDebug(const QString& debug);
    
    // Состояние
    bool isBusy() const { return busy; }
    void setBusy(bool b) { busy = b; }
    
    // Фокус
    void setFocusToInput();
    bool hasFocus() const { return inputLine->hasFocus(); }
    
signals:
    void commandExecuted(const QString& command, const QString& result);
    void promptDisplayed();
    void inputReceived(const QString& input);
    void historyChanged();
    void initializationComplete();
    
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    
private slots:
    void onInputSubmitted();
    void onInputTextChanged(const QString& text);
    void onOutputReceived(const QString& output);
    void onErrorReceived(const QString& error);
    void onPromptDisplayed();
    void onCompletionItemSelected(const QString& item);
    void onClearTriggered();
    void onSaveHistoryTriggered();
    void onLoadHistoryTriggered();
    void onCopyTriggered();
    void onPasteTriggered();
    void onSelectAllTriggered();
    
private:
    void setupUI();
    void setupConnections();
    void setupContextMenu();
    void setupCompletionPopup();
    void updateCompletionPopup();
    void applySyntaxHighlighting();
    void scrollToBottom();
    QString getCurrentInput() const;
    void setCurrentInput(const QString& input);
    int getInputCursorPosition() const;
    void setInputCursorPosition(int pos);
    QStringList getCompletions(const QString& prefix) const;
    QString getWordUnderCursor() const;
    void insertCompletion(const QString& completion);
    QColor getMessageColor(MessageType type) const;
    void formatOutput(const QString& text, MessageType type);
    
    // REPL
    REPL* repl;
    
    // UI компоненты
    QTextEdit* outputDisplay;         // Область вывода
    QPlainTextEdit* inputLine;        // Строка ввода (многострочная)
    QLabel* promptLabel;              // Метка prompt
    QWidget* inputContainer;          // Контейнер для ввода
    REPLSyntaxHighlighter* syntaxHighlighter;  // Подсветка синтаксиса
    
    // Prompt
    QString prompt;
    QColor promptColor;
    
    // История
    QStringList commandHistory;
    int historyIndex;
    bool historyEnabled;
    int maxHistorySize;
    QString savedInput;               // Сохранённый текущий ввод
    
    // Автодополнение
    bool autoCompletionEnabled;
    QPopupWidget* completionPopup;
    QListWidget* completionList;
    QStringList currentCompletions;
    int selectedCompletionIndex;
    
    // Конфигурация
    bool autoScroll;
    bool syntaxHighlightingEnabled;
    bool busy;
    
    // Контекстное меню
    QMenu* contextMenu;
    QAction* clearAction;
    QAction* saveAction;
    QAction* loadAction;
    QAction* copyAction;
    QAction* pasteAction;
    QAction* selectAllAction;
    QAction* historyAction;
    
    // Стили
    QFont consoleFont;
    QMap<MessageType, QColor> messageColors;
    
    // Счётчики
    int commandCount;
    int outputLineCount;
};

} // namespace proxima

#endif // CENTAURI_REPLCONSOLETAB_H