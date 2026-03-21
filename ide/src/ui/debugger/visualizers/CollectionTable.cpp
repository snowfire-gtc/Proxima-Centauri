#include "CollectionTable.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QRegularExpression>
#include "utils/Logger.h"

namespace proxima {

CollectionTable::CollectionTable(QWidget *parent)
    : QWidget(parent)
    , hasActiveFilters(false)
    , alternatingRowColors(true)
    , showGrid(true)
    , readOnly(false)
    , autoResize(true)
    , wordWrap(false)
    , fontSize(10) {
    
    setupUI();
    setupToolbar();
    setupContextMenu();
    setupFilterRow();
}

CollectionTable::~CollectionTable() {}

void CollectionTable::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    
    table = new QTableWidget(this);
    table->setAlternatingRowColors(alternatingRowColors);
    table->setShowGrid(showGrid);
    table->setWordWrap(wordWrap);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(true);
    
    QFont font = table->font();
    font.setPointSize(fontSize);
    table->setFont(font);
    
    layout->addWidget(table, 1);
    
    // Connect signals
    connect(table, &QTableWidget::cellChanged, this, &CollectionTable::onCellChanged);
    connect(table, &QTableWidget::cellDoubleClicked, this, &CollectionTable::onCellDoubleClicked);
    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &CollectionTable::onHeaderClicked);
    connect(table, &QTableWidget::customContextMenuRequested, this, &CollectionTable::onContextMenu);
}

void CollectionTable::setupToolbar() {
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    
    sortAscAction = toolbar->addAction(QIcon(":/icons/sort-asc.svg"), "Sort Ascending", this, &CollectionTable::onSortAscending);
    sortDescAction = toolbar->addAction(QIcon(":/icons/sort-desc.svg"), "Sort Descending", this, &CollectionTable::onSortDescending);
    
    toolbar->addSeparator();
    
    copyAction = toolbar->addAction(QIcon(":/icons/copy.svg"), "Copy", this, &CollectionTable::onCopy);
    copyAction->setShortcut(QKeySequence::Copy);
    
    pasteAction = toolbar->addAction(QIcon(":/icons/paste.svg"), "Paste", this, &CollectionTable::onPaste);
    pasteAction->setShortcut(QKeySequence::Paste);
    
    toolbar->addSeparator();
    
    addRowAction = toolbar->addAction(QIcon(":/icons/add-row.svg"), "Add Row", this, &CollectionTable::onAddRow);
    addColumnAction = toolbar->addAction(QIcon(":/icons/add-column.svg"), "Add Column", this, &CollectionTable::onAddColumn);
    
    deleteRowAction = toolbar->addAction(QIcon(":/icons/delete-row.svg"), "Delete Row", this, &CollectionTable::onDeleteRow);
    deleteColumnAction = toolbar->addAction(QIcon(":/icons/delete-column.svg"), "Delete Column", this, &CollectionTable::onDeleteColumn);
    
    toolbar->addSeparator();
    
    saveCSVAction = toolbar->addAction(QIcon(":/icons/save.svg"), "Save CSV", this, &CollectionTable::onSaveCSV);
    loadCSVAction = toolbar->addAction(QIcon(":/icons/open.svg"), "Load CSV", this, &CollectionTable::onLoadCSV);
    exportJSONAction = toolbar->addAction(QIcon(":/icons/export.svg"), "Export JSON", this, &CollectionTable::onExportJSON);
    
    toolbar->addSeparator();
    
    findAction = toolbar->addAction(QIcon(":/icons/find.svg"), "Find", this, &CollectionTable::onFind);
    findAction->setShortcut(QKeySequence::Find);
    
    replaceAction = toolbar->addAction(QIcon(":/icons/replace.svg"), "Replace", this, &CollectionTable::onReplace);
    replaceAction->setShortcut(QKeySequence::Replace);
}

void CollectionTable::setupContextMenu() {
    contextMenu = new QMenu(this);
    contextMenu->addAction(sortAscAction);
    contextMenu->addAction(sortDescAction);
    contextMenu->addSeparator();
    contextMenu->addAction(copyAction);
    contextMenu->addAction(pasteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(addRowAction);
    contextMenu->addAction(addColumnAction);
    contextMenu->addAction(deleteRowAction);
    contextMenu->addAction(deleteColumnAction);
    contextMenu->addSeparator();
    contextMenu->addAction(saveCSVAction);
    contextMenu->addAction(loadCSVAction);
    contextMenu->addAction(exportJSONAction);
}

void CollectionTable::setupFilterRow() {
    // Filter row is created dynamically based on column count
}

void CollectionTable::setData(const QVector<QVector<QString>>& newData, const QVector<QString>& newHeaders) {
    data = newData;
    headers = newHeaders;
    
    // Initialize cell info
    cellInfo.clear();
    for (int i = 0; i < data.size(); i++) {
        QVector<CollectionCell> row;
        for (int j = 0; j < data[i].size(); j++) {
            CollectionCell cell;
            cell.value = data[i][j];
            cell.type = detectType(data[i][j]);
            cell.isEditable = !readOnly;
            row.append(cell);
        }
        cellInfo.append(row);
    }
    
    filters.clear();
    for (int i = 0; i < headers.size(); i++) {
        filters.append("");
    }
    hasActiveFilters = false;
    
    populateTable();
}

void CollectionTable::clearData() {
    data.clear();
    headers.clear();
    cellInfo.clear();
    filters.clear();
    hasActiveFilters = false;
    table->clear();
}

QVector<QVector<QString>> CollectionTable::getData() const {
    return data;
}

QString CollectionTable::getCell(int row, int col) const {
    if (row >= 0 && row < data.size() && col >= 0 && col < headers.size()) {
        return data[row][col];
    }
    return "";
}

void CollectionTable::setCell(int row, int col, const QString& value) {
    if (row >= 0 && row < data.size() && col >= 0 && col < headers.size()) {
        data[row][col] = value;
        cellInfo[row][col].value = value;
        cellInfo[row][col].type = detectType(value);
        
        QTableWidgetItem* item = table->item(row, col);
        if (item) {
            item->setText(value);
        }
        
        emit cellChanged(row, col, value);
        emit dataModified();
    }
}

CollectionCell CollectionTable::getCellInfo(int row, int col) const {
    if (row >= 0 && row < cellInfo.size() && col >= 0 && col < cellInfo[row].size()) {
        return cellInfo[row][col];
    }
    return CollectionCell();
}

void CollectionTable::setCellInfo(int row, int col, const CollectionCell& info) {
    if (row >= 0 && row < cellInfo.size() && col >= 0 && col < cellInfo[row].size()) {
        cellInfo[row][col] = info;
    }
}

void CollectionTable::addRow(const QVector<QString>& values) {
    data.append(values);
    
    QVector<CollectionCell> row;
    for (const QString& value : values) {
        CollectionCell cell;
        cell.value = value;
        cell.type = detectType(value);
        cell.isEditable = !readOnly;
        row.append(cell);
    }
    cellInfo.append(row);
    
    populateTable();
    emit dataModified();
}

void CollectionTable::removeRow(int row) {
    if (row >= 0 && row < data.size()) {
        data.removeAt(row);
        cellInfo.removeAt(row);
        populateTable();
        emit dataModified();
    }
}

void CollectionTable::insertRow(int row, const QVector<QString>& values) {
    if (row >= 0 && row <= data.size()) {
        data.insert(row, values);
        
        QVector<CollectionCell> newRow;
        for (const QString& value : values) {
            CollectionCell cell;
            cell.value = value;
            cell.type = detectType(value);
            cell.isEditable = !readOnly;
            newRow.append(cell);
        }
        cellInfo.insert(row, newRow);
        
        populateTable();
        emit dataModified();
    }
}

void CollectionTable::clearRows() {
    data.clear();
    cellInfo.clear();
    populateTable();
    emit dataModified();
}

void CollectionTable::addColumn(const QString& header) {
    headers.append(header);
    filters.append("");
    
    for (int i = 0; i < data.size(); i++) {
        if (data[i].size() < headers.size()) {
            data[i].append("");
            cellInfo[i].append(CollectionCell());
        }
    }
    
    populateTable();
    emit dataModified();
}

void CollectionTable::removeColumn(int col) {
    if (col >= 0 && col < headers.size()) {
        headers.removeAt(col);
        filters.removeAt(col);
        
        for (int i = 0; i < data.size(); i++) {
            if (col < data[i].size()) {
                data[i].removeAt(col);
                cellInfo[i].removeAt(col);
            }
        }
        
        populateTable();
        emit dataModified();
    }
}

void CollectionTable::insertColumn(int col, const QString& header) {
    if (col >= 0 && col <= headers.size()) {
        headers.insert(col, header);
        filters.insert(col, "");
        
        for (int i = 0; i < data.size(); i++) {
            if (col <= data[i].size()) {
                data[i].insert(col, "");
                cellInfo[i].insert(col, CollectionCell());
            }
        }
        
        populateTable();
        emit dataModified();
    }
}

void CollectionTable::setHeaders(const QVector<QString>& newHeaders) {
    headers = newHeaders;
    populateTable();
}

void CollectionTable::renameColumn(int col, const QString& name) {
    if (col >= 0 && col < headers.size()) {
        headers[col] = name;
        table->setHorizontalHeaderItem(col, new QTableWidgetItem(name));
    }
}

void CollectionTable::selectRow(int row) {
    if (row >= 0 && row < table->rowCount()) {
        table->selectRow(row);
    }
}

void CollectionTable::selectColumn(int col) {
    if (col >= 0 && col < table->columnCount()) {
        table->selectColumn(col);
    }
}

void CollectionTable::selectRange(int startRow, int startCol, int endRow, int endCol) {
    QTableWidgetSelectionRange range(startRow, startCol, endRow, endCol);
    table->setRangeSelected(range, true);
}

void CollectionTable::clearSelection() {
    table->clearSelection();
}

QVector<QVector<QString>> CollectionTable::getSelectedData() const {
    QVector<QVector<QString>> selected;
    
    QList<QTableWidgetSelectionRange> ranges = table->selectedRanges();
    for (const QTableWidgetSelectionRange& range : ranges) {
        for (int r = range.topRow(); r <= range.bottomRow(); r++) {
            QVector<QString> row;
            for (int c = range.leftColumn(); c <= range.rightColumn(); c++) {
                QTableWidgetItem* item = table->item(r, c);
                if (item) {
                    row.append(item->text());
                } else {
                    row.append("");
                }
            }
            selected.append(row);
        }
    }
    
    return selected;
}

bool CollectionTable::hasSelection() const {
    return table->selectedRanges().size() > 0;
}

void CollectionTable::sort(int column, Qt::SortOrder order) {
    if (column < 0 || column >= headers.size()) return;
    
    // Create index array
    QVector<int> indices(data.size());
    for (int i = 0; i < data.size(); i++) {
        indices[i] = i;
    }
    
    // Sort indices based on column values
    std::sort(indices.begin(), indices.end(), [this, column, order](int a, int b) {
        QString valA = data[a][column];
        QString valB = data[b][column];
        
        // Try numeric comparison
        bool okA, okB;
        double numA = valA.toDouble(&okA);
        double numB = valB.toDouble(&okB);
        
        bool result;
        if (okA && okB) {
            result = (numA < numB);
        } else {
            result = (valA < valB);
        }
        
        return (order == Qt::AscendingOrder) ? result : !result;
    });
    
    // Reorder data
    QVector<QVector<QString>> newData;
    QVector<QVector<CollectionCell>> newCellInfo;
    
    for (int idx : indices) {
        newData.append(data[idx]);
        newCellInfo.append(cellInfo[idx]);
    }
    
    data = newData;
    cellInfo = newCellInfo;
    
    populateTable();
    emit dataModified();
}

void CollectionTable::setFilter(int column, const QString& filter) {
    if (column >= 0 && column < filters.size()) {
        filters[column] = filter;
        hasActiveFilters = !filter.isEmpty();
        applyFilters();
    }
}

void CollectionTable::clearFilter(int column) {
    if (column >= 0 && column < filters.size()) {
        filters[column] = "";
        applyFilters();
    }
}

void CollectionTable::clearAllFilters() {
    for (int i = 0; i < filters.size(); i++) {
        filters[i] = "";
    }
    hasActiveFilters = false;
    populateTable();
}

void CollectionTable::applyFilters() {
    // Would filter the displayed data
    // Simplified implementation
    populateTable();
}

void CollectionTable::copyToClipboard() {
    QString csvData;
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
            csvData += data[i][j];
            if (j < data[i].size() - 1) csvData += ",";
        }
        csvData += "\n";
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvData);
}

void CollectionTable::copySelectionToClipboard() {
    QVector<QVector<QString>> selected = getSelectedData();
    
    QString csvData;
    for (int i = 0; i < selected.size(); i++) {
        for (int j = 0; j < selected[i].size(); j++) {
            csvData += selected[i][j];
            if (j < selected[i].size() - 1) csvData += ",";
        }
        csvData += "\n";
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(csvData);
}

void CollectionTable::saveAsCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR("Failed to save CSV: " + path);
        return;
    }
    
    QTextStream out(&file);
    
    // Write headers
    for (int j = 0; j < headers.size(); j++) {
        out << headers[j];
        if (j < headers.size() - 1) out << ",";
    }
    out << "\n";
    
    // Write data
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
            out << data[i][j];
            if (j < data[i].size() - 1) out << ",";
        }
        out << "\n";
    }
    
    file.close();
    LOG_INFO("CSV saved: " + path);
}

void CollectionTable::saveAsJSON(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("Failed to save JSON: " + path);
        return;
    }
    
    QJsonArray jsonArray;
    
    for (int i = 0; i < data.size(); i++) {
        QJsonObject rowObj;
        for (int j = 0; j < headers.size() && j < data[i].size(); j++) {
            rowObj[headers[j]] = data[i][j];
        }
        jsonArray.append(rowObj);
    }
    
    QJsonDocument doc(jsonArray);
    file.write(doc.toJson());
    file.close();
    LOG_INFO("JSON saved: " + path);
}

void CollectionTable::saveAsExcel(const QString& path) {
    // Would use a library like QtXlsx or similar
    // Simplified - save as CSV for now
    saveAsCSV(path);
}

void CollectionTable::loadFromCSV(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("Failed to load CSV: " + path);
        return;
    }
    
    QTextStream in(&file);
    data.clear();
    headers.clear();
    
    QString line = in.readLine();
    if (!line.isEmpty()) {
        headers = line.split(",");
    }
    
    while (!in.atEnd()) {
        line = in.readLine();
        if (!line.isEmpty()) {
            data.append(line.split(","));
        }
    }
    
    file.close();
    
    // Reinitialize cell info
    cellInfo.clear();
    for (int i = 0; i < data.size(); i++) {
        QVector<CollectionCell> row;
        for (int j = 0; j < data[i].size(); j++) {
            CollectionCell cell;
            cell.value = data[i][j];
            cell.type = detectType(data[i][j]);
            cell.isEditable = !readOnly;
            row.append(cell);
        }
        cellInfo.append(row);
    }
    
    populateTable();
    LOG_INFO("CSV loaded: " + path);
}

void CollectionTable::loadFromJSON(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("Failed to load JSON: " + path);
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isArray()) {
        LOG_ERROR("Invalid JSON format");
        return;
    }
    
    QJsonArray jsonArray = doc.array();
    data.clear();
    headers.clear();
    
    if (jsonArray.isEmpty()) return;
    
    // Extract headers from first object
    QJsonObject firstObj = jsonArray[0].toObject();
    headers = firstObj.keys();
    
    // Extract data
    for (const QJsonValue& val : jsonArray) {
        QJsonObject obj = val.toObject();
        QVector<QString> row;
        for (const QString& header : headers) {
            row.append(obj[header].toString());
        }
        data.append(row);
    }
    
    // Reinitialize cell info
    cellInfo.clear();
    for (int i = 0; i < data.size(); i++) {
        QVector<CollectionCell> row;
        for (int j = 0; j < data[i].size(); j++) {
            CollectionCell cell;
            cell.value = data[i][j];
            cell.type = detectType(data[i][j]);
            cell.isEditable = !readOnly;
            row.append(cell);
        }
        cellInfo.append(row);
    }
    
    populateTable();
    LOG_INFO("JSON loaded: " + path);
}

void CollectionTable::setAlternatingRowColors(bool enable) {
    alternatingRowColors = enable;
    table->setAlternatingRowColors(enable);
}

void CollectionTable::setShowGrid(bool show) {
    showGrid = show;
    table->setShowGrid(show);
}

void CollectionTable::setReadOnly(bool readOnly) {
    this->readOnly = readOnly;
    table->setEditTriggers(readOnly ? QAbstractItemView::NoEditTriggers 
                                    : QAbstractItemView::DoubleClicked);
}

void CollectionTable::setAutoResize(bool enable) {
    autoResize = enable;
    if (enable) {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void CollectionTable::setWordWrap(bool enable) {
    wordWrap = enable;
    table->setWordWrap(enable);
}

void CollectionTable::setFontSize(int size) {
    fontSize = size;
    QFont font = table->font();
    font.setPointSize(size);
    table->setFont(font);
}

int CollectionTable::findText(const QString& text, int startRow, int startCol) const {
    for (int i = startRow; i < data.size(); i++) {
        for (int j = startCol; j < data[i].size(); j++) {
            if (data[i][j].contains(text, Qt::CaseInsensitive)) {
                return i * headers.size() + j;
            }
        }
    }
    return -1;
}

QVector<QPoint> CollectionTable::findAll(const QString& text) const {
    QVector<QPoint> results;
    
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size(); j++) {
            if (data[i][j].contains(text, Qt::CaseInsensitive)) {
                results.append(QPoint(j, i));
            }
        }
    }
    
    return results;
}

QMap<QString, double> CollectionTable::getColumnStatistics(int column) const {
    QMap<QString, double> stats;
    
    if (column < 0 || column >= headers.size()) return stats;
    
    double sum = 0;
    double min = 0;
    double max = 0;
    int count = 0;
    
    for (int i = 0; i < data.size(); i++) {
        if (column < data[i].size()) {
            bool ok;
            double value = data[i][column].toDouble(&ok);
            if (ok) {
                if (count == 0) {
                    min = max = value;
                } else {
                    if (value < min) min = value;
                    if (value > max) max = value;
                }
                sum += value;
                count++;
            }
        }
    }
    
    stats["count"] = count;
    stats["sum"] = sum;
    stats["mean"] = (count > 0) ? sum / count : 0;
    stats["min"] = min;
    stats["max"] = max;
    
    return stats;
}

void CollectionTable::populateTable() {
    table->clear();
    
    // Set headers
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    
    // Set row count
    table->setRowCount(data.size());
    
    // Populate cells
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[i].size() && j < headers.size(); j++) {
            QTableWidgetItem* item = new QTableWidgetItem(data[i][j]);
            
            // Apply cell styling
            if (i < cellInfo.size() && j < cellInfo[i].size()) {
                const CollectionCell& cell = cellInfo[i][j];
                if (cell.backgroundColor.isValid()) {
                    item->setBackground(cell.backgroundColor);
                }
                if (cell.foregroundColor.isValid()) {
                    item->setForeground(cell.foregroundColor);
                }
            }
            
            table->setItem(i, j, item);
        }
    }
    
    if (autoResize) {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void CollectionTable::updateTableWidget() {
    populateTable();
}

void CollectionTable::applyCellStyle(int row, int col) {
    if (row >= 0 && row < table->rowCount() && col >= 0 && col < table->columnCount()) {
        QTableWidgetItem* item = table->item(row, col);
        if (item && row < cellInfo.size() && col < cellInfo[row].size()) {
            const CollectionCell& cell = cellInfo[row][col];
            if (cell.backgroundColor.isValid()) {
                item->setBackground(cell.backgroundColor);
            }
            if (cell.foregroundColor.isValid()) {
                item->setForeground(cell.foregroundColor);
            }
        }
    }
}

QString CollectionTable::detectType(const QString& value) const {
    if (value.isEmpty()) return "empty";
    
    // Try integer
    bool ok;
    value.toInt(&ok);
    if (ok) return "int";
    
    // Try double
    value.toDouble(&ok);
    if (ok) return "double";
    
    // Try boolean
    if (value == "true" || value == "false") return "bool";
    
    return "string";
}

QVariant CollectionTable::convertValue(const QString& value, const QString& type) const {
    if (type == "int") {
        return value.toInt();
    } else if (type == "double") {
        return value.toDouble();
    } else if (type == "bool") {
        return (value == "true");
    }
    return value;
}

// Slot implementations
void CollectionTable::onCellChanged(int row, int col) {
    QTableWidgetItem* item = table->item(row, col);
    if (item) {
        setCell(row, col, item->text());
    }
}

void CollectionTable::onCellDoubleClicked(int row, int col) {
    emit cellDoubleClicked(row, col);
}

void CollectionTable::onHeaderClicked(int col) {
    // Sort by column
    sort(col, Qt::AscendingOrder);
}

void CollectionTable::onContextMenu(const QPoint& pos) {
    contextMenu->exec(table->viewport()->mapToGlobal(pos));
}

void CollectionTable::onFilterChanged() {
    // Apply filters from filter editors
    applyFilters();
}

void CollectionTable::onSortAscending() {
    int col = table->currentColumn();
    if (col >= 0) {
        sort(col, Qt::AscendingOrder);
    }
}

void CollectionTable::onSortDescending() {
    int col = table->currentColumn();
    if (col >= 0) {
        sort(col, Qt::DescendingOrder);
    }
}

void CollectionTable::onCopy() {
    copySelectionToClipboard();
}

void CollectionTable::onPaste() {
    // Would implement paste from clipboard
}

void CollectionTable::onDeleteRow() {
    int row = table->currentRow();
    if (row >= 0) {
        removeRow(row);
    }
}

void CollectionTable::onDeleteColumn() {
    int col = table->currentColumn();
    if (col >= 0) {
        removeColumn(col);
    }
}

void CollectionTable::onAddRow() {
    QVector<QString> newRow(headers.size(), "");
    addRow(newRow);
}

void CollectionTable::onAddColumn() {
    addColumn("New Column");
}

void CollectionTable::onSaveCSV() {
    QString path = QFileDialog::getSaveFileName(this, "Save CSV", "", "CSV Files (*.csv)");
    if (!path.isEmpty()) saveAsCSV(path);
}

void CollectionTable::onLoadCSV() {
    QString path = QFileDialog::getOpenFileName(this, "Load CSV", "", "CSV Files (*.csv)");
    if (!path.isEmpty()) loadFromCSV(path);
}

void CollectionTable::onExportJSON() {
    QString path = QFileDialog::getSaveFileName(this, "Export JSON", "", "JSON Files (*.json)");
    if (!path.isEmpty()) saveAsJSON(path);
}

void CollectionTable::onFind() {
    // Would open find dialog
}

void CollectionTable::onReplace() {
    // Would open replace dialog
}

} // namespace proxima