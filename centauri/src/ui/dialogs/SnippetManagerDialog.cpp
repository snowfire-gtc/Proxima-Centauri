#include "SnippetManagerDialog.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>

namespace centauri::ui {

SnippetManagerDialog::SnippetManagerDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Менеджер сниппетов");
    setMinimumSize(800, 600);
    resize(900, 650);
    
    setupUI();
    populateCategories();
}

SnippetManagerDialog::~SnippetManagerDialog() = default;

void SnippetManagerDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Верхняя часть с поиском и фильтрацией
    auto* topLayout = new QHBoxLayout();
    
    topLayout->addWidget(new QLabel("Поиск:"));
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Поиск сниппетов...");
    connect(m_searchEdit, &QLineEdit::textChanged, 
            this, &SnippetManagerDialog::onSearchTextChanged);
    topLayout->addWidget(m_searchEdit);
    
    topLayout->addWidget(new QLabel("Категория:"));
    m_categoryCombo = new QComboBox();
    m_categoryCombo->addItem("Все категории", "");
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SnippetManagerDialog::onCategoryChanged);
    topLayout->addWidget(m_categoryCombo);
    
    layout->addLayout(topLayout);
    
    // Основная часть с двумя колонками
    auto* mainLayout = new QHBoxLayout();
    
    // Левая колонка - список сниппетов
    auto* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel("Сниппеты:"));
    
    m_snippetList = new QListWidget();
    connect(m_snippetList, &QListWidget::itemSelectionChanged,
            this, &SnippetManagerDialog::onSelectionChanged);
    leftLayout->addWidget(m_snippetList);
    
    mainLayout->addLayout(leftLayout, 1);
    
    // Правая колонка - редактирование
    auto* rightLayout = new QVBoxLayout();
    
    auto* editGroup = new QGroupBox("Редактирование сниппета");
    auto* editFormLayout = new QVBoxLayout();
    
    editFormLayout->addWidget(new QLabel("Название:"));
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("Например: for_loop");
    editFormLayout->addWidget(m_nameEdit);
    
    editFormLayout->addWidget(new QLabel("Категория:"));
    m_categoryEdit = new QComboBox();
    m_categoryEdit->setEditable(true);
    populateCategories();
    editFormLayout->addWidget(m_categoryEdit);
    
    editFormLayout->addWidget(new QLabel("Горячие клавиши:"));
    m_shortcutEdit = new QLineEdit();
    m_shortcutEdit->setPlaceholderText("Например: Ctrl+Shift+F");
    editFormLayout->addWidget(m_shortcutEdit);
    
    editFormLayout->addWidget(new QLabel("Содержимое:"));
    m_contentEdit = new QTextEdit();
    m_contentEdit->setFont(QFont("Consolas", 10));
    m_contentEdit->setPlaceholderText("Введите код сниппета...\nИспользуйте $1, $2 для мест подстановки.");
    editFormLayout->addWidget(m_contentEdit);
    
    editGroup->setLayout(editFormLayout);
    rightLayout->addLayout(editGroup);
    
    mainLayout->addLayout(rightLayout, 1);
    layout->addLayout(mainLayout);
    
    // Кнопки управления
    auto* buttonLayout = new QHBoxLayout();
    
    m_newButton = new QPushButton("Новый");
    connect(m_newButton, &QPushButton::clicked, this, &SnippetManagerDialog::onNewClicked);
    buttonLayout->addWidget(m_newButton);
    
    m_editButton = new QPushButton("Редактировать");
    connect(m_editButton, &QPushButton::clicked, this, &SnippetManagerDialog::onEditClicked);
    buttonLayout->addWidget(m_editButton);
    
    m_deleteButton = new QPushButton("Удалить");
    connect(m_deleteButton, &QPushButton::clicked, this, &SnippetManagerDialog::onDeleteClicked);
    buttonLayout->addWidget(m_deleteButton);
    
    buttonLayout->addStretch();
    
    m_importButton = new QPushButton("Импорт");
    connect(m_importButton, &QPushButton::clicked, this, &SnippetManagerDialog::onImportClicked);
    buttonLayout->addWidget(m_importButton);
    
    m_exportButton = new QPushButton("Экспорт");
    connect(m_exportButton, &QPushButton::clicked, this, &SnippetManagerDialog::onExportClicked);
    buttonLayout->addWidget(m_exportButton);
    
    m_insertButton = new QPushButton("Вставить");
    connect(m_insertButton, &QPushButton::clicked, this, &SnippetManagerDialog::onInsertClicked);
    buttonLayout->addWidget(m_insertButton);
    
    buttonLayout->addStretch();
    
    m_closeButton = new QPushButton("Закрыть");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_closeButton);
    
    layout->addLayout(buttonLayout);
}

void SnippetManagerDialog::populateCategories() {
    QStringList categories = {
        "General",
        "Loops",
        "Functions",
        "Classes",
        "Comments",
        "Templates",
        "Tests",
        "Debug"
    };
    
    for (const auto& cat : categories) {
        m_categoryCombo->addItem(cat);
        m_categoryEdit->addItem(cat);
    }
}

bool SnippetManagerDialog::loadSnippets(const QString& filePath) {
    QString path = filePath;
    if (path.isEmpty()) {
        path = QFileDialog::getOpenFileName(
            this,
            "Импортировать сниппеты",
            "",
            "JSON файлы (*.json);;Все файлы (*)"
        );
        
        if (path.isEmpty()) return false;
    }
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        QMessageBox::critical(this, "Ошибка", "Неверный формат JSON");
        return false;
    }
    
    m_snippets.clear();
    QJsonArray array = doc.array();
    
    for (const auto& value : array) {
        QJsonObject obj = value.toObject();
        Snippet snippet;
        snippet.name = obj["name"].toString();
        snippet.content = obj["content"].toString();
        snippet.category = obj["category"].toString("General");
        snippet.shortcut = obj["shortcut"].toString();
        m_snippets.append(snippet);
    }
    
    m_currentFile = path;
    updateSnippetList();
    emit snippetsLoaded();
    
    return true;
}

bool SnippetManagerDialog::saveSnippets(const QString& filePath) {
    QString path = filePath;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(
            this,
            "Экспортировать сниппеты",
            "snippets.json",
            "JSON файлы (*.json);;Все файлы (*)"
        );
        
        if (path.isEmpty()) return false;
    }
    
    QJsonArray array;
    for (const auto& snippet : m_snippets) {
        QJsonObject obj;
        obj["name"] = snippet.name;
        obj["content"] = snippet.content;
        obj["category"] = snippet.category;
        obj["shortcut"] = snippet.shortcut;
        array.append(obj);
    }
    
    QJsonDocument doc(array);
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл");
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    m_currentFile = path;
    emit snippetsSaved();
    
    return true;
}

void SnippetManagerDialog::addSnippet(const QString& name, const QString& content,
                                       const QString& category, const QString& shortcut) {
    Snippet snippet;
    snippet.name = name;
    snippet.content = content;
    snippet.category = category;
    snippet.shortcut = shortcut;
    
    m_snippets.append(snippet);
    updateSnippetList();
}

QString SnippetManagerDialog::getSelectedSnippet() const {
    int index = m_snippetList->currentRow();
    if (index >= 0 && index < m_snippets.size()) {
        return m_snippets[index].content;
    }
    return QString();
}

void SnippetManagerDialog::onNewClicked() {
    clearInputs();
    m_nameEdit->setFocus();
}

void SnippetManagerDialog::onEditClicked() {
    int index = m_snippetList->currentRow();
    if (index < 0) {
        QMessageBox::warning(this, "Предупреждение", "Выберите сниппет для редактирования");
        return;
    }
    
    const Snippet& snippet = m_snippets[index];
    m_nameEdit->setText(snippet.name);
    m_contentEdit->setText(snippet.content);
    
    int catIndex = m_categoryEdit->findText(snippet.category);
    if (catIndex >= 0) {
        m_categoryEdit->setCurrentIndex(catIndex);
    } else {
        m_categoryEdit->setCurrentText(snippet.category);
    }
    
    m_shortcutEdit->setText(snippet.shortcut);
}

void SnippetManagerDialog::onDeleteClicked() {
    int index = m_snippetList->currentRow();
    if (index < 0) {
        QMessageBox::warning(this, "Предупреждение", "Выберите сниппет для удаления");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Подтверждение",
        "Вы уверены, что хотите удалить этот сниппет?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_snippets.removeAt(index);
        updateSnippetList();
        clearInputs();
    }
}

void SnippetManagerDialog::onImportClicked() {
    loadSnippets();
}

void SnippetManagerDialog::onExportClicked() {
    saveSnippets();
}

void SnippetManagerDialog::onInsertClicked() {
    QString content = getSelectedSnippet();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Выберите сниппет для вставки");
        return;
    }
    
    emit snippetInserted(content);
    accept();
}

void SnippetManagerDialog::onSelectionChanged() {
    onEditClicked();
}

void SnippetManagerDialog::onSearchTextChanged(const QString& text) {
    updateSnippetList();
}

void SnippetManagerDialog::onCategoryChanged(const QString& /*category*/) {
    updateSnippetList();
}

void SnippetManagerDialog::updateSnippetList() {
    m_snippetList->clear();
    
    QString searchText = m_searchEdit->text().toLower();
    QString filterCategory = m_categoryCombo->currentData().toString();
    
    for (const auto& snippet : m_snippets) {
        bool matchesSearch = searchText.isEmpty() || 
                            snippet.name.toLower().contains(searchText) ||
                            snippet.content.toLower().contains(searchText);
        
        bool matchesCategory = filterCategory.isEmpty() || 
                              snippet.category == filterCategory;
        
        if (matchesSearch && matchesCategory) {
            auto* item = new QListWidgetItem(snippet.name);
            item->setData(Qt::UserRole, snippet.category);
            item->setToolTip(QString("Категория: %1\n%2").arg(snippet.category, snippet.content.left(100)));
            m_snippetList->addItem(item);
        }
    }
}

void SnippetManagerDialog::clearInputs() {
    m_nameEdit->clear();
    m_contentEdit->clear();
    m_categoryEdit->setCurrentIndex(0);
    m_shortcutEdit->clear();
}

bool SnippetManagerDialog::validateInputs() {
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Введите название сниппета");
        return false;
    }
    
    if (m_contentEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Введите содержимое сниппета");
        return false;
    }
    
    return true;
}

int SnippetManagerDialog::findSnippetByName(const QString& name) {
    for (int i = 0; i < m_snippets.size(); ++i) {
        if (m_snippets[i].name == name) {
            return i;
        }
    }
    return -1;
}

} // namespace centauri::ui
