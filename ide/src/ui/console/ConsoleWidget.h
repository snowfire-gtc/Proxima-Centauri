#ifndef CENTAURI_CONSOLEWIDGET_H
#define CENTAURI_CONSOLEWIDGET_H

#include <QTabWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include "ConsoleTab.h"

namespace proxima {

enum class ConsoleTabType {
    Build,
    Errors,
    Output,
    CallStack,
    Debug
};

class ConsoleWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ConsoleWidget(QWidget *parent = nullptr);
    ~ConsoleWidget();
    
    // Tab management
    ConsoleTab* getTab(ConsoleTabType type) const;
    void clearTab(ConsoleTabType type);
    void showTab(ConsoleTabType type);
    
    // Message output
    void addBuildMessage(const QString& message, MessageType type = MessageType::Info);
    void addErrorMessage(const QString& message, const QString& file = "", int line = 0);
    void addOutputMessage(const QString& message);
    void addDebugMessage(const QString& message);
    void addCallStack(const QVector<StackFrame>& frames);
    
    // Configuration
    void setVerboseLevel(int level);
    int getVerboseLevel() const { return verboseLevel; }
    void setAutoScroll(bool enable);
    void setMaxLines(int count);
    
    // File operations
    void saveOutput(const QString& path);
    void copyOutput();
    void clearAll();
    
signals:
    void errorClicked(const QString& file, int line);
    void callStackFrameSelected(int frameIndex);
    void verboseLevelChanged(int level);
    
private slots:
    void onTabChanged(int index);
    void onContextMenu(const QPoint& pos);
    void onClearTab();
    void onSaveTab();
    void onCopyTab();
    void onFilterChanged(const QString& filter);
    
private:
    void setupUI();
    void setupConnections();
    void createTabs();
    void styleMessage(QPlainTextEdit* editor, MessageType type);
    QString formatTimestamp() const;
    
    QTabWidget* tabWidget;
    QMap<ConsoleTabType, ConsoleTab*> tabs;
    
    int verboseLevel;
    bool autoScroll;
    int maxLines;
    
    QMenu* contextMenu;
    QAction* clearAction;
    QAction* saveAction;
    QAction* copyAction;
    QAction* filterAction;
    
    ConsoleTabType currentTabType;
};

} // namespace proxima

#endif // CENTAURI_CONSOLEWIDGET_H