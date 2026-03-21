#ifndef CENTAURI_SEARCHWIDGET_H
#define CENTAURI_SEARCHWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QShortcut>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QTimer>
#include <QSettings>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Результат поиска
 */
struct SearchResult {
    QString filePath;       // Путь к файлу
    int lineNumber;         // Номер строки
    int columnNumber;       // Номер колонки
    int matchLength;        // Длина совпадения
    QString lineText;       // Текст строки
    QString matchedText;    // Найденный текст
    bool isReplace;         // Это замена или поиск
    
    SearchResult() : lineNumber(0), columnNumber(0), matchLength(0), isReplace(false) {}
};

/**
 * @brief Виджет поиска в файле
 * 
 * Согласно требованию ide.txt пункт 11:
 * Панель инструментов редактора с кнопками для различных операций
 */
class SearchWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget();
    
    // Настройка
    void setEditor(QTextEdit* editor);
    QTextEdit* getEditor() const { return textEditor; }
    
    // Поиск
    void findText(const QString& text);
    void findNext();
    void findPrevious();
    void replaceCurrent();
    void replaceAll();
    int getMatchCount() const { return matchCount; }
    
    // Опции поиска
    void setCaseSensitive(bool sensitive);
    bool isCaseSensitive() const { return caseSensitiveCheck->isChecked(); }
    
    void setWholeWord(bool whole);
    bool isWholeWord() const { return wholeWordCheck->isChecked(); }
    
    void setRegularExpression(bool regex);
    bool isRegularExpression() const { return regexCheck->isChecked(); }
    
    void setSearchText(const QString& text);
    QString getSearchText() const { return findEdit->text(); }
    
    void setReplaceText(const QString& text);
    QString getReplaceText() const { return replaceEdit->text(); }
    
    // Видимость
    void showFind();
    void showReplace();
    void hideWidget();
    bool isVisible() const { return QWidget::isVisible(); }
    
    // История
    void addToHistory(const QString& text);
    QStringList getHistory() const { return searchHistory; }
    void clearHistory();
    
    // Статистика
    int getCurrentMatchIndex() const { return currentMatchIndex; }
    void setCurrentMatchIndex(int index) { currentMatchIndex = index; }
    
signals:
    void searchStarted(const QString& text);
    void searchFinished(int matches);
    void matchFound(int index, int total);
    void noMatchFound();
    void replacePerformed(int count);
    void visibilityChanged(bool visible);
    
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    
private slots:
    void onFindTextChanged(const QString& text);
    void onReplaceTextChanged(const QString& text);
    void onFindNext();
    void onFindPrevious();
    void onReplaceCurrent();
    void onReplaceAll();
    void onCaseSensitiveToggled(bool checked);
    void onWholeWordToggled(bool checked);
    void onRegularExpressionToggled(bool checked);
    void onHistorySelected(const QString& text);
    void onShowHistory();
    void onClearHistory();
    void onCloseClicked();
    void onToggleReplace();
    void updateMatchCount();
    void highlightMatches();
    void clearHighlights();
    
private:
    void setupUI();
    void setupConnections();
    void setupShortcuts();
    void createHighlights();
    void updateHighlights();
    void loadSettings();
    void saveSettings();
    QString escapeRegExp(const QString& text) const;
    QRegularExpression createRegExp() const;
    QVector<QTextEdit::ExtraSelection> createMatchSelections() const;
    int countMatches() const;
    QString getSettingsPath() const;
    
    // UI компоненты
    QLineEdit* findEdit;                // Поле поиска
    QLineEdit* replaceEdit;             // Поле замены
    QCheckBox* caseSensitiveCheck;      // С учётом регистра
    QCheckBox* wholeWordCheck;          // Целые слова
    QCheckBox* regexCheck;              // Регулярные выражения
    QPushButton* findNextButton;        // Найти далее
    QPushButton* findPrevButton;        // Найти ранее
    QPushButton* replaceButton;         // Заменить
    QPushButton* replaceAllButton;      // Заменить все
    QToolButton* historyButton;         // История
    QToolButton* closeButton;           // Закрыть
    QLabel* matchCountLabel;            // Счётчик совпадений
    QWidget* replaceWidget;             // Виджет замены
    QMenu* historyMenu;                 // Меню истории
    
    // Редактор
    QTextEdit* textEditor;
    
    // Поиск
    int matchCount;                     // Количество совпадений
    int currentMatchIndex;              // Текущее совпадение
    QVector<int> matchPositions;        // Позиции совпадений
    QVector<QTextEdit::ExtraSelection> matchSelections;  // Выделения совпадений
    
    // История
    QStringList searchHistory;
    int maxHistorySize;
    
    // Настройки
    bool caseSensitive;
    bool wholeWord;
    bool useRegex;
    
    // Таймер для отложенного поиска
    QTimer* searchTimer;
};

/**
 * @brief Виджет поиска по проекту
 * 
 * Поиск по всем файлам проекта с отображением результатов.
 */
class ProjectSearchWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ProjectSearchWidget(QWidget *parent = nullptr);
    ~ProjectSearchWidget();
    
    // Настройка
    void setProjectPath(const QString& path);
    QString getProjectPath() const { return projectPath; }
    
    void setSearchText(const QString& text);
    QString getSearchText() const { return searchEdit->text(); }
    
    // Поиск
    void startSearch();
    void stopSearch();
    bool isSearching() const { return isSearchRunning; }
    
    // Опции
    void setCaseSensitive(bool sensitive);
    void setWholeWord(bool whole);
    void setRegularExpression(bool regex);
    void setIncludeHidden(bool include);
    void setFileFilter(const QString& filter);
    
    // Результаты
    QVector<SearchResult> getResults() const { return searchResults; }
    int getResultCount() const { return searchResults.size(); }
    void clearResults();
    void exportResults(const QString& path);
    
    // Фильтрация результатов
    void filterResults(const QString& filter);
    void groupByFile(bool group);
    
signals:
    void searchStarted(const QString& text, const QString& path);
    void searchProgress(int filesSearched, int totalFiles, int matchesFound);
    void searchFinished(int matches, int files);
    void searchCancelled();
    void resultSelected(const SearchResult& result);
    void errorOccurred(const QString& error);
    
private slots:
    void onSearchTextChanged(const QString& text);
    void onSearchButtonClicked();
    void onStopButtonClicked();
    void onResultItemClicked(QListWidgetItem* item);
    void onResultItemDoubleClicked(QListWidgetItem* item);
    void onSearchProgress(int filesSearched, int totalFiles, int matchesFound);
    void onSearchFinished(const QVector<SearchResult>& results);
    void onSearchError(const QString& error);
    void onExportResults();
    void onClearResults();
    void onFilterTextChanged(const QString& text);
    void onGroupByFileToggled(bool checked);
    
private:
    void setupUI();
    void setupConnections();
    void setupOptions();
    void displayResults(const QVector<SearchResult>& results);
    void searchInDirectory(const QString& dirPath);
    void searchInFile(const QString& filePath);
    QVector<SearchResult> findMatchesInFile(const QString& filePath, const QString& content);
    QStringList getFilesToSearch(const QString& dirPath);
    bool shouldSkipFile(const QString& filePath) const;
    QRegularExpression createRegExp() const;
    QString getSettingsPath() const;
    void loadSettings();
    void saveSettings();
    
    // UI компоненты
    QLineEdit* searchEdit;              // Поле поиска
    QLineEdit* filterEdit;              // Фильтр результатов
    QComboBox* fileFilterCombo;         // Фильтр файлов
    QCheckBox* caseSensitiveCheck;      // С учётом регистра
    QCheckBox* wholeWordCheck;          // Целые слова
    QCheckBox* regexCheck;              // Регулярные выражения
    QCheckBox* includeHiddenCheck;      // Включая скрытые
    QCheckBox* groupByFileCheck;        // Группировать по файлам
    QPushButton* searchButton;          // Кнопка поиска
    QPushButton* stopButton;            // Кнопка остановки
    QPushButton* exportButton;          // Экспорт
    QPushButton* clearButton;           // Очистить
    QListWidget* resultsList;           // Список результатов
    QLabel* statusLabel;                // Статус
    QProgressBar* progressBar;          // Прогресс
    
    // Поиск
    QString projectPath;
    QVector<SearchResult> searchResults;
    bool isSearchRunning;
    bool searchCancelled;
    
    // Worker thread
    QThread* searchThread;
    SearchWorker* searchWorker;
};

/**
 * @brief Worker для поиска в отдельном потоке
 */
class SearchWorker : public QObject {
    Q_OBJECT
    
public:
    explicit SearchWorker(QObject *parent = nullptr);
    
    void setSearchText(const QString& text);
    void setProjectPath(const QString& path);
    void setCaseSensitive(bool sensitive);
    void setWholeWord(bool whole);
    void setRegularExpression(bool regex);
    void setFileFilter(const QString& filter);
    void setIncludeHidden(bool include);
    void cancel();
    
public slots:
    void startSearch();
    
signals:
    void progress(int filesSearched, int totalFiles, int matchesFound);
    void finished(const QVector<SearchResult>& results);
    void error(const QString& error);
    
private:
    void searchDirectory(const QString& dirPath);
    void searchFile(const QString& filePath);
    QVector<SearchResult> findMatches(const QString& filePath, const QString& content);
    
    QString searchText;
    QString projectPath;
    QString fileFilter;
    bool caseSensitive;
    bool wholeWord;
    bool useRegex;
    bool includeHidden;
    bool cancelled;
    
    QVector<SearchResult> results;
    int filesSearched;
    int totalFiles;
    int matchesFound;
};

} // namespace proxima

#endif // CENTAURI_SEARCHWIDGET_H