#ifndef CENTAURI_VARIABLEINSPECTOR_H
#define CENTAURI_VARIABLEINSPECTOR_H

#include <QWidget>
#include <QTreeWidget>
#include <QSplitter>
#include <QToolBar>
#include <QLineEdit>
#include <QMap>
#include "visualizers/VectorPlot.h"
#include "visualizers/MatrixView.h"
#include "visualizers/CollectionTable.h"

namespace proxima {

struct VariableInfo {
    QString name;
    QString type;
    QString value;
    int size;
    bool isExpandable;
    QMap<QString, VariableInfo> children;
};

class VariableInspector : public QWidget {
    Q_OBJECT
    
public:
    explicit VariableInspector(QWidget *parent = nullptr);
    ~VariableInspector();
    
    // Variable management
    void setVariables(const QMap<QString, VariableInfo>& vars);
    void addVariable(const VariableInfo& var);
    void removeVariable(const QString& name);
    void updateVariable(const QString& name, const VariableInfo& var);
    QMap<QString, VariableInfo> getVariables() const { return variables; }
    
    // Value editing
    bool editVariableValue(const QString& name, const QString& value);
    QString getVariableValue(const QString& name) const;
    
    // Visualization
    void showVectorPlot(const QString& varName);
    void showMatrixView(const QString& varName);
    void showCollectionView(const QString& varName);
    void showObjectInspector(const QString& varName);
    
    // Watch expressions
    void addWatchExpression(const QString& expression);
    void removeWatchExpression(int index);
    void updateWatchExpressions();
    
    // Search and filter
    void setFilter(const QString& filter);
    QString getFilter() const { return filterText; }
    
    // Configuration
    void setAutoRefresh(bool enable);
    void setRefreshInterval(int ms);
    void setMaxVariables(int count);
    
signals:
    void variableSelected(const QString& name, const QString& type);
    void variableEdited(const QString& name, const QString& oldValue, const QString& newValue);
    void visualizeRequested(const QString& name, const QString& type);
    void watchExpressionAdded(const QString& expression);
    
private slots:
    void onVariableSelected(QTreeWidgetItem* item, int column);
    void onVariableDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemExpanded(QTreeWidgetItem* item);
    void onItemCollapsed(QTreeWidgetItem* item);
    void onFilterChanged(const QString& text);
    void onAddWatch();
    void onRemoveWatch();
    void onRefresh();
    void onCopyValue();
    void onCopyName();
    void onEditValue();
    void onVisualize();
    void onContextMenu(const QPoint& pos);
    
private:
    void setupUI();
    void setupToolbar();
    void setupTree();
    void setupConnections();
    void populateTree();
    QTreeWidgetItem* createVariableItem(const VariableInfo& var, QTreeWidgetItem* parent = nullptr);
    QIcon getTypeIcon(const QString& type) const;
    QColor getTypeColor(const QString& type) const;
    void showVisualizationDialog(const QString& varName, const QString& type);
    void updateWatchValues();
    
    QTreeWidget* variableTree;
    QTreeWidget* watchTree;
    QLineEdit* filterEdit;
    QToolBar* toolbar;
    QSplitter* mainSplitter;
    
    QMap<QString, VariableInfo> variables;
    QStringList watchExpressions;
    QString filterText;
    
    bool autoRefresh;
    int refreshInterval;
    int maxVariables;
    
    QMenu* contextMenu;
    QAction* editValueAction;
    QAction* copyValueAction;
    QAction* copyNameAction;
    QAction* visualizeAction;
    QAction* addWatchAction;
    QAction* removeWatchAction;
    QAction* refreshAction;
    
    // Visualization widgets (cached)
    VectorPlot* vectorPlot;
    MatrixView* matrixView;
    CollectionTable* collectionTable;
};

} // namespace proxima

#endif // CENTAURI_VARIABLEINSPECTOR_H