#ifndef CENTAURI_ADVANCEDSEARCHWIDGET_H
#define CENTAURI_ADVANCEDSEARCHWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QThread>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QKeyEvent>
#include <QPainter>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QLineSeries>
#include <QScatterSeries>
#include "runtime/Runtime.h"
#include "stdlib/Collection.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Результат расширенного поиска значений
 * 
 * Согласно требованию ide.txt пункт 10:
 * Инструменты визуализации состояний объектов
 */
struct AdvancedSearchResult {
    QString variableName;       // Имя переменной
    QString variableType;       // Тип переменной (vector, matrix, collection, etc.)
    int dimension;              // Размерность (1=vector, 2=matrix, 3=layer)
    QVector<int> indices;       // Индексы элемента (например, [row, col] для матрицы)
    QString indexString;        // Строковое представление индексов (например, "[5, 3]")
    double value;               // Найденное значение
    double tolerance;           // Использованный допуск
    int contextId;              // Идентификатор контекста (кадр стека)
    QString filePath;           // Путь к файлу (если применимо)
    int lineNumber;             // Номер строки (если применимо)
    qint64 timestamp;           // Время нахождения
    
    AdvancedSearchResult() : dimension(0), value(0.0), tolerance(0.0), 
                             contextId(0), lineNumber(0), timestamp(0) {}
};

/**
 * @brief Параметры расширенного поиска
 */
struct AdvancedSearchParams {
    double searchValue;         // Искомое значение
    double tolerance;           // Допуск +/-
    QString variableName;       // Имя переменной (пусто = все переменные)
    QString variableType;       // Тип переменной (пусто = все типы)
    int minDimension;           // Минимальная размерность
    int maxDimension;           // Максимальная размерность
    int contextScope;           // Область контекста (0=current, 1=all)
    bool searchInGlobals;       // Искать в глобальных переменных
    bool searchInLocals;        // Искать в локальных переменных
    bool searchInCollections;   // Искать в коллекциях
    bool searchInVectors;       // Искать в векторах
    bool searchInMatrices;      // Искать в матрицах
    bool searchInLayers;        // Искать в слоях
    bool caseSensitive;         // С учётом регистра для имён
    int maxResults;             // Максимальное количество результатов
    QString outputFormat;       // Формат вывода (table, json, csv)
    
    AdvancedSearchParams() : searchValue(0.0), tolerance(0.001), 
                             minDimension(1), maxDimension(3), 
                             contextScope(0), maxResults(1000),
                             searchInGlobals(true), searchInLocals(true),
                             searchInCollections(true), searchInVectors(true),
                             searchInMatrices(true), searchInLayers(true),
                             caseSensitive(false) {}
};

/**
 * @brief Виджет расширенного поиска значений в переменных
 * 
 * Позволяет искать значения переменных и элементов массивов
 * в пределах заданных допусков +/- в текущем контексте выполнения.
 */
class AdvancedSearchWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit AdvancedSearchWidget(QWidget *parent = nullptr);
    ~AdvancedSearchWidget();
    
    // Настройка
    void setRuntime(Runtime* runtime);
    Runtime* getRuntime() const { return runtime; }
    
    void setDebugger(Debugger* debugger);
    Debugger* getDebugger() const { return debugger; }
    
    // Параметры поиска
    void setSearchParams(const AdvancedSearchParams& params);
    AdvancedSearchParams getSearchParams() const { return searchParams; }
    
    // Поиск
    void startSearch();
    void stopSearch();
    bool isSearching() const { return isSearchRunning; }
    
    // Результаты
    QVector<AdvancedSearchResult> getResults() const { return searchResults; }
    int getResultCount() const { return searchResults.size(); }
    void clearResults();
    void exportResults(const QString& path);
    
    // Фильтрация результатов
    void filterResults(const QString& filter);
    void sortResults(int column, Qt::SortOrder order);
    void groupByVariable(bool group);
    
    // Визуализация
    void showValueDistribution();
    void showResultChart();
    void highlightInEditor(const AdvancedSearchResult& result);
    
    // Контекст
    void setCurrentContext(int contextId);
    int getCurrentContext() const { return currentContextId; }
    
    // Статистика
    QMap<QString, int> getVariableStatistics() const;
    double getAverageValue() const;
    double getMinValue() const;
    double getMaxValue() const;
    
signals:
    void searchStarted(const AdvancedSearchParams& params);
    void searchProgress(int variablesSearched, int totalVariables, int matchesFound);
    void searchFinished(int matches, int variables);
    void searchCancelled();
    void resultSelected(const AdvancedSearchResult& result);
    void resultDoubleClicked(const AdvancedSearchResult& result);
    void errorOccurred(const QString& error);
    
private slots:
    void onSearchButtonClicked();
    void onStopButtonClicked();
    void onResultItemClicked(QListWidgetItem* item);
    void onResultItemDoubleClicked(QListWidgetItem* item);
    void onResultTableClicked(int row, int column);
    void onResultTableDoubleClicked(int row, int column);
    void onSearchProgress(int variablesSearched, int totalVariables, int matchesFound);
    void onSearchFinished(const QVector<AdvancedSearchResult>& results);
    void onSearchError(const QString& error);
    void onExportResults();
    void onClearResults();
    void onFilterTextChanged(const QString& text);
    void onToleranceChanged(double value);
    void onVariableTypeChanged(const QString& type);
    void onGroupByVariableToggled(bool checked);
    void onViewChart();
    void onViewDistribution();
    void onCopyResult();
    void onCopyAllResults();
    void onNavigateToSource();
    
private:
    void setupUI();
    void setupSearchPanel();
    void setupResultsPanel();
    void setupControlPanel();
    void setupConnections();
    void setupShortcuts();
    void setupContextMenu();
    void displayResults(const QVector<AdvancedSearchResult>& results);
    void displayResultsInTable(const QVector<AdvancedSearchResult>& results);
    void searchInVariables();
    void searchInVariable(const QString& varName, const QString& varType, const QString& varValue);
    QVector<AdvancedSearchResult> findMatchesInValue(const QString& varName, 
                                                     const QString& varType, 
                                                     const QString& varValue,
                                                     const QVector<int>& indices);
    bool valueMatches(double value, double target, double tolerance) const;
    QVector<int> parseIndices(const QString& indicesStr) const;
    QString formatIndices(const QVector<int>& indices) const;
    QString getVariableType(const QString& value) const;
    int getVariableDimension(const QString& type) const;
    QString getSettingsPath() const;
    void loadSettings();
    void saveSettings();
    void updateStatistics();
    void createChart(const QVector<AdvancedSearchResult>& results);
    void createDistributionChart(const QVector<AdvancedSearchResult>& results);
    
    // UI компоненты
    QGroupBox* searchParamsGroup;       // Параметры поиска
    QLineEdit* valueEdit;               // Искомое значение
    QDoubleSpinBox* toleranceSpin;      // Допуск
    QLineEdit* variableNameEdit;        // Имя переменной
    QComboBox* variableTypeCombo;       // Тип переменной
    QSpinBox* minDimensionSpin;         // Мин. размерность
    QSpinBox* maxDimensionSpin;         // Макс. размерность
    QCheckBox* globalsCheck;            // Глобальные переменные
    QCheckBox* localsCheck;             // Локальные переменные
    QCheckBox* collectionsCheck;        // Коллекции
    QCheckBox* vectorsCheck;            // Вектора
    QCheckBox* matricesCheck;           // Матрицы
    QCheckBox* layersCheck;             // Слои
    QCheckBox* caseSensitiveCheck;      // С учётом регистра
    QSpinBox* maxResultsSpin;           // Макс. результатов
    QPushButton* searchButton;          // Кнопка поиска
    QPushButton* stopButton;            // Кнопка остановки
    
    QGroupBox* resultsGroup;            // Результаты
    QTabWidget* resultsTabWidget;       // Табы результатов
    QListWidget* resultsList;           // Список результатов
    QTableWidget* resultsTable;         // Таблица результатов
    QChartView* chartView;              // График результатов
    QLineEdit* filterEdit;              // Фильтр результатов
    QCheckBox* groupByVariableCheck;    // Группировать по переменным
    QLabel* statisticsLabel;            // Статистика
    
    QGroupBox* controlGroup;            // Управление
    QPushButton* exportButton;          // Экспорт
    QPushButton* clearButton;           // Очистить
    QPushButton* chartButton;           // График
    QPushButton* distributionButton;    // Распределение
    QPushButton* copyButton;            // Копировать
    QPushButton* navigateButton;        // Перейти к источнику
    
    QProgressBar* progressBar;          // Прогресс
    QLabel* statusLabel;                // Статус
    QMenu* contextMenu;                 // Контекстное меню
    
    // Данные
    Runtime* runtime;
    Debugger* debugger;
    AdvancedSearchParams searchParams;
    QVector<AdvancedSearchResult> searchResults;
    bool isSearchRunning;
    bool searchCancelled;
    int currentContextId;
    
    // Worker thread
    QThread* searchThread;
    AdvancedSearchWorker* searchWorker;
    
    // Статистика
    int totalVariablesSearched;
    int totalMatchesFound;
    double averageValue;
    double minValue;
    double maxValue;
    QMap<QString, int> variableStats;
};

/**
 * @brief Worker для расширенного поиска в отдельном потоке
 */
class AdvancedSearchWorker : public QObject {
    Q_OBJECT
    
public:
    explicit AdvancedSearchWorker(QObject *parent = nullptr);
    
    void setSearchParams(const AdvancedSearchParams& params);
    void setRuntime(Runtime* runtime);
    void setDebugger(Debugger* debugger);
    void setContextId(int contextId);
    void cancel();
    
public slots:
    void startSearch();
    
signals:
    void progress(int variablesSearched, int totalVariables, int matchesFound);
    void finished(const QVector<AdvancedSearchResult>& results);
    void error(const QString& error);
    
private:
    void searchVariables();
    void searchVariable(const QString& name, const QString& type, const QString& value);
    QVector<AdvancedSearchResult> findMatches(const QString& varName, 
                                              const QString& varType, 
                                              const QString& varValue,
                                              const QVector<int>& indices);
    bool valueMatches(double value, double target, double tolerance) const;
    QVector<int> parseIndices(const QString& indicesStr) const;
    QString formatIndices(const QVector<int>& indices) const;
    QString getVariableType(const QString& value) const;
    int getVariableDimension(const QString& type) const;
    QVector<QString> getVariableNames() const;
    QString getVariableValue(const QString& name) const;
    QString getVariableTypeByName(const QString& name) const;
    
    AdvancedSearchParams params;
    Runtime* runtime;
    Debugger* debugger;
    int contextId;
    bool cancelled;
    
    QVector<AdvancedSearchResult> results;
    int variablesSearched;
    int totalVariables;
    int matchesFound;
};

} // namespace proxima

#endif // CENTAURI_ADVANCEDSEARCHWIDGET_H