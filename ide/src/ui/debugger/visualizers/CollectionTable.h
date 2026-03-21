#ifndef CENTAURI_COLLECTIONTABLE_H
#define CENTAURI_COLLECTIONTABLE_H

#include <QWidget>
#include <QTableWidget>
#include <QToolBar>
#include <QMenu>
#include <QVector>
#include <QMap>
#include <QLineEdit>

namespace proxima {

struct CollectionCell {
    QString value;
    QString type;
    bool isEditable;
    QColor backgroundColor;
    QColor foregroundColor;
};

class CollectionTable : public QWidget {
    Q_OBJECT
    
public:
    explicit CollectionTable(QWidget *parent = nullptr);
    ~CollectionTable();
    
    // Data management
    void setData(const QVector<QVector<QString>>& data, const QVector<QString>& headers);
    void clearData();
    QVector<QVector<QString>> getData() const;
    QVector<QString> getHeaders() const { return headers; }
    int rowCount() const { return data.size(); }
    int columnCount() const { return headers.size(); }
    
    // Cell access
    QString getCell(int row, int col) const;
    void setCell(int row, int col, const QString& value);
    CollectionCell getCellInfo(int row, int col) const;
    void setCellInfo(int row, int col, const CollectionCell& info);
    
    // Row operations
    void addRow(const QVector<QString>& values);
    void removeRow(int row);
    void insertRow(int row, const QVector<QString>& values);
    void clearRows();
    
    // Column operations
    void addColumn(const QString& header);
    void removeColumn(int col);
    void insertColumn(int col, const QString& header);
    void setHeaders(const QVector<QString>& newHeaders);
    void renameColumn(int col, const QString& name);
    
    // Selection
    void selectRow(int row);
    void selectColumn(int col);
    void selectRange(int startRow, int startCol, int endRow, int endCol);
    void clearSelection();
    QVector<QVector<QString>> getSelectedData() const;
    bool hasSelection() const;
    
    // Sorting and filtering
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void setFilter(int column, const QString& filter);
    void clearFilter(int column);
    void clearAllFilters();
    void applyFilters();
    
    // Export
    void copyToClipboard();
    void copySelectionToClipboard();
    void saveAsCSV(const QString& path);
    void saveAsJSON(const QString& path);
    void saveAsExcel(const QString& path);
    void loadFromCSV(const QString& path);
    void loadFromJSON(const QString& path);
    
    // Configuration
    void setAlternatingRowColors(bool enable);
    void setShowGrid(bool show);
    void setReadOnly(bool readOnly);
    void setAutoResize(bool enable);
    void setWordWrap(bool enable);
    void setFontSize(int size);
    
    // Search
    int findText(const QString& text, int startRow = 0, int startCol = 0) const;
    QVector<QPoint> findAll(const QString& text) const;
    
    // Statistics
    QMap<QString, double> getColumnStatistics(int column) const;
    
signals:
    void cellChanged(int row, int col, const QString& value);
    void cellDoubleClicked(int row, int col);
    void selectionChanged();
    void dataModified();
    
private slots:
    void onCellChanged(int row, int col);
    void onCellDoubleClicked(int row, int col);
    void onHeaderClicked(int col);
    void onContextMenu(const QPoint& pos);
    void onFilterChanged();
    void onSortAscending();
    void onSortDescending();
    void onCopy();
    void onPaste();
    void onDeleteRow();
    void onDeleteColumn();
    void onAddRow();
    void onAddColumn();
    void onSaveCSV();
    void onLoadCSV();
    void onExportJSON();
    void onFind();
    void onReplace();
    
private:
    void setupUI();
    void setupToolbar();
    void setupContextMenu();
    void setupFilterRow();
    void populateTable();
    void updateTableWidget();
    void applyCellStyle(int row, int col);
    QString detectType(const QString& value) const;
    QVariant convertValue(const QString& value, const QString& type) const;
    
    QTableWidget* table;
    QToolBar* toolbar;
    QMenu* contextMenu;
    QLineEdit* filterEdit;
    QVector<QLineEdit*> filterEditors;
    
    QVector<QVector<QString>> data;
    QVector<QString> headers;
    QVector<QVector<CollectionCell>> cellInfo;
    
    QVector<QString> filters;
    bool hasActiveFilters;
    
    bool alternatingRowColors;
    bool showGrid;
    bool readOnly;
    bool autoResize;
    bool wordWrap;
    int fontSize;
    
    QAction* sortAscAction;
    QAction* sortDescAction;
    QAction* copyAction;
    QAction* pasteAction;
    QAction* deleteRowAction;
    QAction* deleteColumnAction;
    QAction* addRowAction;
    QAction* addColumnAction;
    QAction* saveCSVAction;
    QAction* loadCSVAction;
    QAction* exportJSONAction;
    QAction* findAction;
    QAction* replaceAction;
};

} // namespace proxima

#endif // CENTAURI_COLLECTIONTABLE_H