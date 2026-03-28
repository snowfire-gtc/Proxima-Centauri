#include "FoldingSettingsDialog.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

namespace centauri::ui {

FoldingSettingsDialog::FoldingSettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Настройки сворачивания кода");
    setMinimumSize(500, 600);
    resize(600, 650);
    
    setupUI();
    loadSettings();
}

FoldingSettingsDialog::~FoldingSettingsDialog() = default;

void FoldingSettingsDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Группа основных настроек
    auto* mainGroup = new QGroupBox("Основные настройки");
    auto* mainLayout = new QVBoxLayout();
    
    m_enableFolding = new QCheckBox("Включить сворачивание кода");
    m_enableFolding->setChecked(true);
    connect(m_enableFolding, &QCheckBox::stateChanged, this, &FoldingSettingsDialog::onPreviewChanged);
    mainLayout->addWidget(m_enableFolding);
    
    m_showIcons = new QCheckBox("Показывать иконки сворачивания");
    m_showIcons->setChecked(true);
    connect(m_showIcons, &QCheckBox::stateChanged, this, &FoldingSettingsDialog::onPreviewChanged);
    mainLayout->addWidget(m_showIcons);
    
    mainGroup->setLayout(mainLayout);
    layout->addWidget(mainGroup);
    
    // Группа автоматического сворачивания
    auto* autoGroup = new QGroupBox("Автоматическое сворачивание");
    auto* autoLayout = new QVBoxLayout();
    
    m_autoFoldFunctions = new QCheckBox("Сворачивать функции при загрузке");
    connect(m_autoFoldFunctions, &QCheckBox::stateChanged, this, &FoldingSettingsDialog::onPreviewChanged);
    autoLayout->addWidget(m_autoFoldFunctions);
    
    m_autoFoldClasses = new QCheckBox("Сворачивать классы при загрузке");
    connect(m_autoFoldClasses, &QCheckBox::stateChanged, this, &FoldingSettingsDialog::onPreviewChanged);
    autoLayout->addWidget(m_autoFoldClasses);
    
    m_autoFoldComments = new QCheckBox("Сворачивать многострочные комментарии");
    connect(m_autoFoldComments, &QCheckBox::stateChanged, this, &FoldingSettingsDialog::onPreviewChanged);
    autoLayout->addWidget(m_autoFoldComments);
    
    autoGroup->setLayout(autoLayout);
    layout->addWidget(autoGroup);
    
    // Группа дополнительных настроек
    auto* extraGroup = new QGroupBox("Дополнительные настройки");
    auto* extraLayout = new QVBoxLayout();
    
    // Минимальное количество строк для сворачивания
    auto* minLinesLayout = new QHBoxLayout();
    minLinesLayout->addWidget(new QLabel("Мин. строк для сворачивания:"));
    m_minLines = new QSpinBox();
    m_minLines->setRange(3, 100);
    m_minLines->setValue(5);
    connect(m_minLines, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &FoldingSettingsDialog::onPreviewChanged);
    minLinesLayout->addWidget(m_minLines);
    minLinesLayout->addStretch();
    extraLayout->addLayout(minLinesLayout);
    
    // Цвет индикатора сворачивания
    auto* colorLayout = new QHBoxLayout();
    colorLayout->addWidget(new QLabel("Цвет индикатора:"));
    m_colorCombo = new QComboBox();
    populateColorCombo();
    connect(m_colorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FoldingSettingsDialog::onPreviewChanged);
    colorLayout->addWidget(m_colorCombo);
    colorLayout->addStretch();
    extraLayout->addLayout(colorLayout);
    
    extraGroup->setLayout(extraLayout);
    layout->addWidget(extraGroup);
    
    // Предпросмотр
    auto* previewGroup = new QGroupBox("Предпросмотр");
    auto* previewLayout = new QVBoxLayout();
    
    m_previewText = new QTextEdit();
    m_previewText->setReadOnly(true);
    m_previewText->setFont(QFont("Consolas", 10));
    m_previewText->setMaximumHeight(200);
    setupPreview();
    previewLayout->addWidget(m_previewText);
    
    previewGroup->setLayout(previewLayout);
    layout->addWidget(previewGroup);
    
    // Кнопки управления
    auto* buttonLayout = new QHBoxLayout();
    
    m_loadButton = new QPushButton("Загрузить");
    connect(m_loadButton, &QPushButton::clicked, this, &FoldingSettingsDialog::onLoadClicked);
    buttonLayout->addWidget(m_loadButton);
    
    m_saveButton = new QPushButton("Сохранить");
    connect(m_saveButton, &QPushButton::clicked, this, &FoldingSettingsDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveButton);
    
    m_applyButton = new QPushButton("Применить");
    connect(m_applyButton, &QPushButton::clicked, this, &FoldingSettingsDialog::onApplyClicked);
    buttonLayout->addWidget(m_applyButton);
    
    m_resetButton = new QPushButton("Сбросить");
    connect(m_resetButton, &QPushButton::clicked, this, &FoldingSettingsDialog::onResetClicked);
    buttonLayout->addWidget(m_resetButton);
    
    buttonLayout->addStretch();
    
    m_closeButton = new QPushButton("Закрыть");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(m_closeButton);
    
    layout->addLayout(buttonLayout);
}

void FoldingSettingsDialog::setupPreview() {
    QString previewCode = 
        "// Пример кода для предпросмотра\n"
        "function calculateSum(a, b) {\n"
        "    let result = 0;\n"
        "    result = a + b;\n"
        "    return result;\n"
        "}\n"
        "\n"
        "class Calculator {\n"
        "    constructor() {\n"
        "        this.value = 0;\n"
        "    }\n"
        "    \n"
        "    add(x) {\n"
        "        this.value += x;\n"
        "    }\n"
        "}\n"
        "\n"
        "/* Это многострочный\n"
        "   комментарий */\n";
    
    m_previewText->setText(previewCode);
    updatePreview();
}

void FoldingSettingsDialog::populateColorCombo() {
    m_colorCombo->addItem("Серый", "#808080");
    m_colorCombo->addItem("Синий", "#0000FF");
    m_colorCombo->addItem("Зелёный", "#008000");
    m_colorCombo->addItem("Красный", "#FF0000");
    m_colorCombo->addItem("Оранжевый", "#FFA500");
    m_colorCombo->setCurrentIndex(0);
}

void FoldingSettingsDialog::loadSettings(const QString& settingsPath) {
    QString path = settingsPath.isEmpty() ? 
                   "centauri/folding.ini" : settingsPath;
    
    QSettings settings(path, QSettings::IniFormat);
    
    m_enableFolding->setChecked(settings.value("folding/enabled", true).toBool());
    m_showIcons->setChecked(settings.value("folding/showIcons", true).toBool());
    m_autoFoldFunctions->setChecked(settings.value("folding/autoFoldFunctions", false).toBool());
    m_autoFoldClasses->setChecked(settings.value("folding/autoFoldClasses", false).toBool());
    m_autoFoldComments->setChecked(settings.value("folding/autoFoldComments", true).toBool());
    m_minLines->setValue(settings.value("folding/minLines", 5).toInt());
    
    QString color = settings.value("folding/color", "#808080").toString();
    int index = m_colorCombo->findData(color);
    if (index >= 0) {
        m_colorCombo->setCurrentIndex(index);
    }
    
    m_settingsPath = path;
    updatePreview();
}

bool FoldingSettingsDialog::saveSettings(const QString& settingsPath) {
    QString path = settingsPath.isEmpty() ? m_settingsPath : settingsPath;
    if (path.isEmpty()) {
        path = QFileDialog::getSaveFileName(
            this,
            "Сохранить настройки сворачивания",
            "folding.ini",
            "INI файлы (*.ini);;Все файлы (*)"
        );
        
        if (path.isEmpty()) return false;
    }
    
    QSettings settings(path, QSettings::IniFormat);
    
    settings.setValue("folding/enabled", m_enableFolding->isChecked());
    settings.setValue("folding/showIcons", m_showIcons->isChecked());
    settings.setValue("folding/autoFoldFunctions", m_autoFoldFunctions->isChecked());
    settings.setValue("folding/autoFoldClasses", m_autoFoldClasses->isChecked());
    settings.setValue("folding/autoFoldComments", m_autoFoldComments->isChecked());
    settings.setValue("folding/minLines", m_minLines->value());
    settings.setValue("folding/color", m_colorCombo->currentData().toString());
    
    m_settingsPath = path;
    return true;
}

void FoldingSettingsDialog::applySettings() {
    emit settingsApplied();
    emit settingsChanged();
}

void FoldingSettingsDialog::onApplyClicked() {
    applySettings();
}

void FoldingSettingsDialog::onSaveClicked() {
    if (saveSettings()) {
        applySettings();
    }
}

void FoldingSettingsDialog::onLoadClicked() {
    QString path = QFileDialog::getOpenFileName(
        this,
        "Загрузить настройки сворачивания",
        "",
        "INI файлы (*.ini);;Все файлы (*)"
    );
    
    if (!path.isEmpty()) {
        loadSettings(path);
    }
}

void FoldingSettingsDialog::onResetClicked() {
    m_enableFolding->setChecked(true);
    m_showIcons->setChecked(true);
    m_autoFoldFunctions->setChecked(false);
    m_autoFoldClasses->setChecked(false);
    m_autoFoldComments->setChecked(true);
    m_minLines->setValue(5);
    m_colorCombo->setCurrentIndex(0);
    
    updatePreview();
}

void FoldingSettingsDialog::onPreviewChanged() {
    updatePreview();
}

void FoldingSettingsDialog::updatePreview() {
    // Визуальное обновление предпросмотра на основе настроек
    bool enabled = m_enableFolding->isChecked();
    bool showIcons = m_showIcons->isChecked();
    
    if (!enabled) {
        m_previewText->setStyleSheet("background-color: #f0f0f0;");
    } else {
        QString bgColor = showIcons ? "#ffffff" : "#fafafa";
        m_previewText->setStyleSheet(QString("background-color: %1;").arg(bgColor));
    }
}

} // namespace centauri::ui
