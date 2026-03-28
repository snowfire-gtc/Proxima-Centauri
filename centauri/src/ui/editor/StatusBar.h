#ifndef CENTAURI_STATUSBAR_H
#define CENTAURI_STATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QMap>
#include <QString>

namespace proxima {

/**
 * @brief Режимы работы IDE для отображения в строке состояния
 */
enum class IDEMode;

/**
 * @brief Строка состояния редактора кода
 * 
 * Согласно требованию ide.txt пункт 17:
 * Внизу текстового редактора отображается строка состояния, в которой отображается:
 * - позиция курсора в файле;
 * - статус выполнения, позиция выполнения;
 * - статус сохранения изменения;
 * - текущее системное время - отображается только в активном окне;
 * - ресурсы системы (ЦПУ, память, диск) - отображается только в активном окне;
 * - состояние подключения к LLM-модели:
 *   - название модели;
 *   - размер свободного контекста и полный размер контекста;
 *   - статус генерации ("gen" - формирование нового кода, "fix" - исправление ошибок, 
 *     "doc" - формирование документации, "error" - ошибка, "idle" - простаивание).
 */
class StatusBar : public QStatusBar {
    Q_OBJECT

public:
    /**
     * @brief Конструктор строки состояния
     * @param parent Родительский виджет
     */
    explicit StatusBar(QWidget *parent = nullptr);
    
    /**
     * @brief Деструктор строки состояния
     */
    ~StatusBar();

    // ========================================================================
    // Позиция курсора
    // ========================================================================
    
    /**
     * @brief Установка позиции курсора
     * @param line Номер строки (начиная с 1)
     * @param column Номер столбца (начиная с 1)
     */
    void setCursorPosition(int line, int column);
    
    /**
     * @brief Получение текущего номера строки
     * @return Номер строки
     */
    int getCurrentLine() const { return currentLine; }
    
    /**
     * @brief Получение текущего номера столбца
     * @return Номер столбца
     */
    int getCurrentColumn() const { return currentColumn; }

    // ========================================================================
    // Статус выполнения
    // ========================================================================
    
    /**
     * @brief Установка статуса выполнения
     * @param mode Режим выполнения IDE
     */
    void setExecutionStatus(IDEMode mode);
    
    /**
     * @brief Установка позиции выполнения
     * @param file Файл выполнения
     * @param line Номер строки выполнения
     */
    void setExecutionPosition(const QString& file, int line);
    
    /**
     * @brief Получение текущего статуса выполнения
     * @return Строковое представление статуса
     */
    QString getExecutionStatusText() const;

    // ========================================================================
    // Статус сохранения
    // ========================================================================
    
    /**
     * @brief Установка статуса сохранения
     * @param saved true если файл сохранён
     */
    void setSaveStatus(bool saved);
    
    /**
     * @brief Проверка статуса сохранения
     * @return true если файл сохранён
     */
    bool isSaved() const { return fileSaved; }

    // ========================================================================
    // Системное время
    // ========================================================================
    
    /**
     * @brief Установка текущего системного времени
     * @param time Текущее время
     */
    void setCurrentTime(const QTime& time);
    
    /**
     * @brief Получение текущего отображаемого времени
     * @return Текущее время
     */
    QTime getCurrentTime() const { return currentTime; }

    // ========================================================================
    // Ресурсы системы
    // ========================================================================
    
    /**
     * @brief Установка значений системных ресурсов
     * @param cpuUsage Загрузка CPU в процентах
     * @param memoryUsage Использование памяти в МБ
     * @param diskUsage Использование диска в процентах
     */
    void setSystemResources(double cpuUsage, double memoryUsage, double diskUsage);
    
    /**
     * @brief Получение загрузки CPU
     * @return Загрузка CPU в процентах
     */
    double getCpuUsage() const { return cpuUsagePercent; }
    
    /**
     * @brief Получение использования памяти
     * @return Использование памяти в МБ
     */
    double getMemoryUsage() const { return memoryUsageMB; }
    
    /**
     * @brief Получение использования диска
     * @return Использование диска в процентах
     */
    double getDiskUsage() const { return diskUsagePercent; }

    // ========================================================================
    // LLM статус
    // ========================================================================
    
    /**
     * @brief Статусы генерации LLM
     */
    enum class LLMStatus {
        Idle,       // Простаивание
        Generating, // Формирование нового кода (gen)
        Fixing,     // Исправление ошибок (fix)
        Documenting,// Формирование документации (doc)
        Error       // Ошибка
    };
    
    /**
     * @brief Установка информации о подключении к LLM
     * @param modelName Название модели
     * @param freeContext Свободный контекст (токенов)
     * @param totalContext Полный размер контекста (токенов)
     * @param status Статус генерации
     */
    void setLLMStatus(const QString& modelName, 
                     int freeContext, 
                     int totalContext,
                     LLMStatus status);
    
    /**
     * @brief Получение названия модели LLM
     * @return Название модели
     */
    QString getLLMModelName() const { return llmModelName; }
    
    /**
     * @brief Получение статуса LLM
     * @return Текущий статус LLM
     */
    LLMStatus getLLMStatus() const { return llmStatus; }
    
    /**
     * @brief Получение строкового представления статуса LLM
     * @return Строка статуса (gen/fix/doc/error/idle)
     */
    QString getLLMStatusText() const;

    // ========================================================================
    // Обновление отображения
    // ========================================================================
    
    /**
     * @brief Принудительное обновление всех элементов строки состояния
     */
    void updateDisplay();
    
    /**
     * @brief Установка активности окна
     * @param active true если окно активно
     */
    void setActiveWindow(bool active);
    
    /**
     * @brief Проверка активности окна
     * @return true если окно активно
     */
    bool isActiveWindow() const { return windowActive; }

public slots:
    /**
     * @brief Слот обновления системных ресурсов
     */
    void refreshSystemResources();

private:
    /**
     * @brief Инициализация компонентов строки состояния
     */
    void setupUI();
    
    /**
     * @brief Создание метки позиции курсора
     */
    void createCursorPositionLabel();
    
    /**
     * @brief Создание метки статуса выполнения
     */
    void createExecutionStatusLabel();
    
    /**
     * @brief Создание метки статуса сохранения
     */
    void createSaveStatusLabel();
    
    /**
     * @brief Создание метки системного времени
     */
    void createTimeLabel();
    
    /**
     * @brief Создание метки системных ресурсов
     */
    void createSystemResourcesLabel();
    
    /**
     * @brief Создание метки статуса LLM
     */
    void createLLMStatusLabel();
    
    /**
     * @brief Форматирование строки ресурсов
     * @return Отформатированная строка
     */
    QString formatResourcesString() const;
    
    /**
     * @brief Форматирование строки LLM статуса
     * @return Отформатированная строка
     */
    QString formatLLMStatusString() const;
    
    /**
     * @brief Получение цвета для статуса LLM
     * @param status Статус LLM
     * @return QColor для отображения
     */
    static QString getLLMStatusColor(LLMStatus status);

    // Компоненты UI
    QLabel* cursorPositionLabel;    // Позиция курсора
    QLabel* executionStatusLabel;   // Статус выполнения
    QLabel* saveStatusLabel;        // Статус сохранения
    QLabel* timeLabel;              // Системное время
    QLabel* systemResourcesLabel;   // Ресурсы системы
    QLabel* llmStatusLabel;         // Статус LLM
    
    // Данные
    int currentLine;                // Текущая строка курсора
    int currentColumn;              // Текущий столбец курсора
    QString executionFile;          // Файл выполнения
    int executionLine;              // Строка выполнения
    bool fileSaved;                 // Статус сохранения
    QTime currentTime;              // Текущее время
    double cpuUsagePercent;         // Загрузка CPU (%)
    double memoryUsageMB;           // Использование памяти (МБ)
    double diskUsagePercent;        // Использование диска (%)
    bool windowActive;              // Активность окна
    
    // LLM данные
    QString llmModelName;           // Название модели LLM
    int llmFreeContext;             // Свободный контекст
    int llmTotalContext;            // Полный контекст
    LLMStatus llmStatus;            // Статус LLM
    
    // Таймер для обновления времени и ресурсов
    QTimer* updateTimer;
};

} // namespace proxima

#endif // CENTAURI_STATUSBAR_H
