#include "MainWindow.h"
#include "ui_editor/CodeEditor.h"
#include "ui_editor/EditorToolbar.h"
#include "ui_editor/StatusBar.h"
#include "ui_console/ConsoleWidget.h"
#include "ui_project/ProjectTree.h"
#include "ui_menu/MainMenu.h"
#include "ui_dialogs/SettingsDialog.h"
#include "ui_dialogs/BuildDialog.h"
#include "ui_dialogs/LLMAssistantDialog.h"
#include "ui_dialogs/CollaborationDialog.h"
#include "ui_search/SearchWidget.h"
#include "ui_search/ProjectSearchWidget.h"
#include "ui_search/AdvancedSearchWidget.h"
#include "core/Project.h"
#include "core/Module.h"
#include "core/CompilerConnector.h"
#include "core/AutoSaveManager.h"
#include "services/git/GitService.h"
#include "services/llm/LLMService.h"
#include "services/collaboration/CollaborationService.h"
#include "utils/Logger.h"
#include "utils/Config.h"
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabBar>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QSysInfo>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QKeySequence>
#include <QShortcut>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentProject(nullptr)
    , currentMode(IDEMode::Edit)
    , compilerConnector(nullptr)
    , gitService(nullptr)
    , llmService(nullptr)
    , collaborationService(nullptr)
    , autoSaveManager(nullptr)
    , menuBar(nullptr)
    , mainToolBar(nullptr)
    , statusBar(nullptr)
    , mainSplitter(nullptr)
    , editorSplitter(nullptr)
    , projectTree(nullptr)
    , editorTabs(nullptr)
    , consoleWidget(nullptr)
    , editorToolbar(nullptr)
    , variableInspector(nullptr)
    , editorStatusBar(nullptr)
    , fileSearchWidget(nullptr)
    , projectSearchWidget(nullptr)
    , advancedSearchWidget(nullptr)
    , searchDockWidget(nullptr)
    , searchMenu(nullptr)
    , autoSaveTimer(nullptr)
    , statusUpdateTimer(nullptr) {

    LOG_INFO("Creating Centauri IDE main window...");

    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupDockWidgets();
    setupConnections();
    createSearchMenu();
    loadSettings();

    // Инициализация сервисов
    compilerConnector = new CompilerConnector(this);
    gitService = new GitService(this);
    llmService = new LLMService(this);
    collaborationService = new CollaborationService(this);
    autoSaveManager = new AutoSaveManager(this);

    // Настройка авто-сохранения (каждые 5 минут)
    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSaveTimer);
    autoSaveTimer->start(5 * 60 * 1000); // 5 минут

    // Обновление строки состояния (каждую секунду)
    statusUpdateTimer = new QTimer(this);
    connect(statusUpdateTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    statusUpdateTimer->start(1000);

    // Подключение компилятора
    compilerConnector->connectToCompiler("proxima");

    // Подключение Git
    gitService->initialize(QDir::currentPath());

    // Инициализация виджетов поиска
    fileSearchWidget->setEditor(getCurrentEditor());
    advancedSearchWidget->setRuntime(compilerConnector->getRuntime());
    advancedSearchWidget->setDebugger(compilerConnector);
    LOG_INFO("Centauri IDE main window created successfully");
}

MainWindow::~MainWindow() {
    LOG_INFO("Destroying Centauri IDE main window...");

    saveSettings();
    closeProject();

    delete compilerConnector;
    delete gitService;
    delete llmService;
    delete collaborationService;
    delete autoSaveManager;

    LOG_INFO("Centauri IDE main window destroyed");
}

// ============================================================================
// Настройка UI
// ============================================================================

void MainWindow::setupUI() {
    setWindowTitle("Centauri IDE - Proxima");
    setMinimumSize(1024, 768);

    // Главный сплиттер (горизонтальный)
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);

    // Сплиттер редактора (вертикальный)
    editorSplitter = new QSplitter(Qt::Vertical);

    // Табы редактора
    editorTabs = new QTabWidget();
    editorTabs->setTabsClosable(true);
    editorTabs->setMovable(true);
    editorTabs->setDocumentMode(true);
    editorTabs->setElideMode(Qt::ElideMiddle);

    // Панель инструментов редактора
    editorToolbar = new EditorToolbar(this);

    // Консоль
    consoleWidget = new ConsoleWidget(this);

    editorSplitter->addWidget(editorTabs);
    editorSplitter->addWidget(consoleWidget);
    editorSplitter->setStretchFactor(0, 3);
    editorSplitter->setStretchFactor(1, 1);

    mainSplitter->addWidget(editorSplitter);

    // Восстановление геометрии из настроек
    restoreGeometry(Settings::getInstance().getWindowSettings().geometry);
    restoreState(Settings::getInstance().getWindowSettings().state);
}

void MainWindow::setupMenuBar() {
    menuBar = new MainMenu(this, this);
    setMenuBar(menuBar);
    
    // Создание меню поиска
    createSearchMenu();
}

void MainWindow::createSearchMenu() {
    searchMenu = menuBar->addMenu("Поиск");
    searchMenu->setObjectName("menuSearch");
    
    // Поиск в файле
    findInFileAction = searchMenu->addAction("Найти в файле...");
    findInFileAction->setShortcut(QKeySequence::Find);
    findInFileAction->setShortcutContext(Qt::WindowShortcut);
    connect(findInFileAction, &QAction::triggered, this, &MainWindow::onFindInFile);
    
    // Поиск по проекту
    findInProjectAction = searchMenu->addAction("Найти в проекте...");
    findInProjectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    findInProjectAction->setShortcutContext(Qt::WindowShortcut);
    connect(findInProjectAction, &QAction::triggered, this, &MainWindow::onFindInProject);
    
    // Расширенный поиск
    advancedSearchAction = searchMenu->addAction("Расширенный поиск значений...");
    advancedSearchAction->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F));
    advancedSearchAction->setShortcutContext(Qt::WindowShortcut);
    connect(advancedSearchAction, &QAction::triggered, this, &MainWindow::onAdvancedSearch);
    
    searchMenu->addSeparator();
    
    // Найти далее
    findNextAction = searchMenu->addAction("Найти далее");
    findNextAction->setShortcut(QKeySequence::FindNext);
    findNextAction->setShortcutContext(Qt::WindowShortcut);
    connect(findNextAction, &QAction::triggered, this, &MainWindow::onFindNext);
    
    // Найти ранее
    findPreviousAction = searchMenu->addAction("Найти ранее");
    findPreviousAction->setShortcut(QKeySequence::FindPrevious);
    findPreviousAction->setShortcutContext(Qt::WindowShortcut);
    connect(findPreviousAction, &QAction::triggered, this, &MainWindow::onFindPrevious);
    
    searchMenu->addSeparator();
    
    // Заменить в файле
    replaceInFileAction = searchMenu->addAction("Заменить в файле...");
    replaceInFileAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
    replaceInFileAction->setShortcutContext(Qt::WindowShortcut);
    connect(replaceInFileAction, &QAction::triggered, this, &MainWindow::onReplaceInFile);
    
    // Заменить в проекте
    replaceInProjectAction = searchMenu->addAction("Заменить в проекте...");
    replaceInProjectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    replaceInProjectAction->setShortcutContext(Qt::WindowShortcut);
    connect(replaceInProjectAction, &QAction::triggered, this, &MainWindow::onReplaceInProject);
}

void MainWindow::setupToolBar() {
    mainToolBar = addToolBar("Main Toolbar");
    mainToolBar->setMovable(false);
    mainToolBar->setIconSize(QSize(24, 24));
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Файловые операции
    mainToolBar->addAction(QIcon(":/icons/new.svg"), "New", this, &MainWindow::onNewFile);
    mainToolBar->addAction(QIcon(":/icons/open.svg"), "Open", this, &MainWindow::onOpenFile);
    mainToolBar->addAction(QIcon(":/icons/save.svg"), "Save", this, &MainWindow::onSaveFile);
    mainToolBar->addSeparator();
    
    // Поиск
    mainToolBar->addAction(QIcon(":/icons/search.svg"), "Поиск", this, &MainWindow::onFindInFile);
    mainToolBar->addAction(QIcon(":/icons/search-project.svg"), "Проект", this, &MainWindow::onFindInProject);
    mainToolBar->addAction(QIcon(":/icons/search-advanced.svg"), "Расширенный", this, &MainWindow::onAdvancedSearch);
    mainToolBar->addSeparator();
    
    // Выполнение
    mainToolBar->addAction(QIcon(":/icons/run.svg"), "Run", this, &MainWindow::onRunRelease);
    mainToolBar->addAction(QIcon(":/icons/debug.svg"), "Debug", this, &MainWindow::onRunDebug);
    mainToolBar->addAction(QIcon(":/icons/pause.svg"), "Pause", this, &MainWindow::onPause);
    mainToolBar->addAction(QIcon(":/icons/stop.svg"), "Stop", this, &MainWindow::onStop);
    mainToolBar->addSeparator();

    // Отладка
    mainToolBar->addAction(QIcon(":/icons/step-over.svg"), "Step Over", this, &MainWindow::onStepOver);
    mainToolBar->addAction(QIcon(":/icons/step-into.svg"), "Step Into", this, &MainWindow::onStepInto);
    mainToolBar->addAction(QIcon(":/icons/step-out.svg"), "Step Out", this, &MainWindow::onStepOut);
    mainToolBar->addAction(QIcon(":/icons/continue.svg"), "Continue", this, &MainWindow::onContinue);
    mainToolBar->addSeparator();

    // Инструменты
    mainToolBar->addAction(QIcon(":/icons/format.svg"), "Format", this, &MainWindow::onFormatCode);
    mainToolBar->addAction(QIcon(":/icons/llm.svg"), "AI Assist", this, &MainWindow::onLLMAssist);
    mainToolBar->addAction(QIcon(":/icons/git.svg"), "Git", this, &MainWindow::onGitOperation);
}

void MainWindow::setupStatusBar() {
    editorStatusBar = new StatusBar(this);
    setStatusBar(editorStatusBar);
}

void MainWindow::setupDockWidgets() {
    // Дерево проекта
    projectTree = new ProjectTree(this);
    QDockWidget* projectDock = new QDockWidget("Project", this);
    projectDock->setWidget(projectTree);
    projectDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, projectDock);

    // Инспектор переменных (для режима отладки)
    variableInspector = new VariableInspector(this);
    QDockWidget* variableDock = new QDockWidget("Variables", this);
    variableDock->setWidget(variableInspector);
    variableDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, variableDock);

    // Менеджер точек останова
    breakpointManager = new BreakpointManager(this);
    QDockWidget* breakpointDock = new QDockWidget("Breakpoints", this);
    breakpointDock->setWidget(breakpointManager);
    breakpointDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, breakpointDock);

    // Инициализация REPL
    REPL* repl = new REPL(this);
    repl->initialize();
    repl->setRuntime(compilerConnector->getRuntime());
    repl->setTypeChecker(new TypeChecker());
    repl->setIDEInterface(this);

    // Инициализация консоли с REPL
    consoleWidget->initializeREPL(repl);
    searchDockWidget = new QDockWidget("Поиск", this);
    searchDockWidget->setObjectName("dockSearch");
    searchDockWidget->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    
    // Создание виджетов поиска
    fileSearchWidget = new SearchWidget(this);
    projectSearchWidget = new ProjectSearchWidget(this);
    advancedSearchWidget = new AdvancedSearchWidget(this);
    
    // Табы для разных типов поиска
    QTabWidget* searchTabWidget = new QTabWidget(this);
    searchTabWidget->addTab(fileSearchWidget, "В файле");
    searchTabWidget->addTab(projectSearchWidget, "В проекте");
    searchTabWidget->addTab(advancedSearchWidget, "Расширенный");
    
    searchDockWidget->setWidget(searchTabWidget);
    searchDockWidget->setVisible(false);  // Скрыт по умолчанию
    
    addDockWidget(Qt::TopDockWidgetArea, searchDockWidget);





}

void MainWindow::setupConnections() {
    // Табы редактора
    connect(editorTabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0 && index < editorTabs->count()) {
            CodeEditor* editor = qobject_cast<CodeEditor*>(editorTabs->widget(index));
            if (editor) {
                activeFilePath = editor->getFilePath();
                editorToolbar->setEditor(editor);
                onEditorCursorPositionChanged(editor->getCurrentLine(), editor->getCurrentColumn());
                
                // Обновление редактора для виджета поиска
                if (fileSearchWidget) {
                    fileSearchWidget->setEditor(editor);
                }
            }
        }
    });

    connect(editorTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(editorTabs->widget(index));
        if (editor) {
            closeFile(editor->getFilePath());
        }
    });

    // Компилятор
    connect(compilerConnector, &CompilerConnector::connected, this, [this]() {
        consoleWidget->addBuildMessage("Compiler connected\n");
    });

    connect(compilerConnector, &CompilerConnector::disconnected, this, [this]() {
        consoleWidget->addBuildMessage("Compiler disconnected\n");
    });

    connect(compilerConnector, &CompilerConnector::compilationStarted, this, [this]() {
        consoleWidget->addBuildMessage("Compilation started...\n");
        setMode(IDEMode::Runtime);
    });

    connect(compilerConnector, &CompilerConnector::compilationProgress, this, [this](int percent, const QString& message) {
        consoleWidget->addBuildMessage(QString("Progress: %1% - %2\n").arg(percent).arg(message));
        editorStatusBar->setProgress(percent);
    });

    connect(compilerConnector, &CompilerConnector::compileComplete, this, &MainWindow::onCompilerCompileComplete);

    connect(compilerConnector, &CompilerConnector::debugEvent, this, &MainWindow::onCompilerDebugEvent);

    connect(compilerConnector, &CompilerConnector::breakpointHit, this, [this](int id, const QString& file, int line) {
        setMode(IDEMode::Pause);
        consoleWidget->addDebugMessage(QString("Breakpoint %1 hit at %2:%3\n").arg(id).arg(file).arg(line));

        // Открыть файл и перейти к строке
        if (openFile(file)) {
            CodeEditor* editor = getCurrentEditor();
            if (editor) {
                editor->goToLine(line);
                editor->setCurrentExecutionLine(line);
            }
        }

        // Обновить переменные
        compilerConnector->requestVariables();
        compilerConnector->requestCallStack();
    });

    connect(compilerConnector, &CompilerConnector::variablesUpdated, this, [this](const QMap<QString, QString>& variables) {
        variableInspector->setVariables(variables);
        
        // Обновить расширенный поиск
        if (advancedSearchWidget) {
            advancedSearchWidget->setCurrentContext(0);  // Текущий контекст
        }
    });

    connect(compilerConnector, &CompilerConnector::callStackUpdated, this, [this](const QVector<StackFrame>& stack) {
        consoleWidget->addCallStack(stack);
    });

    // Дерево проекта
    connect(projectTree, &ProjectTree::fileDoubleClicked, this, &MainWindow::openFile);
    connect(projectTree, &ProjectTree::projectOpened, this, [this](Project* project) {
        currentProject = project;
        if (project) {
            setWindowTitle("Centauri IDE - " + project->getName());
            autoSaveManager->setProjectPath(project->getPath());

            // Инициализация Git для проекта
            gitService->initialize(project->getPath());
        }
    });

    // Авто-сохранение
    connect(autoSaveManager, &AutoSaveManager::fileAutosaved, this, [this](const QString& filePath, const QString& autosavePath) {
        consoleWidget->addDebugMessage(QString("Auto-saved: %1 -> %2\n").arg(filePath).arg(autosavePath));
    });

    connect(autoSaveManager, &AutoSaveManager::autosaveError, this, [this](const QString& filePath, const QString& error) {
        consoleWidget->addErrorMessage("Auto-save error for " + filePath + ": " + error);
    });

    // Git
    connect(gitService, &GitService::errorOccurred, this, [this](const QString& error) {
        consoleWidget->addErrorMessage("Git error: " + error);
    });

    // LLM
    connect(llmService, &LLMService::errorOccurred, this, [this](const QString& error) {
        consoleWidget->addErrorMessage("LLM error: " + error);
    });
}

    
    // ========================================================================
    // Соединения для поиска
    // ========================================================================
    
    // Поиск в файле
    connect(fileSearchWidget, &SearchWidget::searchStarted, this, &MainWindow::onSearchResultSelected);
    connect(fileSearchWidget, &SearchWidget::searchFinished, this, [this](int matches) {
        statusLabel->setText(QString("Найдено %1 совпадений").arg(matches));
    });
    connect(fileSearchWidget, &SearchWidget::visibilityChanged, this, &MainWindow::onSearchWidgetVisibilityChanged);
    
    // Поиск по проекту
    connect(projectSearchWidget, &ProjectSearchWidget::searchStarted, this, [this](const QString& text, const QString& path) {
        consoleWidget->addBuildMessage(QString("Searching for '%1' in %2...\n").arg(text).arg(path));
    });
    connect(projectSearchWidget, &ProjectSearchWidget::searchFinished, this, [this](int matches, int files) {
        consoleWidget->addBuildMessage(QString("Search completed: %1 matches in %2 files\n").arg(matches).arg(files));
    });
    connect(projectSearchWidget, &ProjectSearchWidget::resultSelected, this, &MainWindow::onSearchResultSelected);
    
    // Расширенный поиск
    connect(advancedSearchWidget, &AdvancedSearchWidget::searchStarted, this, &MainWindow::onAdvancedSearchStarted);
    connect(advancedSearchWidget, &AdvancedSearchWidget::searchFinished, this, &MainWindow::onAdvancedSearchFinished);
    connect(advancedSearchWidget, &AdvancedSearchWidget::resultSelected, this, &MainWindow::onAdvancedSearchResultSelected);
    connect(advancedSearchWidget, &AdvancedSearchWidget::errorOccurred, this, [this](const QString& error) {
        consoleWidget->addErrorMessage("Advanced search error: " + error);
    });
}

// ============================================================================
// Слоты поиска
// ============================================================================

void MainWindow::onFindInFile() {
    showFileSearch();
}

void MainWindow::onFindInProject() {
    showProjectSearch();
}

void MainWindow::onAdvancedSearch() {
    showAdvancedSearch();
}

void MainWindow::onFindNext() {
    if (fileSearchWidget && fileSearchWidget->isVisible()) {
        fileSearchWidget->findNext();
    }
}

void MainWindow::onFindPrevious() {
    if (fileSearchWidget && fileSearchWidget->isVisible()) {
        fileSearchWidget->findPrevious();
    }
}

void MainWindow::onReplaceInFile() {
    if (fileSearchWidget) {
        fileSearchWidget->showReplace();
        fileSearchWidget->show();
    }
}

void MainWindow::onReplaceInProject() {
    // TODO: Реализовать замену в проекте
    QMessageBox::information(this, "Info", "Replace in project - coming soon");
}

void MainWindow::showFileSearch() {
    if (!fileSearchWidget) return;
    
    // Показать док поиска
    searchDockWidget->setVisible(true);
    searchDockWidget->raise();
    
    // Переключиться на вкладку поиска в файле
    QTabWidget* searchTabWidget = qobject_cast<QTabWidget*>(searchDockWidget->widget());
    if (searchTabWidget) {
        searchTabWidget->setCurrentWidget(fileSearchWidget);
    }
    
    // Активировать поиск
    fileSearchWidget->showFind();
    
    // Установить фокус на редактор если есть выделенный текст



// ============================================================================
// Управление проектом
// ============================================================================

bool MainWindow::newProject(const QString& path, const QString& name) {
    LOG_INFO("Creating new project: " + name.toStdString() + " at " + path.toStdString());

    if (currentProject) {
        int reply = QMessageBox::question(this, "Project Open",
            "Close current project before creating new one?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            closeProject();
        } else {
            return false;
        }
    }

    currentProject = new Project(this);

    if (!currentProject->create(path, name)) {
        QMessageBox::critical(this, "Error", "Failed to create project");
        delete currentProject;
        currentProject = nullptr;
        return false;
    }

    projectTree->loadProject(currentProject);
    autoSaveManager->setProjectPath(currentProject->getPath());

    // Инициализация Git
    gitService->initialize(currentProject->getPath());

    setWindowTitle("Centauri IDE - " + currentProject->getName());

    consoleWidget->addBuildMessage("Project created: " + name + "\n");

    emit projectOpened(currentProject);
    updateUI();

    LOG_INFO("Project created successfully");
    return true;
}

bool MainWindow::openProject(const QString& path) {
    LOG_INFO("Opening project: " + path.toStdString());

    if (currentProject) {
        int reply = QMessageBox::question(this, "Project Open",
            "Close current project before opening new one?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            closeProject();
        } else {
            return false;
        }
    }

    currentProject = new Project(this);

    if (!currentProject->load(path)) {
        QMessageBox::critical(this, "Error", "Failed to open project");
        delete currentProject;
        currentProject = nullptr;
        return false;
    }

    projectTree->loadProject(currentProject);
    autoSaveManager->setProjectPath(currentProject->getPath());

    // Инициализация Git
    gitService->initialize(currentProject->getPath());

    setWindowTitle("Centauri IDE - " + currentProject->getName());

    consoleWidget->addBuildMessage("Project opened: " + currentProject->getName() + "\n");

    // Открытие последних файлов
    Settings& settings = Settings::getInstance();
    for (const QString& filePath : settings.getRecentFiles()) {
        if (QFile::exists(filePath)) {
            openFile(filePath);
        }
    }

    emit projectOpened(currentProject);
    updateUI();

    LOG_INFO("Project opened successfully");
    return true;
}

bool MainWindow::saveProject() {
    if (!currentProject) {
        return false;
    }

    LOG_INFO("Saving project: " + currentProject->getName().toStdString());

    // Сохранение всех файлов
    saveAllFiles();

    // Сохранение проекта
    currentProject->save();

    consoleWidget->addBuildMessage("Project saved\n");

    LOG_INFO("Project saved successfully");
    return true;
}

bool MainWindow::closeProject() {
    if (!currentProject) {
        return true;
    }

    LOG_INFO("Closing project: " + currentProject->getName().toStdString());

    // Сохранение всех файлов
    saveAllFiles();

    // Закрытие всех редакторов
    while (editorTabs->count() > 0) {
        editorTabs->removeTab(0);
    }
    openFiles.clear();
    activeFilePath.clear();

    // Очистка дерева проекта
    projectTree->clear();

    // Удаление проекта
    delete currentProject;
    currentProject = nullptr;

    setWindowTitle("Centauri IDE - Proxima");

    consoleWidget->addBuildMessage("Project closed\n");

    emit projectClosed();
    updateUI();

    LOG_INFO("Project closed successfully");
    return true;
}

// ============================================================================
// Управление файлами
// ============================================================================

bool MainWindow::openFile(const QString& path) {
    LOG_INFO("Opening file: " + path.toStdString());

    // Проверка существования файла
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, "Warning", "File does not exist: " + path);
        return false;
    }

    // Проверка - уже открыт ли файл
    if (openFiles.contains(path)) {
        CodeEditor* editor = openFiles[path];
        editorTabs->setCurrentWidget(editor);
        return true;
    }

    // Создание нового редактора
    CodeEditor* editor = new CodeEditor(this);

    if (!editor->loadFile(path)) {
        QMessageBox::critical(this, "Error", "Failed to open file: " + path);
        delete editor;
        return false;
    }

    // Добавление в табы
    QString fileName = QFileInfo(path).fileName();
    int index = editorTabs->addTab(editor, fileName);
    editorTabs->setCurrentIndex(index);

    openFiles[path] = editor;
    activeFilePath = path;

    // Подключение сигналов редактора
    connect(editor, &CodeEditor::textChanged, this, &MainWindow::onEditorTextChanged);
    connect(editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::onEditorCursorPositionChanged);
    connect(editor, &CodeEditor::fileModified, this, &MainWindow::onEditorFileModified);

    // Регистрация в авто-сохранении
    autoSaveManager->registerFile(path);

    // Добавление в recent files
    Settings::getInstance().addRecentFile(path);

    emit fileOpened(path);
    updateUI();

    LOG_INFO("File opened successfully");
    return true;
}

bool MainWindow::saveFile(const QString& path) {
    if (!openFiles.contains(path)) {
        return false;
    }

    CodeEditor* editor = openFiles[path];

    if (editor->saveFile()) {
        autoSaveManager->markSaved(path);
        emit fileSaved(path);
        LOG_INFO("File saved: " + path.toStdString());
        return true;
    }

    return false;
}

bool MainWindow::saveAllFiles() {
    bool allSaved = true;

    for (auto it = openFiles.begin(); it != openFiles.end(); ++it) {
        if (!saveFile(it.key())) {
            allSaved = false;
        }
    }

    if (allSaved) {
        consoleWidget->addBuildMessage("All files saved\n");
    }

    return allSaved;
}

void MainWindow::closeFile(const QString& path) {
    if (!openFiles.contains(path)) {
        return;
    }

    CodeEditor* editor = openFiles[path];

    // Проверка на несохранённые изменения
    if (editor->isModified()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Save Changes",
            "Save changes to " + QFileInfo(path).fileName() + "?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            saveFile(path);
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    // Удаление из табов
    int index = editorTabs->indexOf(editor);
    if (index >= 0) {
        editorTabs->removeTab(index);
    }

    // Удаление из авто-сохранения
    autoSaveManager->unregisterFile(path);

    openFiles.remove(path);

    if (activeFilePath == path) {
        activeFilePath.clear();
    }

    delete editor;

    LOG_INFO("File closed: " + path.toStdString());
}

// ============================================================================
// Выполнение программы
// ============================================================================

void MainWindow::runProgram(RunMode mode) {
    if (!currentProject) {
        QMessageBox::warning(this, "Warning", "No project open");
        return;
    }

    // Сохранение всех файлов
    saveAllFiles();

    LOG_INFO("Running program in mode: " + std::to_string(static_cast<int>(mode)));

    // Установка режима
    setMode(IDEMode::Runtime);

    // Конфигурация компилятора
    CompilerConfig config;
    config.mode = mode;
    config.verboseLevel = Settings::getInstance().getDebuggerSettings().verboseLevel;
    config.maxMemory = Settings::getInstance().getBuildSettings().maxMemory;
    config.enableCUDA = Settings::getInstance().getBuildSettings().enableCUDA;
    config.enableAVX2 = Settings::getInstance().getBuildSettings().enableAVX2;
    config.enableSSE4 = Settings::getInstance().getBuildSettings().enableSSE4;
    config.debugSymbols = (mode == RunMode::Debug);
    config.optimizationLevel = (mode == RunMode::Release) ? 2 : 0;

    // Компиляция
    compilerConnector->compile(currentProject, config);

    emit executionStarted();
    updateUI();
}

void MainWindow::pauseProgram() {
    if (currentMode != IDEMode::Runtime) {
        return;
    }

    compilerConnector->pause();
    setMode(IDEMode::Pause);

    emit executionPaused();
    updateUI();
}

void MainWindow::stopProgram() {
    if (currentMode == IDEMode::Edit) {
        return;
    }

    compilerConnector->stop();
    setMode(IDEMode::Edit);

    // Очистка执行 линии
    for (auto it = openFiles.begin(); it != openFiles.end(); ++it) {
        it.value()->clearExecutionLine();
    }

    emit executionStopped();
    updateUI();
}

void MainWindow::stepOver() {
    if (currentMode != IDEMode::Pause) {
        return;
    }

    compilerConnector->stepOver();
}

void MainWindow::stepInto() {
    if (currentMode != IDEMode::Pause) {
        return;
    }

    compilerConnector->stepInto();
}

void MainWindow::stepOut() {
    if (currentMode != IDEMode::Pause) {
        return;
    }

    compilerConnector->stepOut();
}

void MainWindow::continueExecution() {
    if (currentMode != IDEMode::Pause) {
        return;
    }

    compilerConnector->continueExecution();
    setMode(IDEMode::Runtime);

    updateUI();
}

void MainWindow::runToCursor() {
    if (currentMode != IDEMode::Pause) {
        return;
    }

    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        return;
    }

    int line = editor->getCurrentLine();
    compilerConnector->runToLine(activeFilePath, line);
}

void MainWindow::runToMethodExit() {
    if (currentMode != IDEMode::Pause) {
        return;
    }

    compilerConnector->runToFunctionEnd();
}

void MainWindow::runToFirstMethod() {
    if (currentMode != IDEMode::Pause) {
        return;
    }

    compilerConnector->runToNextFunction();
}

// ============================================================================
// Точки останова
// ============================================================================

void MainWindow::addBreakpoint(const QString& file, int line, BreakpointType type, const QString& condition) {
    compilerConnector->addBreakpoint(file, line, type, condition);
    breakpointManager->addBreakpoint(file, line, type, condition);
}

void MainWindow::removeBreakpoint(int id) {
    compilerConnector->removeBreakpoint(id);
    breakpointManager->removeBreakpoint(id);

    // Обновление редактора
    for (auto it = openFiles.begin(); it != openFiles.end(); ++it) {
        it.value()->toggleBreakpoint(line);
    }
}

void MainWindow::toggleBreakpoint(const QString& file, int line) {
    compilerConnector->toggleBreakpoint(file, line);

    // Обновление редактора
    if (openFiles.contains(file)) {
        openFiles[file]->toggleBreakpoint(line);
    }
}

void MainWindow::clearBreakpoints() {
    compilerConnector->clearBreakpoints();
    breakpointManager->clearBreakpoints();

    // Обновление всех редакторов
    for (auto it = openFiles.begin(); it != openFiles.end(); ++it) {
        it.value()->clearBreakpoints();
    }
}

// ============================================================================
// Режимы IDE
// ============================================================================

IDEMode MainWindow::getMode() const {
    return currentMode;
}

void MainWindow::setMode(IDEMode mode) {
    if (currentMode == mode) {
        return;
    }

    currentMode = mode;
    emit modeChanged(mode);
    updateUI();

    LOG_INFO("IDE mode changed to: " + std::to_string(static_cast<int>(mode)));
}

CodeEditor* MainWindow::getCurrentEditor() const {
    int index = editorTabs->currentIndex();
    if (index >= 0 && index < editorTabs->count()) {
        return qobject_cast<CodeEditor*>(editorTabs->widget(index));
    }
    return nullptr;
}

// ============================================================================
// Слоты меню
// ============================================================================

void MainWindow::onNewFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "New File",
                         currentProject ? currentProject->getPath() + "/src" : QDir::homePath(),
                         "Proxima Files (*.prx);;All Files (*)");

    if (!fileName.isEmpty()) {
        // Создание файла
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            openFile(fileName);
        }
    }
}

void MainWindow::onOpenFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open File",
                         currentProject ? currentProject->getPath() : QDir::homePath(),
                         "Proxima Files (*.prx);;All Files (*)");

    if (!fileName.isEmpty()) {
        openFile(fileName);
    }
}

void MainWindow::onSaveFile() {
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        saveFile(editor->getFilePath());
        QString selectedText = editor->getSelectedCode();
        if (!selectedText.isEmpty()) {
            fileSearchWidget->setSearchText(selectedText);
        }





    }
}

void MainWindow::onSaveAllFiles() {
    saveAllFiles();
}

void MainWindow::onCloseFile() {
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        closeFile(editor->getFilePath());
    }
}

void MainWindow::onNewProject() {
    QString path = QFileDialog::getExistingDirectory(this, "New Project Location");
    if (!path.isEmpty()) {
        bool ok;
        QString name = QInputDialog::getText(this, "Project Name", "Enter project name:",
                                            QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            newProject(path, name);
        }
    }
}

void MainWindow::onOpenProject() {
    QString path = QFileDialog::getExistingDirectory(this, "Open Project");
    if (!path.isEmpty()) {
        openProject(path);
    }
}

void MainWindow::onSaveProject() {
    saveProject();
}

void MainWindow::onProjectSettings() {
    if (!currentProject) {
        QMessageBox::warning(this, "Warning", "No project open");
        return;
    }

    // Открытие диалога настроек проекта
    // TODO: Implement project settings dialog
}

void MainWindow::onRunRelease() {
    runProgram(RunMode::Release);
}

void MainWindow::onRunDebug() {
    runProgram(RunMode::Debug);
}

void MainWindow::onPause() {
    pauseProgram();
}

void MainWindow::onStop() {
    stopProgram();
}

void MainWindow::onStepOver() {
    stepOver();
}

void MainWindow::onStepInto() {
    stepInto();
}

void MainWindow::onStepOut() {
    stepOut();
}

void MainWindow::onContinue() {
    continueExecution();
}

void MainWindow::onToggleBreakpoint() {
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        int line = editor->getCurrentLine();
        toggleBreakpoint(editor->getFilePath(), line);
    }
}

void MainWindow::onClearBreakpoints() {
    clearBreakpoints();
}

void MainWindow::onFormatCode() {
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        editor->formatCode();
    }
}

void MainWindow::onGenerateDoc() {
    if (!currentProject) {
        QMessageBox::warning(this, "Warning", "No project open");
        return;
    }

    compilerConnector->generateDocumentation(currentProject);
}

void MainWindow::onLLMAssist() {
    CodeEditor* editor = getCurrentEditor();
    if (!editor) {
        QMessageBox::warning(this, "Warning", "No file open");
        return;
    }

    // Открытие диалога LLM помощника
    LLMAssistantDialog* dialog = new LLMAssistantDialog(this);
    dialog->setFile(editor->getFilePath());
    dialog->setSelection(
        editor->getSelectionStart(),
        editor->getSelectionEnd(),
        editor->getSelectedCode()
    );

    if (dialog->exec() == QDialog::Accepted) {
        // Применение изменений
        QVector<CodeModification> modifications = dialog->getModifications();
        for (const CodeModification& mod : modifications) {
            if (mod.accepted) {
                // TODO: Apply modification to editor
            }
        }
    }

    dialog->deleteLater();
}

void MainWindow::onGitOperation() {
    // Открытие меню Git операций
    QMenu gitMenu(this);
    gitMenu.addAction("Status", this, [this]() {
        GitStatus status = gitService->getStatus();
        consoleWidget->addBuildMessage("Git Status:\n" + status.branch + "\n");
    });
    gitMenu.addAction("Commit", this, [this]() {
        bool ok;
        QString message = QInputDialog::getText(this, "Commit", "Commit message:",
                                               QLineEdit::Normal, "", &ok);
        if (ok && !message.isEmpty()) {
            gitService->commit(message);
        }
    });
    gitMenu.addAction("Push", this, [this]() {
        gitService->push();
    });
    gitMenu.addAction("Pull", this, [this]() {
        gitService->pull();
    });

    gitMenu.exec(QCursor::pos());
}

void MainWindow::onSettings() {
    SettingsDialog* dialog = new SettingsDialog(this);

    if (dialog->exec() == QDialog::Accepted) {
        dialog->saveSettings();
    }

    dialog->deleteLater();
}

void MainWindow::onHelp() {
    QDesktopServices::openUrl(QUrl("https://proxima-lang.org/docs/ide"));
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Centauri IDE",
        "<h2>Centauri IDE</h2>"
        "<p>Version 1.0.0</p>"
        "<p>IDE for Proxima Programming Language</p>"
        "<p>License: GPLv3</p>"
        "<p>© 2024 Proxima Development Team</p>"
        "<p>Qt Version: " + QString(QT_VERSION_STR) + "</p>"
        "<p>OS: " + QSysInfo::prettyProductName() + "</p>");
}

// ============================================================================
// Слоты редактора
// ============================================================================

void MainWindow::onEditorTextChanged() {
    CodeEditor* editor = qobject_cast<CodeEditor*>(sender());
    if (editor) {
        autoSaveManager->markEdited(editor->getFilePath(), editor->toPlainText());
        updateStatusBar();
    }
}

void MainWindow::onEditorCursorPositionChanged(int line, int column) {
    if (editorStatusBar) {
        editorStatusBar->setCursorPosition(line, column);
    }
}

void MainWindow::onEditorFileModified(bool modified) {
    CodeEditor* editor = qobject_cast<CodeEditor*>(sender());
    if (editor) {
        int index = editorTabs->indexOf(editor);
        if (index >= 0) {
            QString title = QFileInfo(editor->getFilePath()).fileName();
            if (modified) {
                title += " *";
            }
            editorTabs->setTabText(index, title);
        }
        updateStatusBar();
    }
}

// ============================================================================
// Слоты компилятора
// ============================================================================

void MainWindow::onCompilerAnalysisComplete(const AnalysisResponse& response) {
    if (response.status == "ok") {
        // Применение подсветки типов
        for (const SymbolInfo& symbol : response.symbols) {
            if (openFiles.contains(symbol.file)) {
                QMap<int, QString> types;
                types[symbol.line] = symbol.returnType;
                openFiles[symbol.file]->applyTypeHighlighting(types);
            }
        }

        // Применение подсветки времени
        if (response.timingHints.contains("hotspot_lines")) {
            QMap<int, double> timings;
            for (int line : response.timingHints["hotspot_lines"]) {
                timings[line] = 1.0; // Пример
            }

            for (auto it = openFiles.begin(); it != openFiles.end(); ++it) {
                it.value()->applyTimingHighlighting(timings);
            }
        }
    }
}

void MainWindow::onCompilerCompileComplete(const CompileResult& result) {
    if (result.success) {
        consoleWidget->addBuildMessage("Build completed successfully in " +
                                       QString::number(result.compileTime) + "s\n");

        // Если режим выполнения - запуск
        if (currentMode == IDEMode::Runtime) {
            compilerConnector->execute(result.outputPath);
        }
    } else {
        consoleWidget->addErrorMessage("Build failed: " + result.errorMessage + "\n");

        // Отображение ошибок в редакторе
        for (const QString& error : result.errors) {
            // Парсинг ошибки и подсветка в редакторе
            // TODO: Implement error highlighting
        }

        setMode(IDEMode::Edit);
    }

    updateUI();
}

void MainWindow::onCompilerDebugEvent(const DebugEvent& event) {
    switch (event.type) {
        case DebugEventType::BreakpointHit:
            setMode(IDEMode::Pause);
            emit breakpointHit(event.breakpointId, event.file, event.line);
            consoleWidget->addDebugMessage("Breakpoint hit at " + event.file +
                                          ":" + QString::number(event.line) + "\n");
            break;

        case DebugEventType::ProgramExited:
            setMode(IDEMode::Edit);
            consoleWidget->addDebugMessage("Program exited with code: " +
                                          QString::number(event.exitCode) + "\n");

            // Очистка执行 линии
            for (auto it = openFiles.begin(); it != openFiles.end(); ++it) {
                it.value()->clearExecutionLine();
            }
            break;

        case DebugEventType::Error:
            setMode(IDEMode::Edit);
            consoleWidget->addErrorMessage("Debug error: " + event.message + "\n");
            break;

        default:
            break;
    }

    updateUI();
}

// ============================================================================
// Авто-сохранение
// ============================================================================

void MainWindow::onAutoSaveTimer() {
    autoSaveManager->saveAll();
}

// ============================================================================
// Обновление UI
// ============================================================================

void MainWindow::updateUI() {
    bool isEdit = (currentMode == IDEMode::Edit);
    bool isRuntime = (currentMode == IDEMode::Runtime);
    bool isPause = (currentMode == IDEMode::Pause);

    // Обновление toolbar
    mainToolBar->actions()[4]->setEnabled(isEdit);  // Run
    mainToolBar->actions()[5]->setEnabled(isEdit);  // Debug
    mainToolBar->actions()[6]->setEnabled(isRuntime);  // Pause
    mainToolBar->actions()[7]->setEnabled(isRuntime || isPause);  // Stop

    mainToolBar->actions()[9]->setEnabled(isPause);  // Step Over
    mainToolBar->actions()[10]->setEnabled(isPause);  // Step Into
    mainToolBar->actions()[11]->setEnabled(isPause);  // Step Out
    mainToolBar->actions()[12]->setEnabled(isPause);  // Continue

    // Обновление menu
    if (menuBar) {
        qobject_cast<MainMenu*>(menuBar)->updateMenuState();
    }

    // Обновление строки состояния
    updateStatusBar();
}

void MainWindow::updateStatusBar() {
    if (!editorStatusBar) {
        return;
    }

    // Системные ресурсы (только для активного окна)
    if (isActiveWindow()) {
        editorStatusBar->setSystemResources(
            getCPUUsage(),
            getMemoryUsage(),
            getDiskFree()
        );
    }

    // Текущее время
    editorStatusBar->setCurrentTime(QTime::currentTime());

    // Статус выполнения
    editorStatusBar->setExecutionStatus(currentMode);

    // Статус сохранения
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        editorStatusBar->setSaveStatus(!editor->isModified());
    }
}

void MainWindow::loadSettings() {
    Settings& settings = Settings::getInstance();
    settings.load();

    // Применение настроек
    setWindowTitle("Centauri IDE - Proxima " + settings.getBuildSettings().optimizationLevel);
}

void MainWindow::saveSettings() {
    Settings& settings = Settings::getInstance();

    settings.getWindowSettings().geometry = saveGeometry();
    settings.getWindowSettings().state = saveState();

    settings.save();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Сохранение всех файлов
    saveAllFiles();

    // Сохранение настроек
    saveSettings();

    // Закрытие проекта
    closeProject();

    // Остановка сервисов
    autoSaveManager->stop();

    event->accept();

    LOG_INFO("Centauri IDE closed");
}

// ============================================================================
// Вспомогательные функции
// ============================================================================

double MainWindow::getCPUUsage() const {
    // Получение загрузки CPU
    // В полной реализации - использование системных API
    return 0.0;
}

double MainWindow::getMemoryUsage() const {
    // Получение использования памяти
    // В полной реализации - использование системных API
    return 0.0;
}

double MainWindow::getDiskFree() const {
    // Получение свободного места на диске
    // В полной реализации - использование системных API
    return 0.0;
}

} // namespace proxima
