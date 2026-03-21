#ifndef CENTAURI_LLMASSISTANTDIALOG_H
#define CENTAURI_LLMASSISTANTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QScrollArea>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QColor>
#include <QFont>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include "services/llm/LLMService.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Структура предложения по модификации кода
 * 
 * Согласно требованию ide.txt пункт 29:
 * Пользователь может поблочно выбирать, какие модификации следует применить к коду.
 */
struct CodeModification {
    int blockId;                    // Идентификатор блока
    int startLine;                  // Начальная строка
    int endLine;                    // Конечная строка
    QString originalCode;           // Оригинальный код
    QString suggestedCode;          // Предлагаемый код
    QString explanation;            // Объяснение изменения
    double confidence;              // Уверенность LLM (0.0 - 1.0)
    bool accepted;                  // Принято ли пользователем
    bool applied;                   // Применено ли к коду
    QString category;               // Категория изменения (optimization, bugfix, etc.)
    QDateTime timestamp;            // Время получения предложения
    
    CodeModification() : blockId(0), startLine(0), endLine(0), 
                         confidence(0.0), accepted(false), applied(false) {}
};

/**
 * @brief Виджет для отображения одного предложения по модификации
 */
class ModificationItemWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ModificationItemWidget(const CodeModification& mod, QWidget *parent = nullptr);
    ~ModificationItemWidget();
    
    CodeModification getModification() const { return modification; }
    void setModification(const CodeModification& mod);
    bool isAccepted() const { return checkBox->isChecked(); }
    void setAccepted(bool accepted);
    
    void highlightDifferences();
    void showDiff();
    
signals:
    void acceptanceChanged(int blockId, bool accepted);
    void viewOriginalRequested(int blockId);
    void viewSuggestedRequested(int blockId);
    void showDiffRequested(int blockId);
    
private slots:
    void onCheckBoxToggled(bool checked);
    void onViewOriginalClicked();
    void onViewSuggestedClicked();
    void onShowDiffClicked();
    
private:
    void setupUI();
    void setupConnections();
    QString formatConfidence(double confidence) const;
    QColor getCategoryColor(const QString& category) const;
    QString getCategoryIcon(const QString& category) const;
    
    CodeModification modification;
    
    // UI компоненты
    QCheckBox* checkBox;            // Чекбокс принятия
    QLabel* categoryLabel;          // Категория изменения
    QLabel* confidenceLabel;        // Уверенность LLM
    QLabel* linesLabel;             // Диапазон строк
    QTextEdit* explanationEdit;     // Объяснение изменения
    QPushButton* viewOriginalBtn;   // Просмотр оригинала
    QPushButton* viewSuggestedBtn;  // Просмотр предложения
    QPushButton* showDiffBtn;       // Показать различия
    QWidget* headerWidget;          // Заголовок блока
    
    // Стили
    QFont consoleFont;
    QMap<QString, QColor> categoryColors;
};

/**
 * @brief Диалог помощника LLM с выборочным применением рекомендаций
 * 
 * Согласно требованию ide.txt пункт 29:
 * IDE должна уметь обращаться через API к LLM, выполняющейся на сервере LM-Studio,
 * и выдавать предложения по модификации кода, и пользователь может поблочно выбирать,
 * какие модификации следует применить к коду.
 */
class LLMAssistantDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit LLMAssistantDialog(QWidget *parent = nullptr);
    ~LLMAssistantDialog();
    
    // Настройка
    void setFile(const QString& file);
    QString getFile() const { return filePath; }
    
    void setSelection(int startLine, int endLine, const QString& code);
    int getSelectionStart() const { return selectionStart; }
    int getSelectionEnd() const { return selectionEnd; }
    QString getSelectedCode() const { return selectedCode; }
    
    void setPrompt(const QString& prompt);
    QString getPrompt() const { return promptEdit->toPlainText(); }
    
    void setLLMService(LLMService* service);
    LLMService* getLLMService() const { return llmService; }
    
    // Результаты
    QVector<CodeModification> getModifications() const { return modifications; }
    QVector<CodeModification> getAcceptedModifications() const;
    QVector<CodeModification> getAppliedModifications() const;
    int getAcceptedCount() const;
    int getAppliedCount() const;
    
    // Конфигурация
    void setAutoApply(bool enable);
    bool getAutoApply() const { return autoApply; }
    void setMinConfidence(double confidence);
    double getMinConfidence() const { return minConfidence; }
    void setCategoryFilter(const QStringList& categories);
    QStringList getCategoryFilter() const { return categoryFilter; }
    
    // Статистика
    int getTotalModifications() const { return modifications.size(); }
    double getAverageConfidence() const;
    QMap<QString, int> getCategoryStatistics() const;
    
public slots:
    void onRequestAssistance();
    void onCancelRequest();
    void onApplySelected();
    void onApplyAll();
    void onRejectAll();
    void onAcceptAll();
    void onSelectAll();
    void onDeselectAll();
    void onInvertSelection();
    void onViewAllOriginal();
    void onViewAllSuggested();
    void onViewAllDiff();
    void onExportSuggestions();
    void onImportSuggestions();
    void onClearSuggestions();
    void onFilterChanged(const QString& text);
    void onCategoryFilterChanged(const QString& category);
    void onConfidenceFilterChanged(double value);
    void onModificationAccepted(int blockId, bool accepted);
    void onViewOriginal(int blockId);
    void onViewSuggested(int blockId);
    void onShowDiff(int blockId);
    void onLLMResponse(const QVector<CodeSuggestion>& suggestions);
    void onLLMError(const QString& error);
    void onLLMProcessingStarted();
    void onLLMProcessingFinished();
    
private slots:
    void onTimeout();
    void updateProgressBar();
    void updateStatistics();
    void updateButtonStates();
    void onTabChanged(int index);
    
private:
    void setupUI();
    void setupRequestPanel();
    void setupSuggestionsPanel();
    void setupPreviewPanel();
    void setupControlPanel();
    void setupConnections();
    void setupCategoryFilters();
    void loadSettings();
    void saveSettings();
    
    void sendRequestToLLM();
    void parseLLMResponse(const QVector<CodeSuggestion>& suggestions);
    void displayModifications();
    void applyModifications(const QVector<CodeModification>& mods);
    void highlightAcceptedItems();
    void updateModificationStatistics();
    QString generateDiff(const QString& original, const QString& suggested);
    QString formatCodeForDisplay(const QString& code);
    QColor getConfidenceColor(double confidence) const;
    QString getConfidenceTooltip(double confidence) const;
    void showModificationDetails(int blockId);
    void exportToHtml(const QString& path);
    void exportToJson(const QString& path);
    
    // UI компоненты
    QTabWidget* mainTabWidget;          // Главные табы
    QWidget* requestTab;                // Вкладка запроса
    QWidget* suggestionsTab;            // Вкладка предложений
    QWidget* previewTab;                // Вкладка предпросмотра
    
    // Панель запроса
    QTextEdit* promptEdit;              // Поле ввода запроса
    QComboBox* promptTemplateCombo;     // Шаблоны запросов
    QCheckBox* includeContextCheck;     // Включить контекст
    QCheckBox* includeCommentsCheck;    // Включить комментарии
    QPushButton* requestButton;         // Кнопка запроса
    QPushButton* cancelButton;          // Кнопка отмены
    
    // Панель предложений
    QListWidget* modificationsList;     // Список предложений
    QScrollArea* modificationsScroll;   // Прокрутка для предложений
    QWidget* modificationsContainer;    // Контейнер для виджетов предложений
    QVBoxLayout* modificationsLayout;   // Layout для предложений
    
    // Фильтры
    QLineEdit* filterEdit;              // Фильтр по тексту
    QComboBox* categoryFilterCombo;     // Фильтр по категории
    QSlider* confidenceSlider;          // Фильтр по уверенности
    QLabel* confidenceLabel;            // Метка уверенности
    
    // Панель предпросмотра
    QSplitter* previewSplitter;         // Сплиттер предпросмотра
    QTextEdit* originalPreview;         // Оригинальный код
    QTextEdit* suggestedPreview;        // Предлагаемый код
    QTextEdit* diffPreview;             // Различия
    
    // Панель управления
    QPushButton* applySelectedButton;   // Применить выбранное
    QPushButton* applyAllButton;        // Применить всё
    QPushButton* rejectAllButton;       // Отклонить всё
    QPushButton* acceptAllButton;       // Принять всё
    QPushButton* selectAllButton;       // Выделить всё
    QPushButton* deselectAllButton;     // Снять выделение
    QPushButton* invertSelectionButton; // Инвертировать выделение
    QPushButton* exportButton;          // Экспорт
    QPushButton* importButton;          // Импорт
    QPushButton* closeButton;           // Закрыть
    
    // Прогресс и статус
    QProgressBar* progressBar;          // Прогресс выполнения
    QLabel* statusLabel;                // Статус
    QLabel* statisticsLabel;            // Статистика
    
    // Данные
    LLMService* llmService;             // LLM сервис
    QString filePath;                   // Путь к файлу
    int selectionStart;                 // Начало выделения
    int selectionEnd;                   // Конец выделения
    QString selectedCode;               // Выделенный код
    QString prompt;                     // Запрос к LLM
    QVector<CodeModification> modifications;  // Предложения по модификации
    
    // Конфигурация
    bool autoApply;                     // Авто-применение
    double minConfidence;               // Минимальная уверенность
    QStringList categoryFilter;         // Фильтр категорий
    
    // Состояние
    bool isProcessing;                  // Обработка в процессе
    int currentBlockId;                 // Текущий блок
    QTimer* timeoutTimer;               // Таймаут
    int timeout;                        // Время таймаута
    
    // Статистика
    int acceptedCount;                  // Количество принятых
    int appliedCount;                   // Количество применённых
    double averageConfidence;           // Средняя уверенность
    QMap<QString, int> categoryStats;   // Статистика по категориям
    
    // Настройки
    QString settingsPath;               // Путь к настройкам
};

} // namespace proxima

#endif // CENTAURI_LLMASSISTANTDIALOG_H