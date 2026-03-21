#ifndef CENTAURI_MAINWINDOW_H
#define CENTAURI_MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QSplitter>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QDockWidget>
#include "ui/editor/CodeEditor.h"
#include "ui/console/ConsoleWidget.h"
#include "ui/project/ProjectTree.h"
#include "ui/menu/MainMenu.h"
#include "core/Project.h"
#include "core/CompilerConnector.h"
#include "services/git/GitService.h"
#include "services/llm/LLMService.h"
#include "services/collaboration/CollaborationService.h"

namespace proxima {

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // Project management
    bool newProject(const QString& path, const QString& name);
    bool openProject(const QString& path);
    bool saveProject();
    bool closeProject();
    
    // File management
    bool openFile(const QString& path);
    bool saveFile(const QString& path);
    bool saveAllFiles();
    void closeFile(const QString& path);
    
    // Execution control
    void runProgram(RunMode mode = RunMode::Release);
    void pauseProgram();
    void stopProgram();
    void stepOver();
    void stepInto();
    void stepOut();
    void continueExecution();
    
    // Debugging
    void addBreakpoint(const QString& file, int line, 
                      BreakpointType type = BreakpointType::Unconditional,
                      const QString& condition = "");
    void removeBreakpoint(int id);
    void toggleBreakpoint(const QString& file, int line);
    
    // IDE state
    IDEMode getMode() const { return currentMode; }
    void setMode(IDEMode mode);
    
    // Accessors
    Project* getProject() const { return currentProject; }
    CodeEditor* getCurrentEditor() const;
    ConsoleWidget* getConsole() const { return consoleWidget; }
    ProjectTree* getProjectTree() const { return projectTree; }
    
signals:
    void projectOpened(Project* project);
    void projectClosed();
    void fileOpened(const QString& path);
    void fileSaved(const QString& path);
    void modeChanged(IDEMode mode);
    void executionStarted();
    void executionPaused();
    void executionStopped();
    void breakpointHit(int id, const QString& file, int line);
    
private slots:
    // File operations
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAllFiles();
    void onCloseFile();
    
    // Project operations
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onProjectSettings();
    
    // Execution
    void onRunRelease();
    void onRunDebug();
    void onPause();
    void onStop();
    void onStepOver();
    void onStepInto();
    void onStepOut();
    void onContinue();
    
    // Debugging
    void onToggleBreakpoint();
    void onClearBreakpoints();
    
    // IDE features
    void onFormatCode();
    void onGenerateDoc();
    void onLLMAssist();
    void onGitOperation();
    void onSettings();
    void onHelp();
    void onAbout();
    
    // Editor changes
    void onEditorTextChanged();
    void onEditorCursorPositionChanged(int line, int column);
    void onEditorFileModified(bool modified);
    
    // Compiler responses
    void onCompilerAnalysisComplete(const AnalysisResponse& response);
    void onCompilerCompileComplete(const CompileResult& result);
    void onCompilerDebugEvent(const DebugEvent& event);
    
    // Auto-save
    void onAutoSaveTimer();
    
private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupDockWidgets();
    void setupConnections();
    void updateUI();
    void updateStatusBar();
    void loadSettings();
    void saveSettings();
    
    // UI Components
    QMenuBar* menuBar;
    QToolBar* mainToolBar;
    QStatusBar* statusBar;
    
    QSplitter* mainSplitter;
    QSplitter* editorSplitter;
    
    ProjectTree* projectTree;
    QTabWidget* editorTabs;
    ConsoleWidget* consoleWidget;
    
    EditorToolbar* editorToolbar;
    StatusBar* editorStatusBar;
    
    // Services
    Project* currentProject;
    CompilerConnector* compilerConnector;
    GitService* gitService;
    LLMService* llmService;
    CollaborationService* collaborationService;
    AutoSaveManager* autoSaveManager;
    
    // State
    IDEMode currentMode;
    QString currentProjectPath;
    QMap<QString, CodeEditor*> openFiles;
    QString activeFilePath;
    
    // Timers
    QTimer* autoSaveTimer;
    QTimer* statusUpdateTimer;
    
    // Settings
    Config settings;
};

} // namespace proxima

#endif // CENTAURI_MAINWINDOW_H