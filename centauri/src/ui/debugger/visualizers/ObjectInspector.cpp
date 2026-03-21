#include "ObjectInspector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>
#include <QMenu>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include "utils/Logger.h"

namespace proxima {

ObjectInspector::ObjectInspector(QWidget *parent)
    : QWidget(parent)
    , currentObject(nullptr)
    , readOnly(false) {
    
    setupUI();
    setupToolbar();
    setupContextMenu();
}

ObjectInspector::~ObjectInspector() {}

void ObjectInspector::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(toolbar);
    
    // Object tree
    objectTree = new QTreeWidget(this);
    objectTree->setColumnCount(3);
    objectTree->setHeaderLabels(QStringList() << "Property" << "Type" << "Value");
    objectTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    objectTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    objectTree->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    objectTree->setAlternatingRowColors(true);
    objectTree->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(objectTree, &QTreeWidget::itemDoubleClicked, this, &ObjectInspector::onItemDoubleClicked);
    connect(objectTree, &QTreeWidget::customContextMenuRequested, this, &ObjectInspector::onContextMenu);
    connect(objectTree, &QTreeWidget::itemChanged, this, &ObjectInspector::onItemChanged);
    
    mainLayout->addWidget(objectTree, 1);
    
    // Bottom panel with actions
    QWidget* bottomPanel = new QWidget(this);
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomPanel);
    bottomLayout->setContentsMargins(5, 5, 5, 5);
    
    refreshButton = new QPushButton("Refresh", this);
    connect(refreshButton, &QPushButton::clicked, this, &ObjectInspector::onRefresh);
    bottomLayout->addWidget(refreshButton);
    
    expandAllButton = new QPushButton("Expand All", this);
    connect(expandAllButton, &QPushButton::clicked, objectTree, &QTreeWidget::expandAll);
    bottomLayout->addWidget(expandAllButton);
    
    collapseAllButton = new QPushButton("Collapse All", this);
    connect(collapseAllButton, &QPushButton::clicked, objectTree, &QTreeWidget::collapseAll);
    bottomLayout->addWidget(collapseAllButton);
    
    bottomLayout->addStretch();
    
    copyButton = new QPushButton("Copy", this);
    connect(copyButton, &QPushButton::clicked, this, &ObjectInspector::onCopy);
    bottomLayout->addWidget(copyButton);
    
    exportButton = new QPushButton("Export", this);
    connect(exportButton, &QPushButton::clicked, this, &ObjectInspector::onExport);
    bottomLayout->addWidget(exportButton);
    
    mainLayout->addWidget(bottomPanel);
}

void ObjectInspector::setupToolbar() {
    toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    
    viewModeCombo = new QComboBox(this);
    viewModeCombo->addItem("Tree View", TreeView);
    viewModeCombo->addItem("Table View", TableView);
    viewModeCombo->addItem("JSON View", JSONView);
    connect(viewModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ObjectInspector::onViewModeChanged);
    toolbar->addWidget(viewModeCombo);
    
    toolbar->addSeparator();
    
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Filter properties...");
    filterEdit->setMaximumWidth(200);
    connect(filterEdit, &QLineEdit::textChanged, this, &ObjectInspector::onFilterChanged);
    toolbar->addWidget(filterEdit);
}

void ObjectInspector::setupContextMenu() {
    contextMenu = new QMenu(this);
    
    editValueAction = contextMenu->addAction("Edit Value");
    connect(editValueAction, &QAction::triggered, this, &ObjectInspector::onEditValue);
    
    copyValueAction = contextMenu->addAction("Copy Value");
    connect(copyValueAction, &QAction::triggered, this, &ObjectInspector::onCopyValue);
    
    copyNameAction = contextMenu->addAction("Copy Name");
    connect(copyNameAction, &QAction::triggered, this, &ObjectInspector::onCopyName);
    
    contextMenu->addSeparator();
    
    expandAction = contextMenu->addAction("Expand");
    connect(expandAction, &QAction::triggered, this, [this]() {
        QTreeWidgetItem* item = objectTree->currentItem();
        if (item) item->setExpanded(true);
    });
    
    collapseAction = contextMenu->addAction("Collapse");
    connect(collapseAction, &QAction::triggered, this, [this]() {
        QTreeWidgetItem* item = objectTree->currentItem();
        if (item) item->setExpanded(false);
    });
    
    contextMenu->addSeparator();
    
    visualizeAction = contextMenu->addAction("Visualize");
    connect(visualizeAction, &QAction::triggered, this, &ObjectInspector::onVisualize);
}

void ObjectInspector::setObject(const ObjectInfo& object) {
    currentObject = object;
    objectTree->clear();
    
    if (object.type == "class" || object.type == "interface") {
        // Add class name as root
        QTreeWidgetItem* root = new QTreeWidgetItem(objectTree);
        root->setText(0, object.name);
        root->setText(1, object.type);
        root->setText(2, "");
        root->setExpanded(true);
        
        // Add fields
        if (!object.fields.isEmpty()) {
            QTreeWidgetItem* fieldsItem = new QTreeWidgetItem(root);
            fieldsItem->setText(0, "Fields");
            fieldsItem->setText(1, "section");
            
            for (const ObjectField& field : object.fields) {
                QTreeWidgetItem* fieldItem = new QTreeWidgetItem(fieldsItem);
                fieldItem->setText(0, field.name);
                fieldItem->setText(1, field.type);
                fieldItem->setText(2, formatValue(field.value, field.type));
                fieldItem->setFlags(fieldItem->flags() | Qt::ItemIsEditable);
                
                // Store field info
                fieldItem->setData(0, Qt::UserRole, QVariant::fromValue(field));
                
                // If field is complex, add children
                if (isComplexType(field.type)) {
                    fieldItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                }
            }
            
            fieldsItem->setExpanded(true);
        }
        
        // Add methods
        if (!object.methods.isEmpty()) {
            QTreeWidgetItem* methodsItem = new QTreeWidgetItem(root);
            methodsItem->setText(0, "Methods");
            methodsItem->setText(1, "section");
            
            for (const ObjectMethod& method : object.methods) {
                QTreeWidgetItem* methodItem = new QTreeWidgetItem(methodsItem);
                methodItem->setText(0, method.name);
                methodItem->setText(1, "method");
                methodItem->setText(2, method.returnType);
            }
            
            methodsItem->setExpanded(false);
        }
    } else if (object.type == "vector") {
        // Vector visualization
        QTreeWidgetItem* root = new QTreeWidgetItem(objectTree);
        root->setText(0, object.name);
        root->setText(1, object.type);
        root->setText(2, QString("Size: %1").arg(object.size));
        
        // Add elements
        for (int i = 0; i < object.elements.size() && i < 100; i++) {
            QTreeWidgetItem* elemItem = new QTreeWidgetItem(root);
            elemItem->setText(0, QString("[%1]").arg(i + 1));
            elemItem->setText(1, object.elementType);
            elemItem->setText(2, formatValue(object.elements[i], object.elementType));
        }
        
        if (object.elements.size() > 100) {
            QTreeWidgetItem* moreItem = new QTreeWidgetItem(root);
            moreItem->setText(0, QString("... and %1 more elements").arg(object.elements.size() - 100));
            moreItem->setText(1, "info");
        }
    } else if (object.type == "matrix") {
        // Matrix visualization
        QTreeWidgetItem* root = new QTreeWidgetItem(objectTree);
        root->setText(0, object.name);
        root->setText(1, object.type);
        root->setText(2, QString("Size: %1×%2").arg(object.rows).arg(object.cols));
        
        // Show dimensions
        QTreeWidgetItem* dimsItem = new QTreeWidgetItem(root);
        dimsItem->setText(0, "Dimensions");
        dimsItem->setText(1, "info");
        dimsItem->setText(2, QString("%1 rows, %2 columns").arg(object.rows).arg(object.cols));
    } else if (object.type == "collection") {
        // Collection visualization
        QTreeWidgetItem* root = new QTreeWidgetItem(objectTree);
        root->setText(0, object.name);
        root->setText(1, object.type);
        root->setText(2, QString("Rows: %1, Columns: %2").arg(object.rows).arg(object.cols));
        
        // Add headers
        if (!object.headers.isEmpty()) {
            QTreeWidgetItem* headersItem = new QTreeWidgetItem(root);
            headersItem->setText(0, "Headers");
            headersItem->setText(1, "section");
            
            for (const QString& header : object.headers) {
                QTreeWidgetItem* headerItem = new QTreeWidgetItem(headersItem);
                headerItem->setText(0, header);
                headerItem->setText(1, "header");
            }
        }
    } else {
        // Simple type
        QTreeWidgetItem* root = new QTreeWidgetItem(objectTree);
        root->setText(0, object.name);
        root->setText(1, object.type);
        root->setText(2, formatValue(object.value, object.type));
        root->setFlags(root->flags() | Qt::ItemIsEditable);
    }
    
    objectTree->expandAll();
    
    emit objectLoaded(object.name, object.type);
}

void ObjectInspector::clear() {
    currentObject = ObjectInfo();
    objectTree->clear();
}

void ObjectInspector::refresh() {
    if (currentObject.name.isEmpty()) return;
    
    emit refreshRequested(currentObject.name);
}

QString ObjectInspector::formatValue(const QString& value, const QString& type) const {
    if (value.isEmpty()) return "<empty>";
    
    if (type == "string") {
        return "\"" + value + "\"";
    } else if (type == "bool") {
        return (value == "true" || value == "1") ? "true" : "false";
    } else if (type == "double" || type == "single") {
        bool ok;
        double num = value.toDouble(&ok);
        if (ok) {
            if (std::abs(num) < 0.001 || std::abs(num) > 1000000) {
                return QString::number(num, 'e', 4);
            }
            return QString::number(num, 'f', 6);
        }
        return value;
    } else if (type.startsWith("vector") || type.startsWith("matrix") || type.startsWith("layer")) {
        return QString("<%1>").arg(type);
    }
    
    return value;
}

bool ObjectInspector::isComplexType(const QString& type) const {
    return type.startsWith("vector") || 
           type.startsWith("matrix") || 
           type.startsWith("layer") ||
           type.startsWith("collection") ||
           type == "class" ||
           type == "interface";
}

void ObjectInspector::onViewModeChanged(int index) {
    ViewMode mode = static_cast<ViewMode>(viewModeCombo->itemData(index).toInt());
    
    switch (mode) {
        case TreeView:
            objectTree->setVisible(true);
            // Hide table view if implemented
            break;
        case TableView:
            // Show table view if implemented
            break;
        case JSONView:
            // Show JSON view if implemented
            break;
    }
}

void ObjectInspector::onFilterChanged(const QString& text) {
    // Filter tree items
    filterTreeItems(objectTree->invisibleRootItem(), text);
}

void ObjectInspector::filterTreeItems(QTreeWidgetItem* item, const QString& filter) {
    bool hasVisibleChild = false;
    
    for (int i = 0; i < item->childCount(); i++) {
        QTreeWidgetItem* child = item->child(i);
        filterTreeItems(child, filter);
        
        if (child->isHidden() == false) {
            hasVisibleChild = true;
        }
    }
    
    if (!filter.isEmpty()) {
        bool matches = item->text(0).contains(filter, Qt::CaseInsensitive) ||
                      item->text(1).contains(filter, Qt::CaseInsensitive) ||
                      item->text(2).contains(filter, Qt::CaseInsensitive);
        
        item->setHidden(!(matches || hasVisibleChild));
    } else {
        item->setHidden(false);
    }
}

void ObjectInspector::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    if (column == 2 && !readOnly) {
        objectTree->editItem(item, column);
    } else if (item->childCount() > 0) {
        item->setExpanded(!item->isExpanded());
    } else {
        // Try to visualize complex type
        ObjectField field = item->data(0, Qt::UserRole).value<ObjectField>();
        if (!field.name.isEmpty() && isComplexType(field.type)) {
            emit visualizeRequested(field.name, field.type);
        }
    }
}

void ObjectInspector::onItemChanged(QTreeWidgetItem* item, int column) {
    if (column == 2 && !readOnly) {
        ObjectField field = item->data(0, Qt::UserRole).value<ObjectField>();
        if (!field.name.isEmpty()) {
            emit valueChanged(field.name, item->text(2));
        }
    }
}

void ObjectInspector::onContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = objectTree->itemAt(pos);
    if (item) {
        objectTree->setCurrentItem(item);
        
        bool isComplex = isComplexType(item->text(1));
        visualizeAction->setVisible(isComplex);
        
        contextMenu->exec(objectTree->viewport()->mapToGlobal(pos));
    }
}

void ObjectInspector::onEditValue() {
    QTreeWidgetItem* item = objectTree->currentItem();
    if (item) {
        objectTree->editItem(item, 2);
    }
}

void ObjectInspector::onCopyValue() {
    QTreeWidgetItem* item = objectTree->currentItem();
    if (item) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(item->text(2));
    }
}

void ObjectInspector::onCopyName() {
    QTreeWidgetItem* item = objectTree->currentItem();
    if (item) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(item->text(0));
    }
}

void ObjectInspector::onVisualize() {
    QTreeWidgetItem* item = objectTree->currentItem();
    if (item) {
        ObjectField field = item->data(0, Qt::UserRole).value<ObjectField>();
        if (!field.name.isEmpty()) {
            emit visualizeRequested(field.name, field.type);
        }
    }
}

void ObjectInspector::onRefresh() {
    refresh();
}

void ObjectInspector::onCopy() {
    // Copy entire object as JSON
    QString json = toJson(currentObject);
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(json);
}

void ObjectInspector::onExport() {
    QString path = QFileDialog::getSaveFileName(this, "Export Object", "", 
                         "JSON Files (*.json);;All Files (*)");
    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(toJson(currentObject).toUtf8());
            file.close();
            LOG_INFO("Object exported: " + path.toStdString());
        }
    }
}

QString ObjectInspector::toJson(const ObjectInfo& object) const {
    QJsonObject obj;
    obj["name"] = object.name;
    obj["type"] = object.type;
    
    QJsonArray fields;
    for (const ObjectField& field : object.fields) {
        QJsonObject fieldObj;
        fieldObj["name"] = field.name;
        fieldObj["type"] = field.type;
        fieldObj["value"] = field.value;
        fields.append(fieldObj);
    }
    obj["fields"] = fields;
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Indented);
}

} // namespace proxima