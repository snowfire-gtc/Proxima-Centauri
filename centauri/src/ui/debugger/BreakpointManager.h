#ifndef CENTAURI_BREAKPOINTMANAGER_H
#define CENTAURI_BREAKPOINTMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QToolBar>
#include <QMap>
#include "core/CompilerConnector.h"

namespace proxima {

struct BreakpointInfo {
    int id;
    QString file;
    int line;
    BreakpointType type;
    QString condition;
    bool enabled;
    int hitCount;
    QString lastHitTime;
};

class BreakpointManager : public QWidget {
    Q_OBJECT
    
public:
    explicit BreakpointManager(QWidget *parent = nullptr);
    ~BreakpointManager();
    
    // Breakpoint management
    int addBreakpoint(const QString& file, int line, 
                     BreakpointType type = BreakpointType::Unconditional,
                     const QString& condition = "");
    void removeBreakpoint(int id);
    void removeBreakpoint(const QString& file, int line);
    void enableBreakpoint(int id, bool enable);
    void toggleBreakpoint(int id);
    void clearAllBreakpoints();
    void clearFileBreakpoints(const QString& file);
    
    // Query
    BreakpointInfo getBreakpoint(int id) const;
    BreakpointInfo getBreakpoint(const QString& file, int line) const;
    QList<BreakpointInfo> getAllBreakpoints() const { return breakpoints.values(); }
    QList<BreakpointInfo> getFileBreakpoints(const QString& file) const;
    bool hasBreakpoint(const QString& file, int line) const;
    int getBreakpointCount() const { return breakpoints.size(); }
    
    // Hit tracking
    void recordHit(int id);
    void resetHitCounts();
    
    // Import/Export
    void saveToFile(const QString& path);
    void loadFromFile(const QString& path);
    
    // Synchronization with compiler
    void syncWithCompiler(CompilerConnector* connector);
    
signals:
    void breakpointAdded(int id, const QString& file, int line);
    void breakpointRemoved(int id);
    void breakpointToggled(int id, bool enabled);
    void breakpointConditionChanged(int id, const QString& condition);
    
private slots:
    void onItemChanged(QTableWidgetItem* item);
    void onItemDoubleClicked(QTableWidgetItem* item);
    void onContextMenu(const QPoint& pos);
    void onAddBreakpoint();
    void onRemoveBreakpoint();
    void onEnableBreakpoint();
    void onDisableBreakpoint();
    void onClearAll();
    void onEditCondition();
    void onSave();
    void onLoad();
    
private:
    void setupUI();
    void setupToolbar();
    void setupTable();
    void setupContextMenu();
    void populateTable();
    void updateTableRow(int row, const BreakpointInfo& bp);
    int findRowById(int id) const;
    QIcon getTypeIcon(BreakpointType type) const;
    QString typeToString(BreakpointType type) const;
    BreakpointType stringToType(const QString& str) const;
    
    QTableWidget* table;
    QToolBar* toolbar;
    QMenu* contextMenu;
    
    QMap<int, BreakpointInfo> breakpoints;
    int nextId;
    
    QAction* addAction;
    QAction* removeAction;
    QAction* enableAction;
    QAction* disableAction;
    QAction* clearAction;
    QAction* editConditionAction;
    QAction* saveAction;
    QAction* loadAction;
};

} // namespace proxima

#endif // CENTAURI_BREAKPOINTMANAGER_H