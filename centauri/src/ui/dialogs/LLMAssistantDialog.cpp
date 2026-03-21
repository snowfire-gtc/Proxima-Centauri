#include "LLMAssistantDialog.h"
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QToolTip>
#include <QMenu>
#include <QCompleter>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QDiff>

namespace proxima {

// ============================================================================
// ModificationItemWidget Implementation
// ============================================================================

ModificationItemWidget::ModificationItemWidget(const CodeModification& mod, QWidget *parent)
    : QWidget(parent)
    , modification(mod) {
    
    // Настройка шрифта
    consoleFont = QFont("Consolas", 9);
    consoleFont.setStyleHint(QFont::Monospace);
    
    // Инициализация цветов категорий
    categoryColors["optimization"] = QColor(78, 201, 176);    // #4EC9B0
    categoryColors["bugfix"] = QColor(244, 71, 71);           // #F44747
    categoryColors["refactoring"] = QColor(86, 156, 214);     // #569CD6
    categoryColors["documentation"] = QColor(106, 153, 85);   // #6A9955
    categoryColors["style"] = QColor(197, 134, 192);          // #C586C0
    categoryColors["security"] = QColor(206, 167, 0);         // #CEA700
    categoryColors["other"] = QColor(150, 150, 150);          // #969696
    
    setupUI();
    setupConnections();
    
    // Установка начальных значений
    setModification(mod);
}

ModificationItemWidget::~ModificationItemWidget() {
}

void ModificationItemWidget::setupUI() {
    // Главная компоновка
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);
    
    // Заголовок блока
    headerWidget = new QWidget(this);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(10);
    
    // Чекбокс принятия
    checkBox = new QCheckBox(this);
    checkBox->setToolTip("Принять это изменение");
    headerLayout->addWidget(checkBox);
    
    // Категория
    categoryLabel = new QLabel(this);
    categoryLabel->setFont(QFont("Segoe UI", 9, QFont::Bold));
    categoryLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(categoryLabel);
    
    // Уверенность
    confidenceLabel = new QLabel(this);
    confidenceLabel->setFont(QFont("Segoe UI", 9));
    confidenceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    headerLayout->addWidget(confidenceLabel);
    
    // Диапазон строк
    linesLabel = new QLabel(this);
    linesLabel->setFont(QFont("Segoe UI", 9));
    linesLabel->setStyleSheet("color: #808080;");
    headerLayout->addWidget(linesLabel);
    
    headerLayout->addStretch();
    
    mainLayout->addWidget(headerWidget);
    
    // Объяснение
    explanationEdit = new QTextEdit(this);
    explanationEdit->setReadOnly(true);
    explanationEdit->setMaximumHeight(60);
    explanationEdit->setFont(QFont("Segoe UI", 9));
    explanationEdit->setStyleSheet(
        "QTextEdit { "
        "  background-color: #2d2d2d; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 5px; "
        "}"
    );
    mainLayout->addWidget(explanationEdit);
    
    // Кнопки действий
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(5);
    
    viewOriginalBtn = new QPushButton("Оригинал", this);
    viewOriginalBtn->setToolTip("Просмотреть оригинальный код");
    buttonLayout->addWidget(viewOriginalBtn);
    
    viewSuggestedBtn = new QPushButton("Предложение", this);
    viewSuggestedBtn->setToolTip("Просмотреть предлагаемый код");
    buttonLayout->addWidget(viewSuggestedBtn);
    
    showDiffBtn = new QPushButton("Различия", this);
    showDiffBtn->setToolTip("Показать различия между оригиналом и предложением");
    buttonLayout->addWidget(showDiffBtn);
    
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // Стиль виджета
    setStyleSheet(
        "QWidget { "
        "  background-color: #1e1e1e; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 5px; "
        "}"
        "QWidget:hover { "
        "  border: 1px solid #007acc; "
        "}"
        "QCheckBox { "
        "  color: #d4d4d4; "
        "  spacing: 5px; "
        "}"
        "QPushButton { "
        "  background-color: #2d2d2d; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 5px 10px; "
        "}"
        "QPushButton:hover { "
        "  background-color: #3e3e3e; "
        "  border: 1px solid #007acc; "
        "}"
    );
}

void ModificationItemWidget::setupConnections() {
    connect(checkBox, &QCheckBox::toggled, this, &ModificationItemWidget::onCheckBoxToggled);
    connect(viewOriginalBtn, &QPushButton::clicked, this, &ModificationItemWidget::onViewOriginalClicked);
    connect(viewSuggestedBtn, &QPushButton::clicked, this, &ModificationItemWidget::onViewSuggestedClicked);
    connect(showDiffBtn, &QPushButton::clicked, this, &ModificationItemWidget::onShowDiffClicked);
}

void ModificationItemWidget::setModification(const CodeModification& mod) {
    modification = mod;
    
    // Обновление UI
    checkBox->setChecked(mod.accepted);
    checkBox->setEnabled(!mod.applied);
    
    // Категория
    categoryLabel->setText(mod.category.isEmpty() ? "Другое" : mod.category);
    categoryLabel->setStyleSheet("color: " + getCategoryColor(mod.category).name() + ";");
    
    // Уверенность
    confidenceLabel->setText(formatConfidence(mod.confidence));
    confidenceLabel->setStyleSheet("color: " + getConfidenceColor(mod.confidence).name() + ";");
    confidenceLabel->setToolTip(getConfidenceTooltip(mod.confidence));
    
    // Диапазон строк
    linesLabel->setText(QString("Строки %1-%2").arg(mod.startLine).arg(mod.endLine));
    
    // Объяснение
    explanationEdit->setPlainText(mod.explanation);
    
    // Стиль для применённых изменений
    if (mod.applied) {
        setStyleSheet(
            "QWidget { "
            "  background-color: #1a3a1a; "
            "  border: 1px solid #4a7a4a; "
            "  border-radius: 5px; "
            "}"
        );
        checkBox->setEnabled(false);
    } else if (mod.accepted) {
        setStyleSheet(
            "QWidget { "
            "  background-color: #1e3a1e; "
            "  border: 1px solid #4a7a4a; "
            "  border-radius: 5px; "
            "}"
        );
    } else {
        setStyleSheet(
            "QWidget { "
            "  background-color: #1e1e1e; "
            "  border: 1px solid #3e3e3e; "
            "  border-radius: 5px; "
            "}"
            "QWidget:hover { "
            "  border: 1px solid #007acc; "
            "}"
        );
    }
}

void ModificationItemWidget::setAccepted(bool accepted) {
    checkBox->setChecked(accepted);
    modification.accepted = accepted;
    setModification(modification);
}

void ModificationItemWidget::highlightDifferences() {
    // В полной реализации - подсветка различий в коде
}

void ModificationItemWidget::showDiff() {
    emit showDiffRequested(modification.blockId);
}

void ModificationItemWidget::onCheckBoxToggled(bool checked) {
    modification.accepted = checked;
    emit acceptanceChanged(modification.blockId, checked);
    setModification(modification);
}

void ModificationItemWidget::onViewOriginalClicked() {
    emit viewOriginalRequested(modification.blockId);
}

void ModificationItemWidget::onViewSuggestedClicked() {
    emit viewSuggestedRequested(modification.blockId);
}

void ModificationItemWidget::onShowDiffClicked() {
    emit showDiffRequested(modification.blockId);
}

QString ModificationItemWidget::formatConfidence(double confidence) const {
    return QString("%1%").arg(static_cast<int>(confidence * 100));
}

QColor ModificationItemWidget::getCategoryColor(const QString& category) const {
    return categoryColors.value(category.toLower(), categoryColors["other"]);
}

QString ModificationItemWidget::getCategoryIcon(const QString& category) const {
    if (category == "optimization") return "⚡";
    if (category == "bugfix") return "🐛";
    if (category == "refactoring") return "♻️";
    if (category == "documentation") return "📝";
    if (category == "style") return "🎨";
    if (category == "security") return "🔒";
    return "📌";
}

// ============================================================================
// LLMAssistantDialog Implementation
// ============================================================================

LLMAssistantDialog::LLMAssistantDialog(QWidget *parent)
    : QDialog(parent)
    , llmService(nullptr)
    , selectionStart(0)
    , selectionEnd(0)
    , autoApply(false)
    , minConfidence(0.5)
    , isProcessing(false)
    , currentBlockId(0)
    , timeout(30000)
    , acceptedCount(0)
    , appliedCount(0)
    , averageConfidence(0.0) {
    
    // Путь к настройкам
    settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + 
                   "/llm_assistant_settings.json";
    
    setupUI();
    setupConnections();
    setupCategoryFilters();
    loadSettings();
    
    LOG_INFO("LLMAssistantDialog created");
}

LLMAssistantDialog::~LLMAssistantDialog() {
    saveSettings();
    LOG_INFO("LLMAssistantDialog destroyed");
}

void LLMAssistantDialog::setupUI() {
    setWindowTitle("LLM Assistant - Selective Code Modification");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // Главная компоновка
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Табы
    mainTabWidget = new QTabWidget(this);
    mainLayout->addWidget(mainTabWidget, 1);
    
    // Вкладка запроса
    setupRequestPanel();
    mainTabWidget->addTab(requestTab, "Запрос");
    
    // Вкладка предложений
    setupSuggestionsPanel();
    mainTabWidget->addTab(suggestionsTab, "Предложения");
    
    // Вкладка предпросмотра
    setupPreviewPanel();
    mainTabWidget->addTab(previewTab, "Предпросмотр");
    
    // Панель управления
    setupControlPanel();
    mainLayout->addWidget(controlPanel);
    
    // Прогресс и статус
    QHBoxLayout* statusLayout = new QHBoxLayout();
    
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);  // Indeterminate
    progressBar->setVisible(false);
    progressBar->setStyleSheet(
        "QProgressBar { "
        "  background-color: #2d2d2d; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  text-align: center; "
        "}"
        "QProgressBar::chunk { "
        "  background-color: #007acc; "
        "}"
    );
    statusLayout->addWidget(progressBar, 1);
    
    statusLabel = new QLabel("Готов к работе", this);
    statusLabel->setFont(QFont("Segoe UI", 9));
    statusLayout->addWidget(statusLabel);
    
    statisticsLabel = new QLabel(this);
    statisticsLabel->setFont(QFont("Segoe UI", 9));
    statisticsLabel->setStyleSheet("color: #808080;");
    statusLayout->addWidget(statisticsLabel);
    
    mainLayout->addLayout(statusLayout);
    
    // Таймаут таймер
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &LLMAssistantDialog::onTimeout);
}

void LLMAssistantDialog::setupRequestPanel() {
    requestTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(requestTab);
    layout->setSpacing(10);
    
    // Шаблон запроса
    QHBoxLayout* templateLayout = new QHBoxLayout();
    templateLayout->addWidget(new QLabel("Шаблон:", this));
    
    promptTemplateCombo = new QComboBox(this);
    promptTemplateCombo->addItem("Оптимизация кода", "optimize");
    promptTemplateCombo->addItem("Исправление ошибок", "fix_bugs");
    promptTemplateCombo->addItem("Добавление комментариев", "add_comments");
    promptTemplateCombo->addItem("Рефакторинг", "refactor");
    promptTemplateCombo->addItem("Добавление тестов", "add_tests");
    promptTemplateCombo->addItem("Улучшение читаемости", "improve_readability");
    promptTemplateCombo->addItem("Безопасность", "security_check");
    promptTemplateCombo->addItem("Пользовательский", "custom");
    
    connect(promptTemplateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, [this](int index) {
        if (index == promptTemplateCombo->count() - 1) {
            promptEdit->setPlaceholderText("Введите ваш запрос...");
        } else {
            QString templateText;
            switch (index) {
                case 0: templateText = "Оптимизируй этот код для повышения производительности. Учитывай возможность использования векторизации и параллельных вычислений."; break;
                case 1: templateText = "Найди и исправь потенциальные ошибки в этом коде. Обрати внимание на граничные случаи и обработку ошибок."; break;
                case 2: templateText = "Добавь подробные комментарии к этому коду, объясняющие его назначение и логику работы."; break;
                case 3: templateText = "Выполни рефакторинг этого кода для улучшения его структуры и читаемости, сохраняя функциональность."; break;
                case 4: templateText = "Создай комплексные тесты для этого кода, включая тесты на граничные случаи."; break;
                case 5: templateText = "Улучши читаемость этого кода, следуя лучшим практикам программирования."; break;
                case 6: templateText = "Проверь код на потенциальные уязвимости безопасности и предложи исправления."; break;
            }
            promptEdit->setPlainText(templateText);
        }
    });
    
    templateLayout->addWidget(promptTemplateCombo);
    templateLayout->addStretch();
    layout->addLayout(templateLayout);
    
    // Поле ввода запроса
    promptEdit = new QTextEdit(this);
    promptEdit->setPlaceholderText("Выберите шаблон или введите свой запрос...");
    promptEdit->setFont(QFont("Consolas", 10));
    promptEdit->setMaximumHeight(150);
    promptEdit->setStyleSheet(
        "QTextEdit { "
        "  background-color: #2d2d2d; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 5px; "
        "}"
        "QTextEdit:focus { "
        "  border: 1px solid #007acc; "
        "}"
    );
    layout->addWidget(promptEdit);
    
    // Опции
    QGroupBox* optionsGroup = new QGroupBox("Опции", this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    includeContextCheck = new QCheckBox("Включить контекст файла", this);
    includeContextCheck->setChecked(true);
    optionsLayout->addWidget(includeContextCheck);
    
    includeCommentsCheck = new QCheckBox("Включить существующие комментарии", this);
    includeCommentsCheck->setChecked(true);
    optionsLayout->addWidget(includeCommentsCheck);
    
    QHBoxLayout* confidenceLayout = new QHBoxLayout();
    confidenceLayout->addWidget(new QLabel("Минимальная уверенность:", this));
    
    confidenceSlider = new QSlider(Qt::Horizontal, this);
    confidenceSlider->setRange(0, 100);
    confidenceSlider->setValue(50);
    confidenceSlider->setToolTip("Минимальная уверенность LLM для отображения предложений");
    confidenceLayout->addWidget(confidenceSlider);
    
    confidenceLabel = new QLabel("50%", this);
    confidenceLabel->setFixedWidth(50);
    confidenceLayout->addWidget(confidenceLabel);
    
    connect(confidenceSlider, &QSlider::valueChanged, this, [this](int value) {
        confidenceLabel->setText(QString("%1%").arg(value));
        minConfidence = value / 100.0;
    });
    
    optionsLayout->addLayout(confidenceLayout);
    layout->addWidget(optionsGroup);
    
    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    requestButton = new QPushButton("Запросить помощь", this);
    requestButton->setIcon(QIcon(":/icons/llm.svg"));
    requestButton->setMinimumWidth(150);
    buttonLayout->addWidget(requestButton);
    
    cancelButton = new QPushButton("Отмена", this);
    cancelButton->setEnabled(false);
    cancelButton->setMinimumWidth(150);
    buttonLayout->addWidget(cancelButton);
    
    layout->addLayout(buttonLayout);
    layout->addStretch();
}

void LLMAssistantDialog::setupSuggestionsPanel() {
    suggestionsTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(suggestionsTab);
    layout->setSpacing(10);
    
    // Фильтры
    QGroupBox* filterGroup = new QGroupBox("Фильтры", this);
    QVBoxLayout* filterLayout = new QVBoxLayout(filterGroup);
    
    // Текстовый фильтр
    QHBoxLayout* textFilterLayout = new QHBoxLayout();
    textFilterLayout->addWidget(new QLabel("Поиск:", this));
    
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Фильтр по тексту...");
    connect(filterEdit, &QLineEdit::textChanged, this, &LLMAssistantDialog::onFilterChanged);
    textFilterLayout->addWidget(filterEdit);
    filterLayout->addLayout(textFilterLayout);
    
    // Фильтр по категории
    QHBoxLayout* categoryFilterLayout = new QHBoxLayout();
    categoryFilterLayout->addWidget(new QLabel("Категория:", this));
    
    categoryFilterCombo = new QComboBox(this);
    categoryFilterCombo->addItem("Все категории", "all");
    categoryFilterCombo->addItem("Оптимизация", "optimization");
    categoryFilterCombo->addItem("Исправление ошибок", "bugfix");
    categoryFilterCombo->addItem("Рефакторинг", "refactoring");
    categoryFilterCombo->addItem("Документация", "documentation");
    categoryFilterCombo->addItem("Стиль", "style");
    categoryFilterCombo->addItem("Безопасность", "security");
    categoryFilterCombo->addItem("Другое", "other");
    
    connect(categoryFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        onCategoryFilterChanged(categoryFilterCombo->itemData(index).toString());
    });
    
    categoryFilterLayout->addWidget(categoryFilterCombo);
    filterLayout->addLayout(categoryFilterLayout);
    
    layout->addWidget(filterGroup);
    
    // Список предложений
    modificationsScroll = new QScrollArea(this);
    modificationsScroll->setWidgetResizable(true);
    modificationsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    modificationsScroll->setStyleSheet(
        "QScrollArea { "
        "  background-color: #1e1e1e; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "}"
    );
    
    modificationsContainer = new QWidget();
    modificationsLayout = new QVBoxLayout(modificationsContainer);
    modificationsLayout->setSpacing(10);
    modificationsLayout->addStretch();
    
    modificationsScroll->setWidget(modificationsContainer);
    layout->addWidget(modificationsScroll, 1);
}

void LLMAssistantDialog::setupPreviewPanel() {
    previewTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(previewTab);
    layout->setSpacing(10);
    
    // Сплиттер для предпросмотра
    previewSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Оригинальный код
    QWidget* originalWidget = new QWidget(this);
    QVBoxLayout* originalLayout = new QVBoxLayout(originalWidget);
    originalLayout->addWidget(new QLabel("Оригинальный код:", this));
    
    originalPreview = new QTextEdit(this);
    originalPreview->setReadOnly(true);
    originalPreview->setFont(QFont("Consolas", 10));
    originalPreview->setStyleSheet(
        "QTextEdit { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "}"
    );
    originalLayout->addWidget(originalPreview);
    
    // Предлагаемый код
    QWidget* suggestedWidget = new QWidget(this);
    QVBoxLayout* suggestedLayout = new QVBoxLayout(suggestedWidget);
    suggestedLayout->addWidget(new QLabel("Предлагаемый код:", this));
    
    suggestedPreview = new QTextEdit(this);
    suggestedPreview->setReadOnly(true);
    suggestedPreview->setFont(QFont("Consolas", 10));
    suggestedPreview->setStyleSheet(
        "QTextEdit { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "}"
    );
    suggestedLayout->addWidget(suggestedPreview);
    
    // Различия
    QWidget* diffWidget = new QWidget(this);
    QVBoxLayout* diffLayout = new QVBoxLayout(diffWidget);
    diffLayout->addWidget(new QLabel("Различия:", this));
    
    diffPreview = new QTextEdit(this);
    diffPreview->setReadOnly(true);
    diffPreview->setFont(QFont("Consolas", 10));
    diffPreview->setStyleSheet(
        "QTextEdit { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "}"
    );
    diffLayout->addWidget(diffPreview);
    
    previewSplitter->addWidget(originalWidget);
    previewSplitter->addWidget(suggestedWidget);
    previewSplitter->addWidget(diffWidget);
    previewSplitter->setStretchFactor(0, 1);
    previewSplitter->setStretchFactor(1, 1);
    previewSplitter->setStretchFactor(2, 1);
    
    layout->addWidget(previewSplitter, 1);
}

void LLMAssistantDialog::setupControlPanel() {
    controlPanel = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(controlPanel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);
    
    applySelectedButton = new QPushButton("Применить выбранное", this);
    applySelectedButton->setIcon(QIcon(":/icons/apply.svg"));
    connect(applySelectedButton, &QPushButton::clicked, this, &LLMAssistantDialog::onApplySelected);
    layout->addWidget(applySelectedButton);
    
    applyAllButton = new QPushButton("Применить всё", this);
    connect(applyAllButton, &QPushButton::clicked, this, &LLMAssistantDialog::onApplyAll);
    layout->addWidget(applyAllButton);
    
    acceptAllButton = new QPushButton("Принять всё", this);
    connect(acceptAllButton, &QPushButton::clicked, this, &LLMAssistantDialog::onAcceptAll);
    layout->addWidget(acceptAllButton);
    
    rejectAllButton = new QPushButton("Отклонить всё", this);
    connect(rejectAllButton, &QPushButton::clicked, this, &LLMAssistantDialog::onRejectAll);
    layout->addWidget(rejectAllButton);
    
    layout->addStretch();
    
    selectAllButton = new QPushButton("Выделить всё", this);
    connect(selectAllButton, &QPushButton::clicked, this, &LLMAssistantDialog::onSelectAll);
    layout->addWidget(selectAllButton);
    
    deselectAllButton = new QPushButton("Снять выделение", this);
    connect(deselectAllButton, &QPushButton::clicked, this, &LLMAssistantDialog::onDeselectAll);
    layout->addWidget(deselectAllButton);
    
    invertSelectionButton = new QPushButton("Инвертировать", this);
    connect(invertSelectionButton, &QPushButton::clicked, this, &LLMAssistantDialog::onInvertSelection);
    layout->addWidget(invertSelectionButton);
    
    layout->addStretch();
    
    exportButton = new QPushButton("Экспорт", this);
    connect(exportButton, &QPushButton::clicked, this, &LLMAssistantDialog::onExportSuggestions);
    layout->addWidget(exportButton);
    
    importButton = new QPushButton("Импорт", this);
    connect(importButton, &QPushButton::clicked, this, &LLMAssistantDialog::onImportSuggestions);
    layout->addWidget(importButton);
    
    closeButton = new QPushButton("Закрыть", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    layout->addWidget(closeButton);
    
    updateButtonStates();
}

void LLMAssistantDialog::setupConnections() {
    connect(requestButton, &QPushButton::clicked, this, &LLMAssistantDialog::onRequestAssistance);
    connect(cancelButton, &QPushButton::clicked, this, &LLMAssistantDialog::onCancelRequest);
    
    if (llmService) {
        connect(llmService, &LLMService::suggestionsReady, this, &LLMAssistantDialog::onLLMResponse);
        connect(llmService, &LLMService::errorOccurred, this, &LLMAssistantDialog::onLLMError);
        connect(llmService, &LLMService::processingStarted, this, &LLMAssistantDialog::onLLMProcessingStarted);
        connect(llmService, &LLMService::processingFinished, this, &LLMAssistantDialog::onLLMProcessingFinished);
    }
}

void LLMAssistantDialog::setupCategoryFilters() {
    // Инициализация фильтров категорий
    categoryFilter = QStringList();
}

void LLMAssistantDialog::loadSettings() {
    QFile file(settingsPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("minConfidence")) {
            minConfidence = obj["minConfidence"].toDouble();
            confidenceSlider->setValue(static_cast<int>(minConfidence * 100));
        }
        if (obj.contains("autoApply")) {
            autoApply = obj["autoApply"].toBool();
        }
        if (obj.contains("timeout")) {
            timeout = obj["timeout"].toInt();
        }
    }
}

void LLMAssistantDialog::saveSettings() {
    QJsonObject obj;
    obj["minConfidence"] = minConfidence;
    obj["autoApply"] = autoApply;
    obj["timeout"] = timeout;
    
    QFile file(settingsPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        file.close();
    }
}

// ============================================================================
// Публичные слоты
// ============================================================================

void LLMAssistantDialog::setFile(const QString& file) {
    filePath = file;
}

void LLMAssistantDialog::setSelection(int start, int end, const QString& code) {
    selectionStart = start;
    selectionEnd = end;
    selectedCode = code;
    
    // Отображение в предпросмотре
    originalPreview->setPlainText(formatCodeForDisplay(code));
}

void LLMAssistantDialog::setPrompt(const QString& p) {
    prompt = p;
    promptEdit->setPlainText(p);
}

void LLMAssistantDialog::setLLMService(LLMService* service) {
    llmService = service;
    
    if (llmService) {
        setupConnections();
    }
}

void LLMAssistantDialog::setAutoApply(bool enable) {
    autoApply = enable;
}

void LLMAssistantDialog::setMinConfidence(double confidence) {
    minConfidence = confidence;
    confidenceSlider->setValue(static_cast<int>(confidence * 100));
}

void LLMAssistantDialog::setCategoryFilter(const QStringList& categories) {
    categoryFilter = categories;
}

void LLMAssistantDialog::onRequestAssistance() {
    if (!llmService) {
        QMessageBox::warning(this, "Warning", "LLM service not initialized");
        return;
    }
    
    if (selectedCode.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No code selected");
        return;
    }
    
    sendRequestToLLM();
}

void LLMAssistantDialog::onCancelRequest() {
    if (llmService && isProcessing) {
        // Отмена запроса к LLM
        timeoutTimer->stop();
        isProcessing = false;
        statusLabel->setText("Запрос отменён");
        progressBar->setVisible(false);
        updateButtonStates();
    }
}

void LLMAssistantDialog::onApplySelected() {
    QVector<CodeModification> accepted = getAcceptedModifications();
    
    if (accepted.isEmpty()) {
        QMessageBox::information(this, "Info", "No modifications selected");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Apply Modifications",
        QString("Apply %1 selected modification(s)?").arg(accepted.size()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        applyModifications(accepted);
        accept();
    }
}

void LLMAssistantDialog::onApplyAll() {
    if (modifications.isEmpty()) {
        QMessageBox::information(this, "Info", "No modifications available");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Apply All",
        QString("Apply all %1 modification(s)?").arg(modifications.size()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        applyModifications(modifications);
        accept();
    }
}

void LLMAssistantDialog::onRejectAll() {
    for (CodeModification& mod : modifications) {
        mod.accepted = false;
    }
    displayModifications();
    updateStatistics();
}

void LLMAssistantDialog::onAcceptAll() {
    for (CodeModification& mod : modifications) {
        if (!mod.applied) {
            mod.accepted = true;
        }
    }
    displayModifications();
    updateStatistics();
}

void LLMAssistantDialog::onSelectAll() {
    for (CodeModification& mod : modifications) {
        if (!mod.applied) {
            mod.accepted = true;
        }
    }
    displayModifications();
    updateStatistics();
}

void LLMAssistantDialog::onDeselectAll() {
    for (CodeModification& mod : modifications) {
        mod.accepted = false;
    }
    displayModifications();
    updateStatistics();
}

void LLMAssistantDialog::onInvertSelection() {
    for (CodeModification& mod : modifications) {
        if (!mod.applied) {
            mod.accepted = !mod.accepted;
        }
    }
    displayModifications();
    updateStatistics();
}

void LLMAssistantDialog::onViewAllOriginal() {
    mainTabWidget->setCurrentWidget(previewTab);
    // Показать оригинальный код для всех
}

void LLMAssistantDialog::onViewAllSuggested() {
    mainTabWidget->setCurrentWidget(previewTab);
    // Показать предлагаемый код для всех
}

void LLMAssistantDialog::onViewAllDiff() {
    mainTabWidget->setCurrentWidget(previewTab);
    // Показать различия для всех
}

void LLMAssistantDialog::onExportSuggestions() {
    QString path = QFileDialog::getSaveFileName(
        this, "Export Suggestions", "", 
        "HTML Files (*.html);;JSON Files (*.json);;All Files (*)");
    
    if (path.isEmpty()) return;
    
    if (path.endsWith(".html", Qt::CaseInsensitive)) {
        exportToHtml(path);
    } else if (path.endsWith(".json", Qt::CaseInsensitive)) {
        exportToJson(path);
    }
}

void LLMAssistantDialog::onImportSuggestions() {
    QString path = QFileDialog::getOpenFileName(
        this, "Import Suggestions", "", 
        "JSON Files (*.json);;All Files (*)");
    
    if (path.isEmpty()) return;
    
    // Загрузка и парсинг JSON
    // TODO: Implement import
}

void LLMAssistantDialog::onClearSuggestions() {
    modifications.clear();
    displayModifications();
    updateStatistics();
}

void LLMAssistantDialog::onFilterChanged(const QString& text) {
    displayModifications();
}

void LLMAssistantDialog::onCategoryFilterChanged(const QString& category) {
    if (category == "all") {
        categoryFilter.clear();
    } else {
        categoryFilter = QStringList() << category;
    }
    displayModifications();
}

void LLMAssistantDialog::onConfidenceFilterChanged(double value) {
    minConfidence = value;
    displayModifications();
}

void LLMAssistantDialog::onModificationAccepted(int blockId, bool accepted) {
    for (CodeModification& mod : modifications) {
        if (mod.blockId == blockId) {
            mod.accepted = accepted;
            break;
        }
    }
    updateStatistics();
}

void LLMAssistantDialog::onViewOriginal(int blockId) {
    for (const CodeModification& mod : modifications) {
        if (mod.blockId == blockId) {
            originalPreview->setPlainText(formatCodeForDisplay(mod.originalCode));
            mainTabWidget->setCurrentWidget(previewTab);
            break;
        }
    }
}

void LLMAssistantDialog::onViewSuggested(int blockId) {
    for (const CodeModification& mod : modifications) {
        if (mod.blockId == blockId) {
            suggestedPreview->setPlainText(formatCodeForDisplay(mod.suggestedCode));
            mainTabWidget->setCurrentWidget(previewTab);
            break;
        }
    }
}

void LLMAssistantDialog::onShowDiff(int blockId) {
    for (const CodeModification& mod : modifications) {
        if (mod.blockId == blockId) {
            QString diff = generateDiff(mod.originalCode, mod.suggestedCode);
            diffPreview->setPlainText(diff);
            mainTabWidget->setCurrentWidget(previewTab);
            break;
        }
    }
}

void LLMAssistantDialog::onLLMResponse(const QVector<CodeSuggestion>& suggestions) {
    isProcessing = false;
    timeoutTimer->stop();
    progressBar->setVisible(false);
    
    parseLLMResponse(suggestions);
    displayModifications();
    updateStatistics();
    
    statusLabel->setText(QString("Получено %1 предложений").arg(modifications.size()));
    updateButtonStates();
    
    mainTabWidget->setCurrentWidget(suggestionsTab);
}

void LLMAssistantDialog::onLLMError(const QString& error) {
    isProcessing = false;
    timeoutTimer->stop();
    progressBar->setVisible(false);
    
    statusLabel->setText("Ошибка: " + error);
    updateButtonStates();
    
    QMessageBox::critical(this, "LLM Error", error);
}

void LLMAssistantDialog::onLLMProcessingStarted() {
    isProcessing = true;
    progressBar->setVisible(true);
    statusLabel->setText("Запрос к LLM...");
    updateButtonStates();
    
    timeoutTimer->start(timeout);
}

void LLMAssistantDialog::onLLMProcessingFinished() {
    isProcessing = false;
    progressBar->setVisible(false);
    updateButtonStates();
}

// ============================================================================
// Приватные слоты
// ============================================================================

void LLMAssistantDialog::onTimeout() {
    isProcessing = false;
    progressBar->setVisible(false);
    statusLabel->setText("Таймаут запроса");
    updateButtonStates();
    
    QMessageBox::warning(this, "Timeout", "Request timed out. Please try again.");
}

void LLMAssistantDialog::updateProgressBar() {
    // Обновление прогресс бара
}

void LLMAssistantDialog::updateStatistics() {
    acceptedCount = 0;
    appliedCount = 0;
    double totalConfidence = 0.0;
    categoryStats.clear();
    
    for (const CodeModification& mod : modifications) {
        if (mod.accepted) acceptedCount++;
        if (mod.applied) appliedCount++;
        totalConfidence += mod.confidence;
        categoryStats[mod.category]++;
    }
    
    averageConfidence = modifications.isEmpty() ? 0.0 : 
                       totalConfidence / modifications.size();
    
    statisticsLabel->setText(
        QString("Всего: %1 | Принято: %2 | Применено: %3 | Средняя уверенность: %4%")
        .arg(modifications.size())
        .arg(acceptedCount)
        .arg(appliedCount)
        .arg(static_cast<int>(averageConfidence * 100))
    );
}

void LLMAssistantDialog::updateButtonStates() {
    bool hasModifications = !modifications.isEmpty();
    bool hasAccepted = getAcceptedCount() > 0;
    
    applySelectedButton->setEnabled(hasAccepted);
    applyAllButton->setEnabled(hasModifications);
    acceptAllButton->setEnabled(hasModifications && !isProcessing);
    rejectAllButton->setEnabled(hasModifications && !isProcessing);
    selectAllButton->setEnabled(hasModifications && !isProcessing);
    deselectAllButton->setEnabled(hasModifications && !isProcessing);
    invertSelectionButton->setEnabled(hasModifications && !isProcessing);
    exportButton->setEnabled(hasModifications);
    
    requestButton->setEnabled(!isProcessing);
    cancelButton->setEnabled(isProcessing);
}

void LLMAssistantDialog::onTabChanged(int index) {
    // Обработка переключения табов
}

// ============================================================================
// Приватные методы
// ============================================================================

void LLMAssistantDialog::sendRequestToLLM() {
    if (!llmService) return;
    
    QString prompt = promptEdit->toPlainText();
    if (prompt.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter a prompt");
        return;
    }
    
    // Отправка запроса к LLM
    llmService->suggestModifications(
        filePath,
        selectionStart,
        selectionEnd,
        selectedCode,
        prompt
    );
}

void LLMAssistantDialog::parseLLMResponse(const QVector<CodeSuggestion>& suggestions) {
    modifications.clear();
    
    for (const CodeSuggestion& sugg : suggestions) {
        if (sugg.confidence < minConfidence) {
            continue;  // Фильтрация по уверенности
        }
        
        if (!categoryFilter.isEmpty() && !categoryFilter.contains(sugg.explanation)) {
            continue;  // Фильтрация по категории
        }
        
        CodeModification mod;
        mod.blockId = sugg.blockId;
        mod.startLine = sugg.startLine;
        mod.endLine = sugg.endLine;
        mod.originalCode = sugg.originalCode;
        mod.suggestedCode = sugg.suggestedCode;
        mod.explanation = sugg.explanation;
        mod.confidence = sugg.confidence;
        mod.accepted = false;
        mod.applied = false;
        mod.category = "other";  // В полной реализации - определение категории
        mod.timestamp = QDateTime::currentDateTime();
        
        modifications.append(mod);
    }
    
    currentBlockId = modifications.isEmpty() ? 0 : modifications.last().blockId + 1;
}

void LLMAssistantDialog::displayModifications() {
    // Очистка контейнера
    QLayoutItem* child;
    while ((child = modificationsLayout->takeAt(0)) != nullptr) {
        child->widget()->deleteLater();
        delete child;
    }
    
    // Фильтрация и отображение
    for (const CodeModification& mod : modifications) {
        // Применение фильтров
        if (!filterEdit->text().isEmpty()) {
            if (!mod.explanation.contains(filterEdit->text(), Qt::CaseInsensitive) &&
                !mod.originalCode.contains(filterEdit->text(), Qt::CaseInsensitive) &&
                !mod.suggestedCode.contains(filterEdit->text(), Qt::CaseInsensitive)) {
                continue;
            }
        }
        
        if (!categoryFilter.isEmpty() && !categoryFilter.contains(mod.category)) {
            continue;
        }
        
        // Создание виджета
        ModificationItemWidget* widget = new ModificationItemWidget(mod, this);
        connect(widget, &ModificationItemWidget::acceptanceChanged,
                this, &LLMAssistantDialog::onModificationAccepted);
        connect(widget, &ModificationItemWidget::viewOriginalRequested,
                this, &LLMAssistantDialog::onViewOriginal);
        connect(widget, &ModificationItemWidget::viewSuggestedRequested,
                this, &LLMAssistantDialog::onViewSuggested);
        connect(widget, &ModificationItemWidget::showDiffRequested,
                this, &LLMAssistantDialog::onShowDiff);
        
        modificationsLayout->insertWidget(modificationsLayout->count() - 1, widget);
    }
}

void LLMAssistantDialog::applyModifications(const QVector<CodeModification>& mods) {
    // В полной реализации - применение изменений к коду
    // Через сигнал в редактор кода
    
    for (CodeModification& mod : modifications) {
        for (const CodeModification& accepted : mods) {
            if (mod.blockId == accepted.blockId) {
                mod.applied = true;
                appliedCount++;
                break;
            }
        }
    }
    
    displayModifications();
    updateStatistics();
    
    emit accepted(QDialog::Accepted);
}

void LLMAssistantDialog::highlightAcceptedItems() {
    // Подсветка принятых элементов
}

void LLMAssistantDialog::updateModificationStatistics() {
    updateStatistics();
}

QString LLMAssistantDialog::generateDiff(const QString& original, const QString& suggested) {
    // В полной реализации - использование QDiff или аналогичной библиотеки
    // Для примера - простая реализация
    
    QString diff;
    QStringList originalLines = original.split("\n");
    QStringList suggestedLines = suggested.split("\n");
    
    int maxLines = qMax(originalLines.size(), suggestedLines.size());
    
    for (int i = 0; i < maxLines; i++) {
        QString origLine = i < originalLines.size() ? originalLines[i] : "";
        QString suggLine = i < suggestedLines.size() ? suggestedLines[i] : "";
        
        if (origLine == suggLine) {
            diff += "  " + origLine + "\n";
        } else {
            if (!origLine.isEmpty()) {
                diff += "- " + origLine + "\n";
            }
            if (!suggLine.isEmpty()) {
                diff += "+ " + suggLine + "\n";
            }
        }
    }
    
    return diff;
}

QString LLMAssistantDialog::formatCodeForDisplay(const QString& code) {
    return code;
}

QColor LLMAssistantDialog::getConfidenceColor(double confidence) const {
    if (confidence >= 0.8) return QColor(106, 153, 85);    // Зелёный
    if (confidence >= 0.6) return QColor(206, 167, 0);     // Жёлтый
    if (confidence >= 0.4) return QColor(244, 71, 71);     // Оранжевый
    return QColor(200, 50, 50);                             // Красный
}

QString LLMAssistantDialog::getConfidenceTooltip(double confidence) const {
    if (confidence >= 0.8) return "Высокая уверенность";
    if (confidence >= 0.6) return "Средняя уверенность";
    if (confidence >= 0.4) return "Низкая уверенность";
    return "Очень низкая уверенность";
}

void LLMAssistantDialog::showModificationDetails(int blockId) {
    // Показать детали модификации
}

void LLMAssistantDialog::exportToHtml(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;
    
    QTextStream out(&file);
    out << "<!DOCTYPE html>\n<html>\n<head>\n";
    out << "<meta charset=\"UTF-8\">\n";
    out << "<title>LLM Suggestions</title>\n";
    out << "<style>\n";
    out << "body { font-family: Consolas, monospace; background: #1e1e1e; color: #d4d4d4; }\n";
    out << ".accepted { background: #1a3a1a; }\n";
    out << ".rejected { background: #3a1a1a; }\n";
    out << "pre { background: #2d2d2d; padding: 10px; }\n";
    out << "</style>\n";
    out << "</head>\n<body>\n";
    
    for (const CodeModification& mod : modifications) {
        out << "<div class=\"" << (mod.accepted ? "accepted" : "rejected") << "\">\n";
        out << "<h3>Block " << mod.blockId << " (Lines " << mod.startLine << "-" << mod.endLine << ")</h3>\n";
        out << "<p>Confidence: " << static_cast<int>(mod.confidence * 100) << "%</p>\n";
        out << "<p>Explanation: " << mod.explanation.toHtmlEscaped() << "</p>\n";
        out << "<h4>Original:</h4>\n<pre>" << mod.originalCode.toHtmlEscaped() << "</pre>\n";
        out << "<h4>Suggested:</h4>\n<pre>" << mod.suggestedCode.toHtmlEscaped() << "</pre>\n";
        out << "</div>\n";
    }
    
    out << "</body>\n</html>\n";
    file.close();
}

void LLMAssistantDialog::exportToJson(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;
    
    QJsonArray array;
    for (const CodeModification& mod : modifications) {
        QJsonObject obj;
        obj["blockId"] = mod.blockId;
        obj["startLine"] = mod.startLine;
        obj["endLine"] = mod.endLine;
        obj["originalCode"] = mod.originalCode;
        obj["suggestedCode"] = mod.suggestedCode;
        obj["explanation"] = mod.explanation;
        obj["confidence"] = mod.confidence;
        obj["accepted"] = mod.accepted;
        obj["applied"] = mod.applied;
        obj["category"] = mod.category;
        array.append(obj);
    }
    
    file.write(QJsonDocument(array).toJson());
    file.close();
}

// ============================================================================
// Геттеры
// ============================================================================

QVector<CodeModification> LLMAssistantDialog::getAcceptedModifications() const {
    QVector<CodeModification> accepted;
    for (const CodeModification& mod : modifications) {
        if (mod.accepted) {
            accepted.append(mod);
        }
    }
    return accepted;
}

QVector<CodeModification> LLMAssistantDialog::getAppliedModifications() const {
    QVector<CodeModification> applied;
    for (const CodeModification& mod : modifications) {
        if (mod.applied) {
            applied.append(mod);
        }
    }
    return applied;
}

int LLMAssistantDialog::getAcceptedCount() const {
    int count = 0;
    for (const CodeModification& mod : modifications) {
        if (mod.accepted) count++;
    }
    return count;
}

int LLMAssistantDialog::getAppliedCount() const {
    int count = 0;
    for (const CodeModification& mod : modifications) {
        if (mod.applied) count++;
    }
    return count;
}

double LLMAssistantDialog::getAverageConfidence() const {
    if (modifications.isEmpty()) return 0.0;
    
    double total = 0.0;
    for (const CodeModification& mod : modifications) {
        total += mod.confidence;
    }
    return total / modifications.size();
}

QMap<QString, int> LLMAssistantDialog::getCategoryStatistics() const {
    return categoryStats;
}

} // namespace proxima