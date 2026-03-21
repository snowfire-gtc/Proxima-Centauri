#ifndef CENTAURI_MAINWINDOW_H
#define CENTAURI_MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QSplitter>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QDockWidget>
#include <QTimer>
#include <QMap>
#include <QString>
#include <QVector>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QSettings>
#include <QFileSystemWatcher>

// Forward declarations
namespace Ui {
    class MainWindow;
}

namespace proxima {

// Forward declarations for Proxima classes
class Project;
class Module;
class CodeEditor;
class EditorToolbar;
class StatusBar;
class ConsoleWidget;
class ProjectTree;
class FileExplorer;
class MainMenu;
class CompilerConnector;
class GitService;
class LLMService;
class CollaborationService;
class AutoSaveManager;
class VariableInspector;
class BreakpointManager;
class VectorPlot;
class MatrixView;
class Layer3DView;
class CollectionTable;
class ObjectInspector;
class SettingsDialog;
class BuildDialog;
class LLMAssistantDialog;
class CollaborationDialog;
class BuildReportDialog;
class CompatibilityReportDialog;
class SearchWidget;           // Поиск в файле
class ProjectSearchWidget;    // Поиск по проекту
class AdvancedSearchWidget;   // Расширенный поиск значений

class REPL;  // Forward declaration

// ============================================================================
// Enums
// ============================================================================

/**
 * @brief Режимы работы IDE
 *
 * Согласно требованию ide.txt пункт 25:
 * - Режим редактирования (Edit Mode)
 * - Режим выполнения (Runtime Mode)
 * - Режим паузы (Pause Mode)
 */
enum class IDEMode {
    Edit,       // Режим редактирования
    Runtime,    // Режим выполнения
    Pause       // Режим паузы (отладка)
};

/**
 * @brief Режимы запуска программы
 *
 * Согласно требованию ide.txt пункт 26:
 * - Режим релиза
 * - Режим отладки
 * - Режим отладки - выполнение до точки останова
 * - Режим отладки - выполнение до курсора
 * - Режим отладки - выполнение до последней инструкции метода
 * - Режим отладки - выполнение до первого метода
 */
enum class RunMode {
    Release,            // Режим релиза (минимальная нагрузка на CPU)
    Debug,              // Режим отладки
    DebugToBreakpoint,  // Выполнение до точки останова
    DebugToCursor,      // Выполнение до курсора
    DebugToMethodExit,  // Выполнение до выхода из метода
    DebugToFirstMethod  // Выполнение до первого метода
};

/**
 * @brief Типы точек останова
 *
 * Согласно требованию ide.txt пункт 9:
 * - Безусловная точка останова
 * - Точка останова по условию
 */
enum class BreakpointType {
    Unconditional,  // Безусловная
    Conditional,    // По условию
    Temporary       // Временная (N итераций)
};

/**
 * @brief Режимы отображения редактора
 *
 * Согласно требованию ide.txt пункт 11е:
 * - Стандартный с подсветкой синтаксиса
 * - Подсветка типов переменных
 * - Подсветка размерности переменных
 * - Подсветка времени выполнения
 * - Подсветка частоты выполнения
 * - Отображение изменений vs репозиторий
 * - Автор правки
 * - Старость правки
 */
enum class DisplayMode {
    Standard,           // Стандартный с подсветкой синтаксиса
    TypeHighlight,      // Подсветка типов переменных
    DimensionHighlight, // Подсветка размерности
    TimingHighlight,    // Подсветка времени выполнения
    FrequencyHighlight, // Подсветка частоты выполнения
    GitDiff,           // Изменения vs репозиторий
    AuthorHighlight,    // Автор правки
    AgeHighlight        // Старость правки
};

/**
 * @brief События отладки
 */
enum class DebugEventType {
    BreakpointHit,
    StepComplete,
    ProgramExited,
    Error,
    Warning
};

/**
 * @brief Структура события отладки
 */
struct DebugEvent {
    DebugEventType type;
    int breakpointId;
    QString file;
    int line;
    QString message;
    int exitCode;
    QVector<StackFrame> callStack;
    QMap<QString, QString> variables;
};

/**
 * @brief Структура кадра стека вызовов
 */
struct StackFrame {
    QString function;
    QString file;
    int line;
    QMap<QString, QString> parameters;
    QMap<QString, QString> localVariables;
};

/**
 * @brief Структура переменной для инспектора
 */
struct VariableInfo {
    QString name;
    QString type;
    QString value;
    int size;
    bool isExpanded;
    QVector<VariableInfo> children;
};

// ============================================================================
// Главный класс окна IDE
// ============================================================================

/**
 * @brief Главное окно IDE Centauri
 *
 * Основной класс приложения, управляющий всеми компонентами IDE.
 *
 * @note Согласно требованию ide.txt:
 * - Пункт 21: В запущенном экземпляре IDE ведётся работа только с одним проектом
 * - Пункт 22: Название IDE - "Centauri"
 * - Пункт 23: IDE написана на Qt 5.15.2/C++
 * - Пункт 28: Лицензия GPLv3
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного окна
     * @param parent Родительский виджет
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор главного окна
     */
    ~MainWindow();

    // ========================================================================
    // Управление проектом
    // ========================================================================

    /**
     * @brief Создание нового проекта
     * @param path Путь к папке проекта
     * @param name Имя проекта
     * @return true если проект создан успешно
     *
     * @note Согласно требованию ide.txt пункт 2:
     * Каждый проект хранится в отдельной папке
     */
    bool newProject(const QString& path, const QString& name);

    /**
     * @brief Открытие существующего проекта
     * @param path Путь к папке проекта
     * @return true если проект открыт успешно
     */
    bool openProject(const QString& path);

    /**
     * @brief Сохранение проекта
     * @return true если проект сохранён успешно
     */
    bool saveProject();

    /**
     * @brief Закрытие проекта
     * @return true если проект закрыт успешно
     */
    bool closeProject();

    /**
     * @brief Получение текущего проекта
     * @return Указатель на текущий проект
     */
    Project* getProject() const { return currentProject; }

    // ========================================================================
    // Управление файлами
    // ========================================================================

    /**
     * @brief Открытие файла
     * @param path Путь к файлу
     * @return true если файл открыт успешно
     *
     * @note Согласно требованию ide.txt пункт 3:
     * Исходные тексты имеют расширение ".prx" и хранятся в папке "src"
     */
    bool openFile(const QString& path);

    /**
     * @brief Сохранение файла
     * @param path Путь к файлу
     * @return true если файл сохранён успешно
     */
    bool saveFile(const QString& path);

    /**
     * @brief Сохранение всех файлов
     * @return true если все файлы сохранены успешно
     */
    bool saveAllFiles();

    /**
     * @brief Закрытие файла
     * @param path Путь к файлу
     */
    void closeFile(const QString& path);

    /**
     * @brief Получение текущего редактора
     * @return Указатель на текущий редактор кода
     */
    CodeEditor* getCurrentEditor() const;


    // ========================================================================
    // Поиск
    // ========================================================================

    /**
     * @brief Показать поиск в файле
     */
    void showFileSearch();

    /**
     * @brief Показать поиск по проекту
     */
    void showProjectSearch();

    /**
     * @brief Показать расширенный поиск значений
     */
    void showAdvancedSearch();

    /**
     * @brief Получить виджет поиска в файле
     */
    SearchWidget* getFileSearchWidget() const { return fileSearchWidget; }

    /**
     * @brief Получить виджет поиска по проекту
     */
    ProjectSearchWidget* getProjectSearchWidget() const { return projectSearchWidget; }

    /**
     * @brief Получить виджет расширенного поиска
     */
    AdvancedSearchWidget* getAdvancedSearchWidget() const { return advancedSearchWidget; }

    // ========================================================================
    // Выполнение программы
    // ========================================================================

    /**
     * @brief Запуск программы
     * @param mode Режим запуска
     *
     * @note Согласно требованию ide.txt пункт 26:
     * Схемы запуска программы на выполнение
     */
    void runProgram(RunMode mode = RunMode::Release);

    /**
     * @brief Пауза выполнения
     */
    void pauseProgram();

    /**
     * @brief Остановка выполнения
     */
    void stopProgram();

    /**
     * @brief Шаг через метод (Step Over)
     */
    void stepOver();

    /**
     * @brief Шаг внутрь метода (Step Into)
     */
    void stepInto();

    /**
     * @brief Шаг из метода (Step Out)
     */
    void stepOut();

    /**
     * @brief Продолжение выполнения
     */
    void continueExecution();

    /**
     * @brief Выполнение до курсора
     */
    void runToCursor();

    /**
     * @brief Выполнение до выхода из метода
     */
    void runToMethodExit();

    /**
     * @brief Выполнение до первого метода
     */
    void runToFirstMethod();

    // ========================================================================
    // Отладка
    // ========================================================================

    /**
     * @brief Добавление точки останова
     * @param file Файл
     * @param line Номер строки
     * @param type Тип точки останова
     * @param condition Условие (для условных точек)
     *
     * @note Согласно требованию ide.txt пункт 9:
     * Создание безусловной точки останова, точки останова по условию
     */
    void addBreakpoint(const QString& file, int line,
                      BreakpointType type = BreakpointType::Unconditional,
                      const QString& condition = "");

    /**
     * @brief Удаление точки останова
     * @param id Идентификатор точки останова
     */
    void removeBreakpoint(int id);

    /**
     * @brief Переключение точки останова
     * @param file Файл
     * @param line Номер строки
     */
    void toggleBreakpoint(const QString& file, int line);

    /**
     * @brief Очистка всех точек останова
     */
    void clearBreakpoints();

    // ========================================================================
    // Режимы IDE
    // ========================================================================

    /**
     * @brief Получение текущего режима IDE
     * @return Текущий режим
     *
     * @note Согласно требованию ide.txt пункт 25:
     * IDE работает в режимах: Edit, Runtime, Pause
     */
    IDEMode getMode() const { return currentMode; }

    /**
     * @brief Установка режима IDE
     * @param mode Новый режим
     */
    void setMode(IDEMode mode);

    // ========================================================================
    // Доступ к компонентам
    // ========================================================================

    /**
     * @brief Получение дерева проекта
     * @return Указатель на дерево проекта
     */
    ProjectTree* getProjectTree() const { return projectTree; }

    /**
     * @brief Получение консоли
     * @return Указатель на консоль
     */
    ConsoleWidget* getConsole() const { return consoleWidget; }

    /**
     * @brief Получение инспектора переменных
     * @return Указатель на инспектор переменных
     */
    VariableInspector* getVariableInspector() const { return variableInspector; }

    /**
     * @brief Получение менеджера точек останова
     * @return Указатель на менеджер точек останова
     */
    BreakpointManager* getBreakpointManager() const { return breakpointManager; }

    // ========================================================================
    // Сервисы
    // ========================================================================

    /**
     * @brief Получение коннектора компилятора
     * @return Указатель на коннектор компилятора
     */
    CompilerConnector* getCompilerConnector() const { return compilerConnector; }

    /**
     * @brief Получение Git сервиса
     * @return Указатель на Git сервис
     */
    GitService* getGitService() const { return gitService; }

    /**
     * @brief Получение LLM сервиса
     * @return Указатель на LLM сервис
     */
    LLMService* getLLMService() const { return llmService; }

    /**
     * @brief Получение сервиса совместной работы
     * @return Указатель на сервис совместной работы
     */
    CollaborationService* getCollaborationService() const { return collaborationService; }

    /**
     * @brief Получение менеджера авто-сохранения
     * @return Указатель на менеджер авто-сохранения
     */
    AutoSaveManager* getAutoSaveManager() const { return autoSaveManager; }

    // ========================================================================
    // Настройки
    // ========================================================================

    /**
     * @brief Загрузка настроек IDE
     */
    void loadSettings();

    /**
     * @brief Сохранение настроек IDE
     */
    void saveSettings();

signals:
    // ========================================================================
    // Сигналы проекта
    // ========================================================================

    /**
     * @brief Сигнал открытия проекта
     * @param project Указатель на проект
     */
    void projectOpened(Project* project);

    /**
     * @brief Сигнал закрытия проекта
     */
    void projectClosed();

    // ========================================================================
    // Сигналы файлов
    // ========================================================================

    /**
     * @brief Сигнал открытия файла
     * @param path Путь к файлу
     */
    void fileOpened(const QString& path);

    /**
     * @brief Сигнал сохранения файла
     * @param path Путь к файлу
     */
    void fileSaved(const QString& path);

    // ========================================================================
    // Сигналы режима
    // ========================================================================

    /**
     * @brief Сигнал изменения режима IDE
     * @param mode Новый режим
     */
    void modeChanged(IDEMode mode);

    // ========================================================================
    // Сигналы выполнения
    // ========================================================================

    /**
     * @brief Сигнал начала выполнения
     */
    void executionStarted();

    /**
     * @brief Сигнал паузы выполнения
     */
    void executionPaused();

    /**
     * @brief Сигнал остановки выполнения
     */
    void executionStopped();

    /**
     * @brief Сигнал попадания на точку останова
     * @param id Идентификатор точки останова
     * @param file Файл
     * @param line Номер строки
     */
    void breakpointHit(int id, const QString& file, int line);

    // ========================================================================
    // Сигналы поиска
    // ========================================================================

    void searchStarted(const QString& text);
    void searchFinished(int matches);
    void advancedSearchStarted();
    void advancedSearchFinished(int matches);

private slots:
    // ========================================================================
    // Слоты меню "Файл"
    // ========================================================================

    /**
     * @brief Создание нового файла
     *
     * @note Согласно требованию ide.txt пункт 20а:
     * Создание, удаление, сохранение файла
     */
    void onNewFile();

    /**
     * @brief Открытие файла
     */
    void onOpenFile();

    /**
     * @brief Сохранение файла
     */
    void onSaveFile();

    /**
     * @brief Сохранение всех файлов
     */
    void onSaveAllFiles();

    /**
     * @brief Закрытие файла
     */
    void onCloseFile();

    // ========================================================================
    // Слоты меню "Проект"
    // ========================================================================

    /**
     * @brief Создание нового проекта
     *
     * @note Согласно требованию ide.txt пункт 20а:
     * Создание проекта, открытие проекта
     */
    void onNewProject();

    /**
     * @brief Открытие проекта
     */
    void onOpenProject();

    /**
     * @brief Сохранение проекта
     */
    void onSaveProject();

    /**
     * @brief Настройки проекта
     *
     * @note Согласно требованию ide.txt пункт 20б:
     * Настройки проекта
     */
    void onProjectSettings();

    // ========================================================================
    // Слоты меню "Выполнение"
    // ========================================================================

    /**
     * @brief Запуск в режиме релиза
     *
     * @note Согласно требованию ide.txt пункт 27:
     * В режиме релиза IDE создаёт минимальную нагрузку на процессор
     */
    void onRunRelease();

    /**
     * @brief Запуск в режиме отладки
     */
    void onRunDebug();

    /**
     * @brief Пауза выполнения
     */
    void onPause();

    /**
     * @brief Остановка выполнения
     */
    void onStop();

    /**
     * @brief Шаг через метод
     */
    void onStepOver();

    /**
     * @brief Шаг внутрь метода
     */
    void onStepInto();

    /**
     * @brief Шаг из метода
     */
    void onStepOut();

    /**
     * @brief Продолжение выполнения
     */
    void onContinue();

    /**
     * @brief Переключение точки останова
     */
    void onToggleBreakpoint();

    /**
     * @brief Очистка всех точек останова
     */
    void onClearBreakpoints();


    // ========================================================================
    // Слоты меню "Поиск"
    // ========================================================================

    void onFindInFile();              // Ctrl+F
    void onFindInProject();           // Ctrl+Shift+F
    void onAdvancedSearch();          // Ctrl+Alt+F
    void onFindNext();                // F3
    void onFindPrevious();            // Shift+F3
    void onReplaceInFile();           // Ctrl+H
    void onReplaceInProject();        // Ctrl+Shift+H

    // ========================================================================
    // Слоты меню "Генерация"
    // ========================================================================

    /**
     * @brief Форматирование кода
     *
     * @note Согласно требованию ide.txt пункт 11в:
     * Для автоформатирования исходного текста
     */
    void onFormatCode();

    /**
     * @brief Генерация документации
     *
     * @note Согласно требованию ide.txt пункт 20ж:
     * Помощь по проекту (сформированное при запуске сборки с аргументом "doc")
     */
    void onGenerateDoc();

    /**
     * @brief Помощь LLM
     *
     * @note Согласно требованию ide.txt пункт 20г и 29:
     * Окно запроса к LLM для помощи в ИИ-генерации кода
     */
    void onLLMAssist();

    /**
     * @brief Git операции
     *
     * @note Согласно требованию ide.txt пункт 17:
     * В IDE интегрировано взаимодействие с GIT
     */
    void onGitOperation();

    // ========================================================================
    // Слоты меню "Настройки"
    // ========================================================================

    /**
     * @brief Настройки IDE
     *
     * @note Согласно требованию ide.txt пункт 20д:
     * Настройка параметров IDE и времени выполнения
     */
    void onSettings();

    // ========================================================================
    // Слоты меню "Сведения"
    // ========================================================================

    /**
     * @brief Отчёт о сборке
     *
     * @note Согласно требованию ide.txt пункт 20е:
     * Сведения о сборке - диалоговое окно для формирования отчёта о сборке
     */
    void onBuildReport();

    /**
     * @brief Отчёт о совместимости
     *
     * @note Согласно требованию ide.txt пункт 20е:
     * Сведения о совместимости - Построение отчёта о совместимости кода проекта
     */
    void onCompatibilityReport();

    // ========================================================================
    // Слоты меню "Помощь"
    // ========================================================================

    /**
     * @brief Помощь по IDE
     *
     * @note Согласно требованию ide.txt пункт 20ж:
     * Помощь по IDE показывает в гипертекстовом формате руководство по использованию IDE
     */
    void onIDEHelp();

    /**
     * @brief Помощь по Proxima
     *
     * @note Согласно требованию ide.txt пункт 20ж:
     * Помощь по Proxima показывает в гипертекстовом формате руководство по использованию языка
     */
    void onLanguageHelp();

    /**
     * @brief Помощь по сборке
     *
     * @note Согласно требованию ide.txt пункт 20ж:
     * Помощь по сборке показывает в гипертекстовом формате руководство по настройке и сборке
     */
    void onBuildHelp();

    /**
     * @brief О программе
     */
    void onAbout();

    /**
     * @brief Об авторах
     */
    void onAboutAuthors();

    // ========================================================================
    // Слоты редактора
    // ========================================================================

    /**
     * @brief Обработка изменения текста в редакторе
     */
    void onEditorTextChanged();

    /**
     * @brief Обработка изменения позиции курсора
     * @param line Номер строки
     * @param column Номер колонки
     */
    void onEditorCursorPositionChanged(int line, int column);

    /**
     * @brief Обработка изменения статуса файла
     * @param modified Файл изменён
     */
    void onEditorFileModified(bool modified);

    // ========================================================================
    // Слоты компилятора
    // ========================================================================

    /**
     * @brief Обработка завершения анализа
     * @param response Ответ анализа
     */
    void onCompilerAnalysisComplete(const AnalysisResponse& response);

    /**
     * @brief Обработка завершения компиляции
     * @param result Результат компиляции
     */
    void onCompilerCompileComplete(const CompileResult& result);

    /**
     * @brief Обработка события отладки
     * @param event Событие отладки
     */
    void onCompilerDebugEvent(const DebugEvent& event);

    // ========================================================================
    // Слоты авто-сохранения
    // ========================================================================

    /**
     * @brief Таймер авто-сохранения
     *
     * @note Согласно требованию ide.txt пункт 18 и 19:
     * Каждые 5 минут редактируемый файл автосохраняется в папке "autosave"
     */
    void onAutoSaveTimer();

    // ========================================================================
    // Слоты обновления UI
    // ========================================================================

    /**
     * @brief Обновление строки состояния
     */
    void updateStatusBar();

    /**
     * @brief Обновление UI в зависимости от режима
     */
    void updateUI();

    // ========================================================================
    // Слоты поиска
    // ========================================================================

    void onSearchResultSelected(const SearchResult& result);
    void onAdvancedSearchResultSelected(const AdvancedSearchResult& result);
    void onSearchWidgetVisibilityChanged(bool visible);

private:
    // ========================================================================
    // Методы настройки UI
    // ========================================================================

    /**
     * @brief Настройка пользовательского интерфейса
     */
    void setupUI();

    /**
     * @brief Настройка главного меню
     *
     * @note Согласно требованию ide.txt пункт 20:
     * Пиктографическое главное меню приложения
     */
    void setupMenuBar();

    /**
     * @brief Настройка панели инструментов
     */
    void setupToolBar();

    /**
     * @brief Настройка строки состояния
     *
     * @note Согласно требованию ide.txt пункт 12:
     * Внизу текстового редактора отображается строка состояния
     */
    void setupStatusBar();

    /**
     * @brief Настройка док-виджетов
     */
    void setupDockWidgets();

    /**
     * @brief Настройка соединений сигналов/слотов
     */
    void setupConnections();

    // ========================================================================
    // Вспомогательные методы
    // ========================================================================

    /**
     * @brief Получение загрузки CPU
     * @return Загрузка CPU в процентах
     *
     * @note Согласно требованию ide.txt пункт 12:
     * Ресурсы системы (ЦПУ, память, диск) - отображается только в активном окне
     */
    double getCPUUsage() const;

    /**
     * @brief Получение использования памяти
     * @return Использование памяти в байтах
     */
    quint64 getMemoryUsage() const;

    /**
     * @brief Получение свободного места на диске
     * @return Свободное место в байтах
     */
    quint64 getDiskFree() const;

    /**
     * @brief Обработка события закрытия окна
     * @param event Событие закрытия
     */
    void closeEvent(QCloseEvent* event) override;

    // ========================================================================
    // Компоненты UI
    // ========================================================================

    Ui::MainWindow* ui;                 // UI форма

    QMenuBar* menuBar;                  // Главное меню
    QToolBar* mainToolBar;              // Главная панель инструментов
    QStatusBar* editorStatusBar;        // Строка состояния

    QSplitter* mainSplitter;            // Главный сплиттер
    QSplitter* editorSplitter;          // Сплиттер редактора

    ProjectTree* projectTree;           // Дерево проекта
    QTabWidget* editorTabs;             // Табы редактора
    ConsoleWidget* consoleWidget;       // Консоль
    REPL* repl;                         // REPL instance

    EditorToolbar* editorToolbar;       // Панель инструментов редактора

    VariableInspector* variableInspector;   // Инспектор переменных
    BreakpointManager* breakpointManager;   // Менеджер точек останова

    // Виджеты поиска
    SearchWidget* fileSearchWidget;           // Поиск в файле
    ProjectSearchWidget* projectSearchWidget; // Поиск по проекту
    AdvancedSearchWidget* advancedSearchWidget; // Расширенный поиск значений
    QDockWidget* searchDockWidget;            // Док для поиска

    // ========================================================================
    // Сервисы
    // ========================================================================

    Project* currentProject;            // Текущий проект
    CompilerConnector* compilerConnector;   // Коннектор компилятора
    GitService* gitService;             // Git сервис
    LLMService* llmService;             // LLM сервис
    CollaborationService* collaborationService; // Сервис совместной работы
    AutoSaveManager* autoSaveManager;   // Менеджер авто-сохранения

    // ========================================================================
    // Состояние
    // ========================================================================

    IDEMode currentMode;                // Текущий режим IDE
    QString currentProjectPath;         // Путь к текущему проекту
    QString activeFilePath;             // Путь к активному файлу
    QMap<QString, CodeEditor*> openFiles;   // Открытые файлы

    // ========================================================================
    // Таймеры
    // ========================================================================

    QTimer* autoSaveTimer;              // Таймер авто-сохранения
    QTimer* statusUpdateTimer;          // Таймер обновления строки состояния

    // ========================================================================
    // Настройки
    // ========================================================================

    Settings settings;                  // Настройки IDE

    // ========================================================================
    // Меню поиска
    // ========================================================================

    QMenu* searchMenu;
    QAction* findInFileAction;
    QAction* findInProjectAction;
    QAction* advancedSearchAction;
    QAction* findNextAction;
    QAction* findPreviousAction;
    QAction* replaceInFileAction;
    QAction* replaceInProjectAction;
};

} // namespace proxima

#endif // CENTAURI_MAINWINDOW_H
