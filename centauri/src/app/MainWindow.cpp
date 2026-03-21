#include "MainWindow.h"
#include "ui/editor/EditorToolbar.h"
#include "ui/editor/StatusBar.h"
#include "utils/Logger.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QTimer>
#include <QSystemTrayIcon>

namespace proxima {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentProject(nullptr)
    , currentMode(IDEMode::Edit)
    , compilerConnector(nullptr)
    , gitService(nullptr)
    , llmService(nullptr)
    , collaborationService(nullptr)
    , autoSaveManager(nullptr) {
    
    LOG_INFO("Creating main window...");
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupDockWidgets();
    setupConnections();
    loadSettings();
    
    // Initialize services
    compilerConnector = new CompilerConnector(this);
    gitService = new GitService(this);
    llmService = new LLMService(this);
    collaborationService = new CollaborationService(this);
    autoSaveManager = new AutoSaveManager(this);
    
    // Auto-save timer (5 minutes)
    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSaveTimer);
    autoSaveTimer->start(5 * 60 * 1000); // 5 minutes
    
    // Status update timer (1 second)
    statusUpdateTimer = new QTimer(this);
    connect(statusUpdateTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    statusUpdateTimer->start(1000);
    
    LOG_INFO("Main window created successfully");
}

MainWindow::~MainWindow() {
    LOG_INFO("Destroying main window...");
    
    saveSettings();
    closeProject();
    
    delete compilerConnector;
    delete gitService;
    delete llmService;
    delete collaborationService;
    delete autoSaveManager;
}

void MainWindow::setupUI() {
    setWindowTitle("Centauri IDE - Proxima");
    setMinimumSize(1024, 768);
    
    // Main splitter
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);
    
    // Editor splitter (vertical)
    editorSplitter = new QSplitter(Qt::Vertical);
    
    // Editor tabs
    editorTabs = new QTabWidget();
    editorTabs->setTabsClosable(true);
    editorTabs->setMovable(true);
    editorTabs->setDocumentMode(true);
    
    // Editor toolbar (per tab)
    editorToolbar = new EditorToolbar(this);
    
    // Console
    consoleWidget = new ConsoleWidget(this);
    
    editorSplitter->addWidget(editorTabs);
    editorSplitter->addWidget(consoleWidget);
    editorSplitter->setStretchFactor(0, 3);
    editorSplitter->setStretchFactor(1, 1);
    
    mainSplitter->addWidget(editorSplitter);
}

void MainWindow::setupMenuBar() {
    menuBar = new MainMenu(this, this);
    setMenuBar(menuBar);
}

void MainWindow::setupToolBar() {
    mainToolBar = addToolBar("Main Toolbar");
    mainToolBar->setMovable(false);
    mainToolBar->setIconSize(QSize(24, 24));
    
    // File operations
    mainToolBar->addAction(QIcon(":/icons/new.svg"), "New", this, &MainWindow::onNewFile);
    mainToolBar->addAction(QIcon(":/icons/open.svg"), "Open", this, &MainWindow::onOpenFile);
    mainToolBar->addAction(QIcon(":/icons/save.svg"), "Save", this, &MainWindow::onSaveFile);
    mainToolBar->addSeparator();
    
    // Execution
    mainToolBar->addAction(QIcon(":/icons/run.svg"), "Run", this, &MainWindow::onRunRelease);
    mainToolBar->addAction(QIcon(":/icons/debug.svg"), "Debug", this, &MainWindow::onRunDebug);
    mainToolBar->addAction(QIcon(":/icons/pause.svg"), "Pause", this, &MainWindow::onPause);
    mainToolBar->addAction(QIcon(":/icons/stop.svg"), "Stop", this, &MainWindow::onStop);
    mainToolBar->addSeparator();
    
    // Debugging
    mainToolBar->addAction(QIcon(":/icons/step-over.svg"), "Step Over", this, &MainWindow::onStepOver);
    mainToolBar->addAction(QIcon(":/icons/step-into.svg"), "Step Into", this, &MainWindow::onStepInto);
    mainToolBar->addAction(QIcon(":/icons/step-out.svg"), "Step Out", this, &MainWindow::onStepOut);
    mainToolBar->addAction(QIcon(":/icons/continue.svg"), "Continue", this, &MainWindow::onContinue);
    mainToolBar->addSeparator();
    
    // Tools
    mainToolBar->addAction(QIcon(":/icons/format.svg"), "Format", this, &MainWindow::onFormatCode);
    mainToolBar->addAction(QIcon(":/icons/llm.svg"), "AI Assist", this, &MainWindow::onLLMAssist);
    mainToolBar->addAction(QIcon(":/icons/git.svg"), "Git", this, &MainWindow::onGitOperation);
}

void MainWindow::setupStatusBar() {
    statusBar = new StatusBar(this);
    setStatusBar(statusBar);
}

void MainWindow::setupDockWidgets() {
    // Project tree
    projectTree = new ProjectTree(this);
    QDockWidget* projectDock = new QDockWidget("Project", this);
    projectDock->setWidget(projectTree);
    projectDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, projectDock);
    
    // Variable inspector (for debug mode)
    // Will be added when needed
}

void MainWindow::setupConnections() {
    // Editor tabs
    connect(editorTabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0 && index < editorTabs->count()) {
            CodeEditor* editor = qobject_cast<CodeEditor*>(editorTabs->widget(index));
            if (editor) {
                activeFilePath = editor->getFilePath();
                editorToolbar->setEditor(editor);
                onEditorCursorPositionChanged(editor->getCurrentLine(), editor->getCurrentColumn());
            }
        }
    });
    
    connect(editorTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(editorTabs->widget(index));
        if (editor) {
            closeFile(editor->getFilePath());
        }
    });
    
    // Compiler connector
    connect(compilerConnector, &CompilerConnector::analysisComplete,
            this, &MainWindow::onCompilerAnalysisComplete);
    connect(compilerConnector, &CompilerConnector::compileComplete,
            this, &MainWindow::onCompilerCompileComplete);
    connect(compilerConnector, &CompilerConnector::debugEvent,
            this, &MainWindow::onCompilerDebugEvent);
    
    // Project tree
    connect(projectTree, &ProjectTree::fileDoubleClicked, this, &MainWindow::openFile);
}

bool MainWindow::newProject(const QString& path, const QString& name) {
    LOG_INFO("Creating new project: " + name + " at " + path);
    
    if (currentProject) {
        closeProject();
    }
    
    currentProject = new Project(this);
    if (!currentProject->create(path, name)) {
        delete currentProject;
        currentProject = nullptr;
        return false;
    }
    
    currentProjectPath = path;
    projectTree->loadProject(currentProject);
    autoSaveManager->setProjectPath(path);
    
    emit projectOpened(currentProject);
    updateUI();
    
    LOG_INFO("Project created successfully");
    return true;
}

bool MainWindow::openProject(const QString& path) {
    LOG_INFO("Opening project: " + path);
    
    if (currentProject) {
        closeProject();
    }
    
    currentProject = new Project(this);
    if (!currentProject->load(path)) {
        delete currentProject;
        currentProject = nullptr;
        QMessageBox::critical(this, "Error", "Failed to open project");
        return false;
    }
    
    currentProjectPath = path;
    projectTree->loadProject(currentProject);
    autoSaveManager->setProjectPath(path);
    
    // Initialize git if repository exists
    gitService->initialize(path);
    
    emit projectOpened(currentProject);
    updateUI();
    
    LOG_INFO("Project opened successfully");
    return true;
}

bool MainWindow::saveProject() {
    if (!currentProject) return false;
    
    saveAllFiles();
    currentProject->save();
    
    LOG_INFO("Project saved");
    return true;
}

bool MainWindow::closeProject() {
    if (!currentProject) return true;
    
    // Save all files
    saveAllFiles();
    
    // Close all editors
    while (editorTabs->count() > 0) {
        editorTabs->removeTab(0);
    }
    openFiles.clear();
    activeFilePath.clear();
    
    // Clear project tree
    projectTree->clear();
    
    // Delete project
    delete currentProject;
    currentProject = nullptr;
    currentProjectPath.clear();
    
    emit projectClosed();
    updateUI();
    
    LOG_INFO("Project closed");
    return true;
}

bool MainWindow::openFile(const QString& path) {
    LOG_INFO("Opening file: " + path);
    
    // Check if already open
    if (openFiles.contains(path)) {
        CodeEditor* editor = openFiles[path];
        editorTabs->setCurrentWidget(editor);
        return true;
    }
    
    // Create new editor
    CodeEditor* editor = new CodeEditor(this);
    if (!editor->loadFile(path)) {
        delete editor;
        return false;
    }
    
    // Add to tabs
    QString fileName = QFileInfo(path).fileName();
    int index = editorTabs->addTab(editor, fileName);
    editorTabs->setCurrentIndex(index);
    
    openFiles[path] = editor;
    activeFilePath = path;
    
    // Connect editor signals
    connect(editor, &CodeEditor::textChanged, this, &MainWindow::onEditorTextChanged);
    connect(editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::onEditorCursorPositionChanged);
    connect(editor, &CodeEditor::fileModified, this, &MainWindow::onEditorFileModified);
    
    // Register with auto-save
    autoSaveManager->registerFile(path);
    
    emit fileOpened(path);
    updateUI();
    
    LOG_INFO("File opened: " + path);
    return true;
}

bool MainWindow::saveFile(const QString& path) {
    if (!openFiles.contains(path)) return false;
    
    CodeEditor* editor = openFiles[path];
    if (editor->saveFile()) {
        autoSaveManager->markSaved(path);
        emit fileSaved(path);
        LOG_INFO("File saved: " + path);
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
    
    return allSaved;
}

void MainWindow::closeFile(const QString& path) {
    if (!openFiles.contains(path)) return;
    
    CodeEditor* editor = openFiles[path];
    
    // Check for unsaved changes
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
    
    // Remove from tabs
    int index = editorTabs->indexOf(editor);
    if (index >= 0) {
        editorTabs->removeTab(index);
    }
    
    // Unregister from auto-save
    autoSaveManager->unregisterFile(path);
    
    openFiles.remove(path);
    
    if (activeFilePath == path) {
        activeFilePath.clear();
    }
    
    delete editor;
    
    LOG_INFO("File closed: " + path);
}

void MainWindow::runProgram(RunMode mode) {
    if (!currentProject) {
        QMessageBox::warning(this, "Warning", "No project open");
        return;
    }
    
    // Save all files first
    saveAllFiles();
    
    LOG_INFO("Running program in mode: " + QString::number(static_cast<int>(mode)));
    
    // Set mode
    setMode(IDEMode::Runtime);
    
    // Configure compiler
    CompilerConfig config;
    config.mode = mode;
    config.verboseLevel = settings.verboseLevel;
    config.maxMemory = settings.maxMemory;
    config.enableCUDA = settings.enableCUDA;
    config.enableAVX2 = settings.enableAVX2;
    
    // Send compile request
    compilerConnector->compile(currentProject, config);
    
    emit executionStarted();
    updateUI();
}

void MainWindow::pauseProgram() {
    if (currentMode != IDEMode::Runtime) return;
    
    compilerConnector->pause();
    setMode(IDEMode::Pause);
    
    emit executionPaused();
    updateUI();
}

void MainWindow::stopProgram() {
    if (currentMode == IDEMode::Edit) return;
    
    compilerConnector->stop();
    setMode(IDEMode::Edit);
    
    emit executionStopped();
    updateUI();
}

void MainWindow::stepOver() {
    if (currentMode != IDEMode::Pause) return;
    compilerConnector->stepOver();
}

void MainWindow::stepInto() {
    if (currentMode != IDEMode::Pause) return;
    compilerConnector->stepInto();
}

void MainWindow::stepOut() {
    if (currentMode != IDEMode::Pause) return;
    compilerConnector->stepOut();
}

void MainWindow::continueExecution() {
    if (currentMode != IDEMode::Pause) return;
    compilerConnector->continueExecution();
    setMode(IDEMode::Runtime);
    
    updateUI();
}

void MainWindow::addBreakpoint(const QString& file, int line, 
                               BreakpointType type, const QString& condition) {
    compilerConnector->addBreakpoint(file, line, type, condition);
}

void MainWindow::removeBreakpoint(int id) {
    compilerConnector->removeBreakpoint(id);
}

void MainWindow::toggleBreakpoint(const QString& file, int line) {
    compilerConnector->toggleBreakpoint(file, line);
}

IDEMode MainWindow::getMode() const {
    return currentMode;
}

void MainWindow::setMode(IDEMode mode) {
    if (currentMode == mode) return;
    
    currentMode = mode;
    emit modeChanged(mode);
    updateUI();
    
    LOG_INFO("IDE mode changed to: " + QString::number(static_cast<int>(mode)));
}

CodeEditor* MainWindow::getCurrentEditor() const {
    int index = editorTabs->currentIndex();
    if (index >= 0 && index < editorTabs->count()) {
        return qobject_cast<CodeEditor*>(editorTabs->widget(index));
    }
    return nullptr;
}

// Slot implementations
void MainWindow::onNewFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "New File",
                         currentProjectPath + "/src",
                         "Proxima Files (*.prx);;All Files (*)");
    if (!fileName.isEmpty()) {
        // Create new file
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            openFile(fileName);
        }
    }
}

void MainWindow::onOpenFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open File",
                         currentProjectPath,
                         "Proxima Files (*.prx);;All Files (*)");
    if (!fileName.isEmpty()) {
        openFile(fileName);
    }
}

void MainWindow::onSaveFile() {
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        saveFile(editor->getFilePath());
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
        QString name = QInputDialog::getText(this, "Project Name", "Enter project name:");
        if (!name.isEmpty()) {
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
    if (currentProject) {
        // Open project settings dialog
        // TODO: Implement settings dialog
    }
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
    compilerConnector->clearBreakpoints();
}

void MainWindow::onFormatCode() {
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        editor->formatCode();
    }
}

void MainWindow::onGenerateDoc() {
    if (currentProject) {
        compilerConnector->generateDocumentation(currentProject);
    }
}

void MainWindow::onLLMAssist() {
    CodeEditor* editor = getCurrentEditor();
    if (editor) {
        // Get selected code or current line
        QString code = editor->getSelectedCode();
        if (code.isEmpty()) {
            code = editor->getCurrentLineCode();
        }
        
        // Show LLM assistant dialog
        // TODO: Implement LLM assistant dialog
        llmService->requestSuggestions(editor->getFilePath(), 
                                       editor->getSelectionStart(),
                                       editor->getSelectionEnd(),
                                       code);
    }
}

void MainWindow::onGitOperation() {
    // Show Git operations menu
    // TODO: Implement Git operations dialog
}

void MainWindow::onSettings() {
    // Open settings dialog
    // TODO: Implement settings dialog
}

void MainWindow::onHelp() {
    // Open help browser
    // TODO: Implement help browser
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Centauri IDE",
        "<h2>Centauri IDE</h2>"
        "<p>Version 1.0.0</p>"
        "<p>IDE for Proxima Programming Language</p>"
        "<p>License: GPLv3</p>"
        "<p>© 2024 Proxima Development Team</p>");
}

void MainWindow::onEditorTextChanged() {
    CodeEditor* editor = qobject_cast<CodeEditor*>(sender());
    if (editor) {
        autoSaveManager->markEdited(editor->getFilePath(), editor->toPlainText());
        updateStatusBar();
    }
}

void MainWindow::onEditorCursorPositionChanged(int line, int column) {
    statusBar->setCursorPosition(line, column);
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

void MainWindow::onCompilerAnalysisComplete(const AnalysisResponse& response) {
    // Update editor with analysis results
    // Highlight types, timing, etc.
    if (response.status == "ok") {
        for (const auto& symbol : response.symbols) {
            // Apply highlighting based on symbol type
        }
    }
}

void MainWindow::onCompilerCompileComplete(const CompileResult& result) {
    if (result.success) {
        consoleWidget->addBuildMessage("Build completed successfully in " + 
                                       QString::number(result.compileTime) + "s");
        
        // If running, execute
        if (currentMode == IDEMode::Runtime) {
            compilerConnector->execute(result.outputPath);
        }
    } else {
        consoleWidget->addErrorMessage("Build failed: " + result.errorMessage);
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
                                          ":" + QString::number(event.line));
            break;
        case DebugEventType::ProgramExited:
            setMode(IDEMode::Edit);
            consoleWidget->addDebugMessage("Program exited with code: " + 
                                          QString::number(event.exitCode));
            break;
        case DebugEventType::Error:
            setMode(IDEMode::Edit);
            consoleWidget->addErrorMessage("Debug error: " + event.message);
            break;
        default:
            break;
    }
    
    updateUI();
}

void MainWindow::onAutoSaveTimer() {
    autoSaveManager->saveAll();
}

void MainWindow::updateUI() {
    // Enable/disable actions based on mode
    bool isEdit = (currentMode == IDEMode::Edit);
    bool isRuntime = (currentMode == IDEMode::Runtime);
    bool isPause = (currentMode == IDEMode::Pause);
    
    // Update toolbar
    // TODO: Update toolbar button states
    
    // Update menu
    // TODO: Update menu item states
    
    // Update status bar
    updateStatusBar();
}

void MainWindow::updateStatusBar() {
    if (!statusBar) return;
    
    // Update system resources (only for active window)
    if (isActiveWindow()) {
        statusBar->setSystemResources(
            getCPUUsage(),
            getMemoryUsage(),
            getDiskFree()
        );
    }
    
    // Update time
    statusBar->setCurrentTime(QTime::currentTime());
    
    // Update execution status
    statusBar->setExecutionStatus(currentMode);
}

void MainWindow::loadSettings() {
    settings.loadFromFile("centauri.config");
    
    // Apply settings
    setWindowTitle("Centauri IDE - Proxima " + settings.version);
    
    // Restore window geometry
    restoreGeometry(settings.windowGeometry);
    restoreState(settings.windowState);
}

void MainWindow::saveSettings() {
    settings.windowGeometry = saveGeometry();
    settings.windowState = saveState();
    settings.saveToFile("centauri.config");
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Save all files
    saveAllFiles();
    
    // Save settings
    saveSettings();
    
    // Close project
    closeProject();
    
    // Stop services
    autoSaveManager->stop();
    
    event->accept();
    
    LOG_INFO("Centauri IDE closed");
}

// Helper functions for system resources
double MainWindow::getCPUUsage() const {
    // TODO: Implement CPU usage monitoring
    return 0.0;
}

double MainWindow::getMemoryUsage() const {
    // TODO: Implement memory usage monitoring
    return 0.0;
}

double MainWindow::getDiskFree() const {
    // TODO: Implement disk free space monitoring
    return 0.0;
}

} // namespace proxima