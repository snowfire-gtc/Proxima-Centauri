#include "AdvancedSearchWidget.h"
#include <QApplication>
#include <QClipboard>
#include <QTextCursor>
#include <QScrollBar>
#include <QXmlStreamWriter>
#include <QCsvWriter>
#include <QDateTime>
#include <QToolTip>
#include <QRegularExpression>
#include <QMath>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>

namespace proxima {

// ============================================================================
// AdvancedSearchWidget Implementation
// ============================================================================

AdvancedSearchWidget::AdvancedSearchWidget(QWidget *parent)
    : QWidget(parent)
    , runtime(nullptr)
    , debugger(nullptr)
    , isSearchRunning(false)
    , searchCancelled(false)
    , currentContextId(0)
    , searchThread(nullptr)
    , searchWorker(nullptr)
    , totalVariablesSearched(0)
    , totalMatchesFound(0)
    , averageValue(0.0)
    , minValue(0.0)
    , maxValue(0.0) {
    
    setupUI();
    setupConnections();
    setupShortcuts();
    setupContextMenu();
    loadSettings();
    
    LOG_DEBUG("AdvancedSearchWidget created");
}

AdvancedSearchWidget::~AdvancedSearchWidget() {
    if (isSearchRunning) {
        stopSearch();
    }
    
    saveSettings();
    
    if (searchThread) {
        delete searchThread;
    }
    if (searchWorker) {
        delete searchWorker;
    }
    
    LOG_DEBUG("AdvancedSearchWidget destroyed");
}

void AdvancedSearchWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Стиль виджета
    setStyleSheet(
        "QWidget { "
        "  background-color: #252526; "
        "  color: #d4d4d4; "
        "}"
        "QGroupBox { "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 5px; "
        "  margin-top: 10px; "
        "  padding-top: 10px; "
        "}"
        "QGroupBox::title { "
        "  subcontrol-origin: margin; "
        "  left: 10px; "
        "  padding: 0 5px; "
        "  color: #4ec9b0; "
        "}"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox { "
        "  background-color: #3c3c3c; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 5px; "
        "}"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus { "
        "  border: 1px solid #007acc; "
        "}"
        "QPushButton { "
        "  background-color: #3c3c3c; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  padding: 5px 15px; "
        "}"
        "QPushButton:hover { "
        "  background-color: #4c4c4c; "
        "  border: 1px solid #007acc; "
        "}"
        "QPushButton:pressed { "
        "  background-color: #2c2c2c; "
        "}"
        "QCheckBox { "
        "  color: #d4d4d4; "
        "  spacing: 5px; "
        "}"
        "QLabel { "
        "  color: #808080; "
        "}"
        "QProgressBar { "
        "  background-color: #2d2d2d; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  text-align: center; "
        "}"
        "QProgressBar::chunk { "
        "  background-color: #007acc; "
        "}"
        "QTableWidget, QListWidget { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  border: 1px solid #3e3e3e; "
        "  border-radius: 3px; "
        "  gridline-color: #2d2d2d; "
        "}"
        "QTableWidget::item:selected, QListWidget::item:selected { "
        "  background-color: #094771; "
        "  color: #ffffff; "
        "}"
        "QTableWidget::item:hover, QListWidget::item:hover { "
        "  background-color: #2a2d2e; "
        "}"
    );
    
    // Панель параметров поиска
    setupSearchPanel();
    mainLayout->addWidget(searchParamsGroup);
    
    // Панель результатов
    setupResultsPanel();
    mainLayout->addWidget(resultsGroup, 1);
    
    // Панель управления
    setupControlPanel();
    mainLayout->addWidget(controlGroup);
}

void AdvancedSearchWidget::setupSearchPanel() {
    searchParamsGroup = new QGroupBox("Параметры поиска", this);
    QVBoxLayout* layout = new QVBoxLayout(searchParamsGroup);
    layout->setSpacing(8);
    
    // Искомое значение и допуск
    QHBoxLayout* valueLayout = new QHBoxLayout();
    valueLayout->addWidget(new QLabel("Значение:", this));
    
    valueEdit = new QLineEdit(this);
    valueEdit->setPlaceholderText("Например: 3.14159");
    valueEdit->setToolTip("Искомое числовое значение");
    valueLayout->addWidget(valueEdit, 1);
    
    valueLayout->addWidget(new QLabel("Допуск ±:", this));
    
    toleranceSpin = new QDoubleSpinBox(this);
    toleranceSpin->setRange(0.0, 1000000.0);
    toleranceSpin->setDecimals(10);
    toleranceSpin->setValue(0.001);
    toleranceSpin->setSingleStep(0.001);
    toleranceSpin->setToolTip("Допуск для поиска значений (±)");
    connect(toleranceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AdvancedSearchWidget::onToleranceChanged);
    valueLayout->addWidget(toleranceSpin);
    
    layout->addLayout(valueLayout);
    
    // Имя и тип переменной
    QHBoxLayout* varLayout = new QHBoxLayout();
    varLayout->addWidget(new QLabel("Имя переменной:", this));
    
    variableNameEdit = new QLineEdit(this);
    variableNameEdit->setPlaceholderText("Пусто = все переменные");
    varLayout->addWidget(variableNameEdit, 1);
    
    varLayout->addWidget(new QLabel("Тип:", this));
    
    variableTypeCombo = new QComboBox(this);
    variableTypeCombo->addItem("Все типы", "");
    variableTypeCombo->addItem("vector", "vector");
    variableTypeCombo->addItem("matrix", "matrix");
    variableTypeCombo->addItem("layer", "layer");
    variableTypeCombo->addItem("collection", "collection");
    variableTypeCombo->addItem("scalar", "scalar");
    connect(variableTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() {
        onVariableTypeChanged(variableTypeCombo->currentData().toString());
    });
    varLayout->addWidget(variableTypeCombo);
    
    layout->addLayout(varLayout);
    
    // Размерность
    QHBoxLayout* dimLayout = new QHBoxLayout();
    dimLayout->addWidget(new QLabel("Размерность:", this));
    
    minDimensionSpin = new QSpinBox(this);
    minDimensionSpin->setRange(1, 3);
    minDimensionSpin->setValue(1);
    minDimensionSpin->setToolTip("Минимальная размерность (1=vector, 2=matrix, 3=layer)");
    dimLayout->addWidget(minDimensionSpin);
    
    dimLayout->addWidget(new QLabel("до", this));
    
    maxDimensionSpin = new QSpinBox(this);
    maxDimensionSpin->setRange(1, 3);
    maxDimensionSpin->setValue(3);
    maxDimensionSpin->setToolTip("Максимальная размерность");
    dimLayout->addWidget(maxDimensionSpin);
    
    dimLayout->addStretch();
    layout->addLayout(dimLayout);
    
    // Область поиска
    QGridLayout* scopeLayout = new QGridLayout();
    
    globalsCheck = new QCheckBox("Глобальные", this);
    globalsCheck->setChecked(true);
    scopeLayout->addWidget(globalsCheck, 0, 0);
    
    localsCheck = new QCheckBox("Локальные", this);
    localsCheck->setChecked(true);
    scopeLayout->addWidget(localsCheck, 0, 1);
    
    collectionsCheck = new QCheckBox("Collection", this);
    collectionsCheck->setChecked(true);
    scopeLayout->addWidget(collectionsCheck, 1, 0);
    
    vectorsCheck = new QCheckBox("Vector", this);
    vectorsCheck->setChecked(true);
    scopeLayout->addWidget(vectorsCheck, 1, 1);
    
    matricesCheck = new QCheckBox("Matrix", this);
    matricesCheck->setChecked(true);
    scopeLayout->addWidget(matricesCheck, 2, 0);
    
    layersCheck = new QCheckBox("Layer", this);
    layersCheck->setChecked(true);
    scopeLayout->addWidget(layersCheck, 2, 1);
    
    layout->addLayout(scopeLayout);
    
    // Дополнительные опции
    QHBoxLayout* optionsLayout = new QHBoxLayout();
    
    caseSensitiveCheck = new QCheckBox("С учётом регистра", this);
    optionsLayout->addWidget(caseSensitiveCheck);
    
    optionsLayout->addWidget(new QLabel("Макс. результатов:", this));
    
    maxResultsSpin = new QSpinBox(this);
    maxResultsSpin->setRange(10, 100000);
    maxResultsSpin->setValue(1000);
    maxResultsSpin->setSingleStep(100);
    optionsLayout->addWidget(maxResultsSpin);
    
    optionsLayout->addStretch();
    layout->addLayout(optionsLayout);
    
    // Кнопки поиска
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    searchButton = new QPushButton("🔍 Начать поиск", this);
    searchButton->setMinimumWidth(150);
    buttonLayout->addWidget(searchButton);
    
    stopButton = new QPushButton("⏹ Остановить", this);
    stopButton->setMinimumWidth(150);
    stopButton->setEnabled(false);
    buttonLayout->addWidget(stopButton);
    
    layout->addLayout(buttonLayout);
}

void AdvancedSearchWidget::setupResultsPanel() {
    resultsGroup = new QGroupBox("Результаты", this);
    QVBoxLayout* layout = new QVBoxLayout(resultsGroup);
    layout->setSpacing(8);
    
    // Табы результатов
    resultsTabWidget = new QTabWidget(this);
    
    // Список результатов
    resultsList = new QListWidget(this);
    resultsList->setAlternatingRowColors(true);
    resultsTabWidget->addTab(resultsList, "Список");
    
    // Таблица результатов
    resultsTable = new QTableWidget(this);
    resultsTable->setColumnCount(7);
    resultsTable->setHorizontalHeaderLabels(
        QStringList() << "Переменная" << "Тип" << "Индексы" << "Значение" 
                      << "Допуск" << "Контекст" << "Время");
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    resultsTabWidget->addTab(resultsTable, "Таблица");
    
    // График
    chartView = new QChartView(this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->chart()->setBackgroundVisible(false);
    chartView->chart()->setAnimationOptions(QChart::SeriesAnimations);
    resultsTabWidget->addTab(chartView, "График");
    
    layout->addWidget(resultsTabWidget, 1);
    
    // Фильтр и опции
    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Фильтр:", this));
    
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Фильтр результатов...");
    filterEdit->setClearButtonEnabled(true);
    connect(filterEdit, &QLineEdit::textChanged, this, &AdvancedSearchWidget::onFilterTextChanged);
    filterLayout->addWidget(filterEdit, 1);
    
    groupByVariableCheck = new QCheckBox("Группировать по переменным", this);
    groupByVariableCheck->setChecked(true);
    connect(groupByVariableCheck, &QCheckBox::toggled, this, &AdvancedSearchWidget::onGroupByVariableToggled);
    filterLayout->addWidget(groupByVariableCheck);
    
    layout->addLayout(filterLayout);
    
    // Статистика
    statisticsLabel = new QLabel("Результатов: 0", this);
    statisticsLabel->setStyleSheet("color: #4ec9b0; font-weight: bold;");
    layout->addWidget(statisticsLabel);
}

void AdvancedSearchWidget::setupControlPanel() {
    controlGroup = new QGroupBox("Управление", this);
    QHBoxLayout* layout = new QHBoxLayout(controlGroup);
    layout->setSpacing(5);
    
    exportButton = new QPushButton("Экспорт", this);
    layout->addWidget(exportButton);
    
    clearButton = new QPushButton("Очистить", this);
    layout->addWidget(clearButton);
    
    layout->addStretch();
    
    chartButton = new QPushButton("📊 График", this);
    layout->addWidget(chartButton);
    
    distributionButton = new QPushButton("📈 Распределение", this);
    layout->addWidget(distributionButton);
    
    layout->addStretch();
    
    copyButton = new QPushButton("📋 Копировать", this);
    layout->addWidget(copyButton);
    
    navigateButton = new QPushButton("📍 Перейти", this);
    layout->addWidget(navigateButton);
    
    layout->addStretch();
    
    // Прогресс и статус
    QVBoxLayout* statusLayout = new QVBoxLayout();
    
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setVisible(false);
    progressBar->setFixedHeight(20);
    statusLayout->addWidget(progressBar);
    
    statusLabel = new QLabel("Готов к поиску", this);
    statusLabel->setStyleSheet("color: #808080;");
    statusLayout->addWidget(statusLabel);
    
    layout->addLayout(statusLayout);
}

void AdvancedSearchWidget::setupConnections() {
    connect(searchButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onSearchButtonClicked);
    connect(stopButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onStopButtonClicked);
    connect(resultsList, &QListWidget::itemClicked, this, &AdvancedSearchWidget::onResultItemClicked);
    connect(resultsList, &QListWidget::itemDoubleClicked, this, &AdvancedSearchWidget::onResultItemDoubleClicked);
    connect(resultsTable, &QTableWidget::cellClicked, this, &AdvancedSearchWidget::onResultTableClicked);
    connect(resultsTable, &QTableWidget::cellDoubleClicked, this, &AdvancedSearchWidget::onResultTableDoubleClicked);
    connect(exportButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onExportResults);
    connect(clearButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onClearResults);
    connect(chartButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onViewChart);
    connect(distributionButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onViewDistribution);
    connect(copyButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onCopyResult);
    connect(navigateButton, &QPushButton::clicked, this, &AdvancedSearchWidget::onNavigateToSource);
}

void AdvancedSearchWidget::setupShortcuts() {
    // Ctrl+F - Поиск
    QShortcut* searchShortcut = new QShortcut(QKeySequence::Find, this);
    connect(searchShortcut, &QShortcut::activated, this, [this]() {
        valueEdit->setFocus();
        valueEdit->selectAll();
    });
    
    // Ctrl+Shift+F - Расширенный поиск
    QShortcut* advancedSearchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F), this);
    connect(advancedSearchShortcut, &QShortcut::activated, this, &AdvancedSearchWidget::onSearchButtonClicked);
    
    // Esc - Остановить поиск
    QShortcut* stopShortcut = new QShortcut(QKeySequence::Cancel, this);
    connect(stopShortcut, &QShortcut::activated, this, [this]() {
        if (isSearchRunning) {
            onStopButtonClicked();
        }
    });
    
    // Ctrl+C - Копировать результат
    QShortcut* copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, &AdvancedSearchWidget::onCopyResult);
    
    // Ctrl+E - Экспорт
    QShortcut* exportShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), this);
    connect(exportShortcut, &QShortcut::activated, this, &AdvancedSearchWidget::onExportResults);
}

void AdvancedSearchWidget::setupContextMenu() {
    contextMenu = new QMenu(this);
    
    QAction* copyAction = contextMenu->addAction("Копировать значение", this, &AdvancedSearchWidget::onCopyResult);
    copyAction->setShortcut(QKeySequence::Copy);
    
    QAction* copyAllAction = contextMenu->addAction("Копировать все результаты", this, &AdvancedSearchWidget::onCopyAllResults);
    
    contextMenu->addSeparator();
    
    QAction* navigateAction = contextMenu->addAction("Перейти к источнику", this, &AdvancedSearchWidget::onNavigateToSource);
    
    contextMenu->addSeparator();
    
    QAction* chartAction = contextMenu->addAction("Показать график", this, &AdvancedSearchWidget::onViewChart);
    
    QAction* distributionAction = contextMenu->addAction("Показать распределение", this, &AdvancedSearchWidget::onViewDistribution);
    
    contextMenu->addSeparator();
    
    QAction* exportAction = contextMenu->addAction("Экспорт результатов", this, &AdvancedSearchWidget::onExportResults);
    
    resultsList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(resultsList, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        contextMenu->exec(resultsList->mapToGlobal(pos));
    });
    
    resultsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(resultsTable, &QTableWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        contextMenu->exec(resultsTable->mapToGlobal(pos));
    });
}

void AdvancedSearchWidget::setRuntime(Runtime* rt) {
    runtime = rt;
}

void AdvancedSearchWidget::setDebugger(Debugger* dbg) {
    debugger = dbg;
}

void AdvancedSearchWidget::setSearchParams(const AdvancedSearchParams& params) {
    searchParams = params;
    
    valueEdit->setText(QString::number(params.searchValue));
    toleranceSpin->setValue(params.tolerance);
    variableNameEdit->setText(params.variableName);
    
    int typeIndex = variableTypeCombo->findData(params.variableType);
    if (typeIndex >= 0) {
        variableTypeCombo->setCurrentIndex(typeIndex);
    }
    
    minDimensionSpin->setValue(params.minDimension);
    maxDimensionSpin->setValue(params.maxDimension);
    globalsCheck->setChecked(params.searchInGlobals);
    localsCheck->setChecked(params.searchInLocals);
    collectionsCheck->setChecked(params.searchInCollections);
    vectorsCheck->setChecked(params.searchInVectors);
    matricesCheck->setChecked(params.searchInMatrices);
    layersCheck->setChecked(params.searchInLayers);
    caseSensitiveCheck->setChecked(params.caseSensitive);
    maxResultsSpin->setValue(params.maxResults);
}

void AdvancedSearchWidget::startSearch() {
    // Валидация параметров
    if (valueEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Введите искомое значение");
        return;
    }
    
    searchParams.searchValue = valueEdit->text().toDouble();
    searchParams.tolerance = toleranceSpin->value();
    searchParams.variableName = variableNameEdit->text().trimmed();
    searchParams.variableType = variableTypeCombo->currentData().toString();
    searchParams.minDimension = minDimensionSpin->value();
    searchParams.maxDimension = maxDimensionSpin->value();
    searchParams.searchInGlobals = globalsCheck->isChecked();
    searchParams.searchInLocals = localsCheck->isChecked();
    searchParams.searchInCollections = collectionsCheck->isChecked();
    searchParams.searchInVectors = vectorsCheck->isChecked();
    searchParams.searchInMatrices = matricesCheck->isChecked();
    searchParams.searchInLayers = layersCheck->isChecked();
    searchParams.caseSensitive = caseSensitiveCheck->isChecked();
    searchParams.maxResults = maxResultsSpin->value();
    
    if (isSearchRunning) {
        stopSearch();
    }
    
    isSearchRunning = true;
    searchCancelled = false;
    searchResults.clear();
    resultsList->clear();
    resultsTable->setRowCount(0);
    
    searchButton->setEnabled(false);
    stopButton->setEnabled(true);
    progressBar->setVisible(true);
    statusLabel->setText("Поиск...");
    
    emit searchStarted(searchParams);
    
    // Создание worker для поиска в отдельном потоке
    searchThread = new QThread();
    searchWorker = new AdvancedSearchWorker();
    searchWorker->moveToThread(searchThread);
    
    // Настройка worker
    searchWorker->setSearchParams(searchParams);
    searchWorker->setRuntime(runtime);
    searchWorker->setDebugger(debugger);
    searchWorker->setContextId(currentContextId);
    
    // Соединения
    connect(searchThread, &QThread::started, searchWorker, &AdvancedSearchWorker::startSearch);
    connect(searchWorker, &AdvancedSearchWorker::progress, this, &AdvancedSearchWidget::onSearchProgress);
    connect(searchWorker, &AdvancedSearchWorker::finished, this, &AdvancedSearchWidget::onSearchFinished);
    connect(searchWorker, &AdvancedSearchWorker::error, this, &AdvancedSearchWidget::onSearchError);
    connect(this, &AdvancedSearchWidget::searchCancelled, searchWorker, &AdvancedSearchWorker::cancel);
    
    searchThread->start();
}

void AdvancedSearchWidget::stopSearch() {
    if (!isSearchRunning) return;
    
    searchCancelled = true;
    emit searchCancelled();
    
    if (searchThread) {
        searchThread->quit();
        searchThread->wait(3000);
        delete searchThread;
        searchThread = nullptr;
    }
    
    if (searchWorker) {
        delete searchWorker;
        searchWorker = nullptr;
    }
    
    isSearchRunning = false;
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);
    statusLabel->setText("Поиск остановлен");
    
    emit searchCancelled();
}

void AdvancedSearchWidget::clearResults() {
    searchResults.clear();
    resultsList->clear();
    resultsTable->setRowCount(0);
    statisticsLabel->setText("Результатов: 0");
    statusLabel->setText("Результаты очищены");
    
    if (chartView->chart()) {
        chartView->chart()->removeAllSeries();
    }
}

void AdvancedSearchWidget::filterResults(const QString& filter) {
    // Фильтрация отображаемых результатов
    displayResults(searchResults);
}

void AdvancedSearchWidget::sortByVariable(bool group) {
    displayResults(searchResults);
}

void AdvancedSearchWidget::exportResults(const QString& path) {
    if (searchResults.isEmpty()) {
        QMessageBox::information(this, "Info", "Нет результатов для экспорта");
        return;
    }
    
    QFileInfo fileInfo(path);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "json") {
        // Экспорт в JSON
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Error", "Cannot create file");
            return;
        }
        
        QJsonArray array;
        for (const AdvancedSearchResult& result : searchResults) {
            QJsonObject obj;
            obj["variable"] = result.variableName;
            obj["type"] = result.variableType;
            obj["dimension"] = result.dimension;
            obj["indices"] = result.indexString;
            obj["value"] = result.value;
            obj["tolerance"] = result.tolerance;
            obj["context"] = result.contextId;
            obj["file"] = result.filePath;
            obj["line"] = result.lineNumber;
            obj["timestamp"] = result.timestamp;
            array.append(obj);
        }
        
        file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
        file.close();
    } else if (suffix == "csv") {
        // Экспорт в CSV
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Error", "Cannot create file");
            return;
        }
        
        QTextStream out(&file);
        out << "Variable,Type,Dimension,Indices,Value,Tolerance,Context,File,Line,Timestamp\n";
        
        for (const AdvancedSearchResult& result : searchResults) {
            out << result.variableName << ","
                << result.variableType << ","
                << result.dimension << ","
                << result.indexString << ","
                << result.value << ","
                << result.tolerance << ","
                << result.contextId << ","
                << result.filePath << ","
                << result.lineNumber << ","
                << result.timestamp << "\n";
        }
        
        file.close();
    } else {
        // Экспорт в TXT
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Error", "Cannot create file");
            return;
        }
        
        QTextStream out(&file);
        out << "Advanced Search Results\n";
        out << "======================\n\n";
        out << "Search value: " << searchParams.searchValue << "\n";
        out << "Tolerance: ±" << searchParams.tolerance << "\n";
        out << "Total matches: " << searchResults.size() << "\n\n";
        
        for (const AdvancedSearchResult& result : searchResults) {
            out << "Variable: " << result.variableName << "\n";
            out << "  Type: " << result.variableType << "\n";
            out << "  Dimension: " << result.dimension << "\n";
            out << "  Indices: " << result.indexString << "\n";
            out << "  Value: " << result.value << "\n";
            out << "  Tolerance: ±" << result.tolerance << "\n";
            out << "  Context: " << result.contextId << "\n";
            if (!result.filePath.isEmpty()) {
                out << "  File: " << result.filePath << ":" << result.lineNumber << "\n";
            }
            out << "\n";
        }
        
        file.close();
    }
    
    QMessageBox::information(this, "Export", "Results exported to " + path);
}

void AdvancedSearchWidget::showValueDistribution() {
    if (searchResults.isEmpty()) {
        QMessageBox::information(this, "Info", "Нет результатов для отображения");
        return;
    }
    
    createDistributionChart(searchResults);
    resultsTabWidget->setCurrentWidget(chartView);
}

void AdvancedSearchWidget::showResultChart() {
    if (searchResults.isEmpty()) {
        QMessageBox::information(this, "Info", "Нет результатов для отображения");
        return;
    }
    
    createChart(searchResults);
    resultsTabWidget->setCurrentWidget(chartView);
}

void AdvancedSearchWidget::highlightInEditor(const AdvancedSearchResult& result) {
    // Выделение в редакторе кода
    emit resultSelected(result);
}

void AdvancedSearchWidget::setCurrentContext(int contextId) {
    currentContextId = contextId;
}

QMap<QString, int> AdvancedSearchWidget::getVariableStatistics() const {
    return variableStats;
}

double AdvancedSearchWidget::getAverageValue() const {
    return averageValue;
}

double AdvancedSearchWidget::getMinValue() const {
    return minValue;
}

double AdvancedSearchWidget::getMaxValue() const {
    return maxValue;
}

void AdvancedSearchWidget::onSearchButtonClicked() {
    startSearch();
}

void AdvancedSearchWidget::onStopButtonClicked() {
    stopSearch();
}

void AdvancedSearchWidget::onResultItemClicked(QListWidgetItem* item) {
    AdvancedSearchResult result = item->data(Qt::UserRole).value<AdvancedSearchResult>();
    emit resultSelected(result);
}

void AdvancedSearchWidget::onResultItemDoubleClicked(QListWidgetItem* item) {
    AdvancedSearchResult result = item->data(Qt::UserRole).value<AdvancedSearchResult>();
    emit resultDoubleClicked(result);
    highlightInEditor(result);
}

void AdvancedSearchWidget::onResultTableClicked(int row, int column) {
    if (row < 0 || row >= searchResults.size()) return;
    
    AdvancedSearchResult result = searchResults[row];
    emit resultSelected(result);
}

void AdvancedSearchWidget::onResultTableDoubleClicked(int row, int column) {
    if (row < 0 || row >= searchResults.size()) return;
    
    AdvancedSearchResult result = searchResults[row];
    emit resultDoubleClicked(result);
    highlightInEditor(result);
}

void AdvancedSearchWidget::onSearchProgress(int variablesSearched, int totalVariables, int matchesFound) {
    statusLabel->setText(QString("Поиск: %1/%2 переменных, %3 совпадений")
        .arg(variablesSearched)
        .arg(totalVariables)
        .arg(matchesFound));
}

void AdvancedSearchWidget::onSearchFinished(const QVector<AdvancedSearchResult>& results) {
    isSearchRunning = false;
    searchResults = results;
    
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);
    
    displayResults(results);
    updateStatistics();
    
    statusLabel->setText(QString("Найдено %1 совпадений в %2 переменных")
        .arg(results.size())
        .arg(totalVariablesSearched));
    
    emit searchFinished(results.size(), totalVariablesSearched);
    
    // Очистка worker
    if (searchThread) {
        searchThread->quit();
        searchThread->wait();
        delete searchThread;
        searchThread = nullptr;
    }
    
    if (searchWorker) {
        delete searchWorker;
        searchWorker = nullptr;
    }
}

void AdvancedSearchWidget::onSearchError(const QString& error) {
    isSearchRunning = false;
    searchButton->setEnabled(true);
    stopButton->setEnabled(false);
    progressBar->setVisible(false);
    statusLabel->setText("Ошибка: " + error);
    
    emit errorOccurred(error);
}

void AdvancedSearchWidget::onExportResults() {
    QString path = QFileDialog::getSaveFileName(
        this, "Export Results", "", 
        "JSON Files (*.json);;CSV Files (*.csv);;Text Files (*.txt);;All Files (*)");
    
    if (!path.isEmpty()) {
        exportResults(path);
    }
}

void AdvancedSearchWidget::onClearResults() {
    clearResults();
}

void AdvancedSearchWidget::onFilterTextChanged(const QString& text) {
    filterResults(text);
}

void AdvancedSearchWidget::onToleranceChanged(double value) {
    searchParams.tolerance = value;
}

void AdvancedSearchWidget::onVariableTypeChanged(const QString& type) {
    searchParams.variableType = type;
}

void AdvancedSearchWidget::onGroupByVariableToggled(bool checked) {
    displayResults(searchResults);
}

void AdvancedSearchWidget::onViewChart() {
    showResultChart();
}

void AdvancedSearchWidget::onViewDistribution() {
    showValueDistribution();
}

void AdvancedSearchWidget::onCopyResult() {
    if (resultsList->currentRow() >= 0) {
        QListWidgetItem* item = resultsList->currentItem();
        AdvancedSearchResult result = item->data(Qt::UserRole).value<AdvancedSearchResult>();
        
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(QString("Variable: %1\nIndices: %2\nValue: %3")
            .arg(result.variableName)
            .arg(result.indexString)
            .arg(result.value));
    } else if (resultsTable->currentRow() >= 0) {
        int row = resultsTable->currentRow();
        if (row < searchResults.size()) {
            AdvancedSearchResult result = searchResults[row];
            
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(QString("Variable: %1\nIndices: %2\nValue: %3")
                .arg(result.variableName)
                .arg(result.indexString)
                .arg(result.value));
        }
    }
}

void AdvancedSearchWidget::onCopyAllResults() {
    if (searchResults.isEmpty()) return;
    
    QString text;
    for (const AdvancedSearchResult& result : searchResults) {
        text += QString("Variable: %1, Indices: %2, Value: %3\n")
            .arg(result.variableName)
            .arg(result.indexString)
            .arg(result.value);
    }
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void AdvancedSearchWidget::onNavigateToSource() {
    if (resultsList->currentRow() >= 0) {
        QListWidgetItem* item = resultsList->currentItem();
        AdvancedSearchResult result = item->data(Qt::UserRole).value<AdvancedSearchResult>();
        highlightInEditor(result);
    }
}

void AdvancedSearchWidget::displayResults(const QVector<AdvancedSearchResult>& results) {
    resultsList->clear();
    
    if (groupByVariableCheck->isChecked()) {
        // Группировка по переменным
        QMap<QString, QVector<AdvancedSearchResult>> grouped;
        for (const AdvancedSearchResult& result : results) {
            grouped[result.variableName].append(result);
        }
        
        for (auto it = grouped.begin(); it != grouped.end(); ++it) {
            // Заголовок переменной
            QListWidgetItem* varItem = new QListWidgetItem("📊 " + it.key() + 
                                                           " (" + QString::number(it.value().size()) + ")");
            varItem->setForeground(QColor(78, 201, 176));
            varItem->setFont(QFont("Segoe UI", 10, QFont::Bold));
            resultsList->addItem(varItem);
            
            // Результаты в переменной
            for (const AdvancedSearchResult& result : it.value()) {
                QListWidgetItem* item = new QListWidgetItem(
                    QString("  %s: %1").arg(result.indexString, -15).arg(result.value, 0, 'f', 6));
                item->setData(Qt::UserRole, QVariant::fromValue(result));
                item->setForeground(QColor(212, 212, 212));
                resultsList->addItem(item);
            }
        }
    } else {
        // Плоский список
        for (const AdvancedSearchResult& result : results) {
            QListWidgetItem* item = new QListWidgetItem(
                QString("%1%s: %2")
                    .arg(result.variableName, -20)
                    .arg(result.indexString, -15)
                    .arg(result.value, 0, 'f', 6));
            item->setData(Qt::UserRole, QVariant::fromValue(result));
            resultsList->addItem(item);
        }
    }
    
    displayResultsInTable(results);
}

void AdvancedSearchWidget::displayResultsInTable(const QVector<AdvancedSearchResult>& results) {
    resultsTable->setRowCount(results.size());
    
    for (int i = 0; i < results.size(); i++) {
        const AdvancedSearchResult& result = results[i];
        
        QTableWidgetItem* varItem = new QTableWidgetItem(result.variableName);
        QTableWidgetItem* typeItem = new QTableWidgetItem(result.variableType);
        QTableWidgetItem* indicesItem = new QTableWidgetItem(result.indexString);
        QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(result.value, 'f', 10));
        QTableWidgetItem* toleranceItem = new QTableWidgetItem(QString::number(result.tolerance, 'e', 4));
        QTableWidgetItem* contextItem = new QTableWidgetItem(QString::number(result.contextId));
        QTableWidgetItem* timeItem = new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(result.timestamp).toString("hh:mm:ss.zzz"));
        
        resultsTable->setItem(i, 0, varItem);
        resultsTable->setItem(i, 1, typeItem);
        resultsTable->setItem(i, 2, indicesItem);
        resultsTable->setItem(i, 3, valueItem);
        resultsTable->setItem(i, 4, toleranceItem);
        resultsTable->setItem(i, 5, contextItem);
        resultsTable->setItem(i, 6, timeItem);
    }
}

void AdvancedSearchWidget::updateStatistics() {
    if (searchResults.isEmpty()) {
        statisticsLabel->setText("Результатов: 0");
        averageValue = 0.0;
        minValue = 0.0;
        maxValue = 0.0;
        variableStats.clear();
        return;
    }
    
    double sum = 0.0;
    minValue = searchResults[0].value;
    maxValue = searchResults[0].value;
    variableStats.clear();
    
    for (const AdvancedSearchResult& result : searchResults) {
        sum += result.value;
        if (result.value < minValue) minValue = result.value;
        if (result.value > maxValue) maxValue = result.value;
        variableStats[result.variableName]++;
    }
    
    averageValue = sum / searchResults.size();
    
    statisticsLabel->setText(
        QString("Результатов: %1 | Среднее: %2 | Мин: %3 | Макс: %4")
        .arg(searchResults.size())
        .arg(averageValue, 0, 'f', 6)
        .arg(minValue, 0, 'f', 6)
        .arg(maxValue, 0, 'f', 6)
    );
}

void AdvancedSearchWidget::createChart(const QVector<AdvancedSearchResult>& results) {
    if (!chartView->chart()) {
        QChart* chart = new QChart();
        chart->setTitle("Search Results");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        chartView->setChart(chart);
    }
    
    chartView->chart()->removeAllSeries();
    
    QLineSeries* series = new QLineSeries();
    series->setName("Values");
    
    for (int i = 0; i < results.size(); i++) {
        series->append(i, results[i].value);
    }
    
    chartView->chart()->addSeries(series);
    
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Index");
    axisX->setRange(0, results.size());
    chartView->chart()->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Value");
    axisY->setRange(minValue - 0.1, maxValue + 0.1);
    chartView->chart()->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}

void AdvancedSearchWidget::createDistributionChart(const QVector<AdvancedSearchResult>& results) {
    if (!chartView->chart()) {
        QChart* chart = new QChart();
        chart->setTitle("Value Distribution");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->legend()->setVisible(true);
        chartView->setChart(chart);
    }
    
    chartView->chart()->removeAllSeries();
    
    // Гистограмма распределения
    QBarSeries* barSeries = new QBarSeries();
    QBarSet* barSet = new QBarSet("Distribution");
    
    // Разбиение на диапазоны
    int bins = 10;
    double range = maxValue - minValue;
    double binSize = range / bins;
    
    QVector<int> binCounts(bins, 0);
    
    for (const AdvancedSearchResult& result : results) {
        int binIndex = qMin(static_cast<int>((result.value - minValue) / binSize), bins - 1);
        binCounts[binIndex]++;
    }
    
    for (int count : binCounts) {
        *barSet << count;
    }
    
    barSeries->append(barSet);
    chartView->chart()->addSeries(barSeries);
    
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Value Range");
    axisX->setRange(0, bins);
    chartView->chart()->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Count");
    chartView->chart()->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
}

void AdvancedSearchWidget::loadSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    searchParams.tolerance = settings.value("advancedSearch/tolerance", 0.001).toDouble();
    searchParams.variableType = settings.value("advancedSearch/variableType", "").toString();
    searchParams.minDimension = settings.value("advancedSearch/minDimension", 1).toInt();
    searchParams.maxDimension = settings.value("advancedSearch/maxDimension", 3).toInt();
    searchParams.searchInGlobals = settings.value("advancedSearch/searchInGlobals", true).toBool();
    searchParams.searchInLocals = settings.value("advancedSearch/searchInLocals", true).toBool();
    searchParams.caseSensitive = settings.value("advancedSearch/caseSensitive", false).toBool();
    searchParams.maxResults = settings.value("advancedSearch/maxResults", 1000).toInt();
    
    toleranceSpin->setValue(searchParams.tolerance);
    minDimensionSpin->setValue(searchParams.minDimension);
    maxDimensionSpin->setValue(searchParams.maxDimension);
    globalsCheck->setChecked(searchParams.searchInGlobals);
    localsCheck->setChecked(searchParams.searchInLocals);
    caseSensitiveCheck->setChecked(searchParams.caseSensitive);
    maxResultsSpin->setValue(searchParams.maxResults);
}

void AdvancedSearchWidget::saveSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    settings.setValue("advancedSearch/tolerance", searchParams.tolerance);
    settings.setValue("advancedSearch/variableType", searchParams.variableType);
    settings.setValue("advancedSearch/minDimension", searchParams.minDimension);
    settings.setValue("advancedSearch/maxDimension", searchParams.maxDimension);
    settings.setValue("advancedSearch/searchInGlobals", searchParams.searchInGlobals);
    settings.setValue("advancedSearch/searchInLocals", searchParams.searchInLocals);
    settings.setValue("advancedSearch/caseSensitive", searchParams.caseSensitive);
    settings.setValue("advancedSearch/maxResults", searchParams.maxResults);
}

QString AdvancedSearchWidget::getSettingsPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + 
           "/advanced_search_settings.ini";
}

// ============================================================================
// AdvancedSearchWorker Implementation
// ============================================================================

AdvancedSearchWorker::AdvancedSearchWorker(QObject *parent)
    : QObject(parent)
    , runtime(nullptr)
    , debugger(nullptr)
    , contextId(0)
    , cancelled(false)
    , variablesSearched(0)
    , totalVariables(0)
    , matchesFound(0) {
}

void AdvancedSearchWorker::setSearchParams(const AdvancedSearchParams& p) {
    params = p;
}

void AdvancedSearchWorker::setRuntime(Runtime* rt) {
    runtime = rt;
}

void AdvancedSearchWorker::setDebugger(Debugger* dbg) {
    debugger = dbg;
}

void AdvancedSearchWorker::setContextId(int id) {
    contextId = id;
}

void AdvancedSearchWorker::cancel() {
    cancelled = true;
}

void AdvancedSearchWorker::startSearch() {
    cancelled = false;
    results.clear();
    variablesSearched = 0;
    matchesFound = 0;
    
    searchVariables();
    
    emit finished(results);
}

void AdvancedSearchWorker::searchVariables() {
    // Получение списка переменных из отладчика
    QVector<QString> varNames = getVariableNames();
    totalVariables = varNames.size();
    
    for (const QString& varName : varNames) {
        if (cancelled) break;
        
        // Фильтрация по имени
        if (!params.variableName.isEmpty()) {
            if (params.caseSensitive) {
                if (varName != params.variableName) continue;
            } else {
                if (varName.toLower() != params.variableName.toLower()) continue;
            }
        }
        
        // Получение типа и значения
        QString varType = getVariableTypeByName(varName);
        QString varValue = getVariableValue(varName);
        
        // Фильтрация по типу
        if (!params.variableType.isEmpty() && varType != params.variableType) {
            continue;
        }
        
        // Фильтрация по размерности
        int dimension = getVariableDimension(varType);
        if (dimension < params.minDimension || dimension > params.maxDimension) {
            continue;
        }
        
        // Поиск в переменной
        searchVariable(varName, varType, varValue);
        
        variablesSearched++;
        emit progress(variablesSearched, totalVariables, matchesFound);
        
        // Проверка лимита результатов
        if (results.size() >= params.maxResults) {
            break;
        }
    }
}

void AdvancedSearchWorker::searchVariable(const QString& name, const QString& type, const QString& value) {
    QVector<int> indices;
    QVector<AdvancedSearchResult> matches = findMatches(name, type, value, indices);
    
    if (!matches.isEmpty()) {
        results.append(matches);
        matchesFound += matches.size();
    }
}

QVector<AdvancedSearchResult> AdvancedSearchWorker::findMatches(const QString& varName, 
                                                                 const QString& varType, 
                                                                 const QString& varValue,
                                                                 const QVector<int>& indices) {
    QVector<AdvancedSearchResult> matches;
    
    // Парсинг значения переменной
    // Для vector: [1.0, 2.0, 3.0, ...]
    // Для matrix: [[1.0, 2.0], [3.0, 4.0], ...]
    // Для collection: более сложный формат
    
    QRegularExpression numberRe(R"((-?\d+\.?\d*(?:[eE][+-]?\d+)?)");
    QRegularExpressionMatchIterator it = numberRe.globalMatch(varValue);
    
    int elementIndex = 0;
    while (it.hasNext()) {
        if (cancelled) break;
        if (matches.size() >= params.maxResults) break;
        
        QRegularExpressionMatch match = it.next();
        double value = match.captured(1).toDouble();
        
        if (valueMatches(value, params.searchValue, params.tolerance)) {
            AdvancedSearchResult result;
            result.variableName = varName;
            result.variableType = varType;
            result.dimension = getVariableDimension(varType);
            result.indices = indices;
            result.indices.append(elementIndex);
            result.indexString = formatIndices(result.indices);
            result.value = value;
            result.tolerance = params.tolerance;
            result.contextId = contextId;
            result.timestamp = QDateTime::currentMSecsSinceEpoch();
            
            matches.append(result);
        }
        
        elementIndex++;
    }
    
    return matches;
}

bool AdvancedSearchWorker::valueMatches(double value, double target, double tolerance) const {
    return qAbs(value - target) <= tolerance;
}

QVector<int> AdvancedSearchWorker::parseIndices(const QString& indicesStr) const {
    QVector<int> indices;
    
    QRegularExpression numberRe(R"(\d+)");
    QRegularExpressionMatchIterator it = numberRe.globalMatch(indicesStr);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        indices.append(match.captured(0).toInt());
    }
    
    return indices;
}

QString AdvancedSearchWorker::formatIndices(const QVector<int>& indices) const {
    if (indices.isEmpty()) return "";
    
    QString result = "[";
    for (int i = 0; i < indices.size(); i++) {
        if (i > 0) result += ", ";
        result += QString::number(indices[i]);
    }
    result += "]";
    
    return result;
}

QString AdvancedSearchWorker::getVariableType(const QString& value) const {
    if (value.startsWith("[")) {
        if (value.contains(",,,")) return "layer";
        if (value.contains(",, ")) return "matrix";
        if (value.contains(", ")) return "vector";
    }
    return "scalar";
}

int AdvancedSearchWorker::getVariableDimension(const QString& type) const {
    if (type == "layer") return 3;
    if (type == "matrix") return 2;
    if (type == "vector") return 1;
    return 0;
}

QVector<QString> AdvancedSearchWorker::getVariableNames() const {
    QVector<QString> names;
    
    if (debugger) {
        auto vars = debugger->getLocalVariables();
        for (auto it = vars.begin(); it != vars.end(); ++it) {
            names.append(it.key());
        }
    }
    
    return names;
}

QString AdvancedSearchWorker::getVariableValue(const QString& name) const {
    if (debugger) {
        return debugger->getVariableValue(name);
    }
    return "";
}

QString AdvancedSearchWorker::getVariableTypeByName(const QString& name) const {
    if (debugger) {
        // В полной реализации - получение типа из отладчика
        return "auto";
    }
    return "auto";
}

} // namespace proxima