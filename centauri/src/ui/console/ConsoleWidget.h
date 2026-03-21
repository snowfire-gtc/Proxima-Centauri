#ifndef CENTAURI_CONSOLEWIDGET_H
#define CENTAURI_CONSOLEWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QFont>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include "core/CompilerConnector.h"
#include "runtime/REPL.h"
#include "utils/Logger.h"

namespace proxima {

// ============================================================================
// Типы вкладок консоли
// ============================================================================

/**
 * @brief Типы вкладок консоли
 * 
 * Согласно требованию ide.txt пункт 12:
 * - процесс сборки
 * - ошибки сборки приложения
 * - стандартный вывод приложения
 * - call-stack со значениями параметров
 * - REPL (интерактивная консоль Proxima)
 */
enum class ConsoleTabType {
    Build,          // Процесс сборки
    Errors,         // Ошибки сборки
    Output,         // Стандартный вывод
    CallStack,      // Call-stack
    REPL            // REPL консоль (интерактивная)
};

/**
 * @brief Типы сообщений консоли
 */
enum class MessageType {
    Info,
    Warning,
    Error,
    Success,
    Debug,
    Command,
    Output
};

// ============================================================================
// Вкладка консоли
// ============================================================================

/**
 * @brief Базовый класс вкладки консоли
 */
class ConsoleTab : public QWidget {
    Q_OBJECT
    
public:
    explicit ConsoleTab(ConsoleTabType type, QWidget *parent = nullptr);
    ~ConsoleTab();
    
    ConsoleTabType getTabType() const { return tabType; }
    void appendText(const QString& text, MessageType type = MessageType::Info);
    void clear();
    void setPlaceholderText(const QString& text);
    QString getPlainText() const;
    int getLineCount() const;
    void scrollToBottom();
    void setMaxLines(int count);
    void setFilter(const QString& filter);
    
protected:
    QTextEdit* textEdit;
    ConsoleTabType tabType;
    int maxLines;
    QString currentFilter;
    
private:
    void setupUI();
    void applyFilter();
    QColor getMessageColor(MessageType type) const;
};

// ============================================================================
// REPL вкладка консоли
// ============================================================================

/**
 * @brief Вкладка REPL консоли
 * 
 * Интерактивная консоль для выполнения кода Proxima напрямую.
 * Согласно требованию language.txt - язык интерпретируемый с возможностью компиляции.
 */
class REPLConsoleTab : public ConsoleTab {
    Q_OBJECT
    
public:
    explicit REPLConsoleTab(QWidget *parent = nullptr);
    ~REPLConsoleTab();
    
    void initialize(REPL* repl);
    void setPrompt(const QString& prompt);
    QString getPrompt() const;
    void executeCommand(const QString& command);
    void setHistoryEnabled(bool enable);
    bool isHistoryEnabled() const;
    void clearHistory();
    int getHistorySize() const;
    void saveHistory(const QString& path);
    void loadHistory(const QString& path);
    
protected:
    void keyPressEvent(QKeyEvent* event) override;
    
private slots:
    void onInputSubmitted(const QString& input);
    void onOutputReceived(const QString& output);
    void onErrorReceived(const QString& error);
    void onPromptDisplayed();
    void onCommandHistory(const QStringList& history);
    
private:
    void setupUI();
    void setupConnections();
    void displayPrompt();
    void appendInput(const QString& input);
    void appendOutput(const QString& output);
    void navigateHistory(int direction);
    QString getCurrentInput() const;
    void setCurrentInput(const QString& input);
    
    REPL* repl;
    QLineEdit* inputLine;
    QString prompt;
    QStringList commandHistory;
    int historyIndex;
    bool historyEnabled;
    QString currentInput;
    
    QAction* clearAction;
    QAction* saveAction;
    QAction* loadAction;
    QAction* historyAction;
    QAction* copyAction;
    QAction* pasteAction;
};

// ============================================================================
// Виджет консоли
// ============================================================================

/**
 * @brief Главный виджет консоли IDE
 * 
 * Содержит вкладки для различных типов вывода консоли.
 * Согласно требованию ide.txt пункт 12.
 */
class ConsoleWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ConsoleWidget(QWidget *parent = nullptr);
    ~ConsoleWidget();
    
    // Вкладки
    ConsoleTab* getTab(ConsoleTabType type) const;
    REPLConsoleTab* getREPLTab() const { return replTab; }
    void showTab(ConsoleTabType type);
    void hideTab(ConsoleTabType type);
    bool isTabVisible(ConsoleTabType type) const;
    
    // Сообщения
    void addBuildMessage(const QString& message, MessageType type = MessageType::Info);
    void addErrorMessage(const QString& message, const QString& file = "", int line = 0);
    void addOutputMessage(const QString& message);
    void addDebugMessage(const QString& message);
    void addCallStack(const QVector<StackFrame>& frames);
    
    // REPL
    void initializeREPL(REPL* repl);
    void executeREPLCommand(const QString& command);
    bool isREPLInitialized() const { return repl != nullptr; }
    void setREPLEnabled(bool enable);
    bool isREPLEnabled() const { return replEnabled; }
    
    // Конфигурация
    void setVerboseLevel(int level);
    int getVerboseLevel() const { return verboseLevel; }
    void setAutoScroll(bool enable);
    bool getAutoScroll() const { return autoScroll; }
    void setMaxLines(int count);
    int getMaxLines() const { return maxLines; }
    
    // Файловые операции
    void saveOutput(const QString& path);
    void saveAllTabs(const QString& dir);
    void copyOutput();
    void clearAll();
    void clearTab(ConsoleTabType type);
    
signals:
    void errorClicked(const QString& file, int line);
    void callStackFrameSelected(int frameIndex);
    void verboseLevelChanged(int level);
    void replCommandExecuted(const QString& command, const QString& result);
    void replInitialized();
    
private slots:
    void onTabChanged(int index);
    void onContextMenu(const QPoint& pos);
    void onClearTab();
    void onSaveTab();
    void onCopyTab();
    void onFilterChanged(const QString& filter);
    void onREPLCommandExecuted(const QString& command, const QString& result);
    void onREPLInitialized();
    
private:
    void setupUI();
    void setupConnections();
    void createTabs();
    void styleMessage(QTextEdit* editor, MessageType type);
    QString formatTimestamp() const;
    void updateTabIcons();
    
    QTabWidget* tabWidget;
    QMap<ConsoleTabType, ConsoleTab*> tabs;
    REPLConsoleTab* replTab;
    
    REPL* repl;
    bool replEnabled;
    int verboseLevel;
    bool autoScroll;
    int maxLines;
    
    QMenu* contextMenu;
    QAction* clearAction;
    QAction* saveAction;
    QAction* copyAction;
    QAction* filterAction;
    QAction* showREPLAction;
    QAction* hideREPLAction;
    
    ConsoleTabType currentTabType;
};

} // namespace proxima

#endif // CENTAURI_CONSOLEWIDGET_H