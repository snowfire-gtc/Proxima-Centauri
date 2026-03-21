#include "MainMenu.h"
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include "utils/Logger.h"

namespace proxima {

MainMenu::MainMenu(MainWindow* mainWindow, QWidget *parent)
    : QMenuBar(parent)
    , mainWindow(mainWindow) {
    
    createFileMenu();
    createProjectMenu();
    createRunMenu();
    createGenerateMenu();
    createSettingsMenu();
    createInfoMenu();
    createHelpMenu();
    setupShortcuts();
    
    connect(this, &MainMenu::menuActionTriggered, this, &MainMenu::onActionTriggered);
}

MainMenu::~MainMenu() {}

void MainMenu::createFileMenu() {
    fileMenu = addMenu(tr("&File"));
    
    newFileAction = fileMenu->addAction(tr("&New File"), mainWindow, &MainWindow::onNewFile);
    newFileAction->setIcon(QIcon(":/icons/new-file.svg"));
    
    openFileAction = fileMenu->addAction(tr("&Open File..."), mainWindow, &MainWindow::onOpenFile);
    openFileAction->setIcon(QIcon(":/icons/open.svg"));
    
    fileMenu->addSeparator();
    
    saveAction = fileMenu->addAction(tr("&Save"), mainWindow, &MainWindow::onSaveFile);
    saveAction->setIcon(QIcon(":/icons/save.svg"));
    
    saveAllAction = fileMenu->addAction(tr("Save &All"), mainWindow, &MainWindow::onSaveAllFiles);
    
    closeFileAction = fileMenu->addAction(tr("&Close File"), mainWindow, &MainWindow::onCloseFile);
    
    fileMenu->addSeparator();
    
    newProjectAction = fileMenu->addAction(tr("New &Project"), mainWindow, &MainWindow::onNewProject);
    newProjectAction->setIcon(QIcon(":/icons/project-new.svg"));
    
    openProjectAction = fileMenu->addAction(tr("Open &Project..."), mainWindow, &MainWindow::onOpenProject);
    openProjectAction->setIcon(QIcon(":/icons/project-open.svg"));
    
    fileMenu->addSeparator();
    
    exitAction = fileMenu->addAction(tr("E&xit"), qApp, &QApplication::quit);
    exitAction->setIcon(QIcon(":/icons/exit.svg"));
}

void MainMenu::createProjectMenu() {
    projectMenu = addMenu(tr("&Project"));
    
    projectSettingsAction = projectMenu->addAction(tr("Project &Settings"), mainWindow, &MainWindow::onProjectSettings);
    projectSettingsAction->setIcon(QIcon(":/icons/settings.svg"));
    
    manifestAction = projectMenu->addAction(tr("&Manifest"), this, [this]() {
        emit menuActionTriggered("manifest");
    });
    
    dependenciesAction = projectMenu->addAction(tr("&Dependencies"), this, [this]() {
        emit menuActionTriggered("dependencies");
    });
}

void MainMenu::createRunMenu() {
    runMenu = addMenu(tr("&Run"));
    
    runAction = runMenu->addAction(tr("&Run (Release)"), mainWindow, &MainWindow::onRunRelease);
    runAction->setIcon(QIcon(":/icons/run.svg"));
    
    debugAction = runMenu->addAction(tr("&Debug"), mainWindow, &MainWindow::onRunDebug);
    debugAction->setIcon(QIcon(":/icons/debug.svg"));
    
    runMenu->addSeparator();
    
    pauseAction = runMenu->addAction(tr("&Pause"), mainWindow, &MainWindow::onPause);
    pauseAction->setIcon(QIcon(":/icons/pause.svg"));
    
    stopAction = runMenu->addAction(tr("&Stop"), mainWindow, &MainWindow::onStop);
    stopAction->setIcon(QIcon(":/icons/stop.svg"));
    
    runMenu->addSeparator();
    
    stepOverAction = runMenu->addAction(tr("Step &Over"), mainWindow, &MainWindow::onStepOver);
    stepOverAction->setIcon(QIcon(":/icons/step-over.svg"));
    
    stepIntoAction = runMenu->addAction(tr("Step &Into"), mainWindow, &MainWindow::onStepInto);
    stepIntoAction->setIcon(QIcon(":/icons/step-into.svg"));
    
    stepOutAction = runMenu->addAction(tr("Step O&ut"), mainWindow, &MainWindow::onStepOut);
    stepOutAction->setIcon(QIcon(":/icons/step-out.svg"));
    
    continueAction = runMenu->addAction(tr("&Continue"), mainWindow, &MainWindow::onContinue);
    continueAction->setIcon(QIcon(":/icons/continue.svg"));
    
    runMenu->addSeparator();
    
    runToCursorAction = runMenu->addAction(tr("Run to &Cursor"), this, [this]() {
        emit menuActionTriggered("run_to_cursor");
    });
    
    runToExitAction = runMenu->addAction(tr("Run to Method E&xit"), this, [this]() {
        emit menuActionTriggered("run_to_exit");
    });
    
    runToFirstMethodAction = runMenu->addAction(tr("Run to &First Method"), this, [this]() {
        emit menuActionTriggered("run_to_first_method");
    });
    
    runMenu->addSeparator();
    
    toggleBreakpointAction = runMenu->addAction(tr("Toggle &Breakpoint"), mainWindow, &MainWindow::onToggleBreakpoint);
    toggleBreakpointAction->setIcon(QIcon(":/icons/breakpoint.svg"));
    
    clearBreakpointsAction = runMenu->addAction(tr("&Clear All Breakpoints"), mainWindow, &MainWindow::onClearBreakpoints);
    
    runMenu->addSeparator();
    
    verboseLevelAction = runMenu->addAction(tr("&Verbose Level..."), this, [this]() {
        emit menuActionTriggered("verbose_level");
    });
    
    maxMemoryAction = runMenu->addAction(tr("Max &Memory..."), this, [this]() {
        emit menuActionTriggered("max_memory");
    });
    
    deviceConfigAction = runMenu->addAction(tr("&Device Configuration..."), this, [this]() {
        emit menuActionTriggered("device_config");
    });
}

void MainMenu::createGenerateMenu() {
    generateMenu = addMenu(tr("&Generate"));
    
    llmAssistAction = generateMenu->addAction(tr("&LLM Assistant..."), mainWindow, &MainWindow::onLLMAssist);
    llmAssistAction->setIcon(QIcon(":/icons/llm.svg"));
    
    codeSuggestionsAction = generateMenu->addAction(tr("&Code Suggestions"), this, [this]() {
        emit menuActionTriggered("code_suggestions");
    });
    
    applyChangesAction = generateMenu->addAction(tr("&Apply Changes"), this, [this]() {
        emit menuActionTriggered("apply_changes");
    });
}

void MainMenu::createSettingsMenu() {
    settingsMenu = addMenu(tr("&Settings"));
    
    ideSettingsAction = settingsMenu->addAction(tr("&IDE Settings..."), mainWindow, &MainWindow::onSettings);
    ideSettingsAction->setIcon(QIcon(":/icons/settings.svg"));
    
    editorSettingsAction = settingsMenu->addAction(tr("&Editor Settings..."), this, [this]() {
        emit menuActionTriggered("editor_settings");
    });
    
    debuggerSettingsAction = settingsMenu->addAction(tr("&Debugger Settings..."), this, [this]() {
        emit menuActionTriggered("debugger_settings");
    });
    
    buildSettingsAction = settingsMenu->addAction(tr("&Build Settings..."), this, [this]() {
        emit menuActionTriggered("build_settings");
    });
    
    llmSettingsAction = settingsMenu->addAction(tr("LL&M Settings..."), this, [this]() {
        emit menuActionTriggered("llm_settings");
    });
    
    gitSettingsAction = settingsMenu->addAction(tr("&Git Settings..."), this, [this]() {
        emit menuActionTriggered("git_settings");
    });
    
    collaborationSettingsAction = settingsMenu->addAction(tr("&Collaboration Settings..."), this, [this]() {
        emit menuActionTriggered("collaboration_settings");
    });
}

void MainMenu::createInfoMenu() {
    infoMenu = addMenu(tr("&Info"));
    
    buildReportAction = infoMenu->addAction(tr("&Build Report"), this, [this]() {
        emit menuActionTriggered("build_report");
    });
    buildReportAction->setIcon(QIcon(":/icons/report.svg"));
    
    compatibilityReportAction = infoMenu->addAction(tr("&Compatibility Report"), this, [this]() {
        emit menuActionTriggered("compatibility_report");
    });
    
    projectStatisticsAction = infoMenu->addAction(tr("Project &Statistics"), this, [this]() {
        emit menuActionTriggered("project_statistics");
    });
}

void MainMenu::createHelpMenu() {
    helpMenu = addMenu(tr("&Help"));
    
    ideHelpAction = helpMenu->addAction(tr("IDE &Help"), this, [this]() {
        QDesktopServices::openUrl(QUrl("https://proxima-lang.org/docs/ide"));
    });
    ideHelpAction->setIcon(QIcon(":/icons/help.svg"));
    
    languageHelpAction = helpMenu->addAction(tr("&Language Help"), this, [this]() {
        QDesktopServices::openUrl(QUrl("https://proxima-lang.org/docs/language"));
    });
    
    buildHelpAction = helpMenu->addAction(tr("&Build Help"), this, [this]() {
        QDesktopServices::openUrl(QUrl("https://proxima-lang.org/docs/build"));
    });
    
    projectHelpAction = helpMenu->addAction(tr("Project &Help"), this, [this]() {
        emit menuActionTriggered("project_help");
    });
    
    helpMenu->addSeparator();
    
    aboutAction = helpMenu->addAction(tr("&About Centauri"), mainWindow, &MainWindow::onAbout);
    aboutAction->setIcon(QIcon(":/icons/about.svg"));
    
    aboutAuthorsAction = helpMenu->addAction(tr("About &Authors"), this, [this]() {
        emit menuActionTriggered("about_authors");
    });
}

void MainMenu::setupShortcuts() {
    newFileAction->setShortcut(QKeySequence::New);
    openFileAction->setShortcut(QKeySequence::Open);
    saveAction->setShortcut(QKeySequence::Save);
    saveAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    exitAction->setShortcut(QKeySequence::Quit);
    
    runAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F5));
    debugAction->setShortcut(QKeySequence(Qt::Key_F5));
    pauseAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Pause));
    stopAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));
    
    stepOverAction->setShortcut(QKeySequence(Qt::Key_F10));
    stepIntoAction->setShortcut(QKeySequence(Qt::Key_F11));
    stepOutAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F11));
    continueAction->setShortcut(QKeySequence(Qt::Key_F5));
    
    toggleBreakpointAction->setShortcut(QKeySequence(Qt::Key_F9));
    
    ideSettingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
    ideHelpAction->setShortcut(QKeySequence::HelpContents);
}

void MainMenu::updateMenuState() {
    IDEMode mode = mainWindow->getMode();
    
    bool isEdit = (mode == IDEMode::Edit);
    bool isRuntime = (mode == IDEMode::Runtime);
    bool isPause = (mode == IDEMode::Pause);
    
    // Enable/disable run actions based on mode
    runAction->setEnabled(isEdit);
    debugAction->setEnabled(isEdit);
    pauseAction->setEnabled(isRuntime);
    stopAction->setEnabled(isRuntime || isPause);
    
    stepOverAction->setEnabled(isPause);
    stepIntoAction->setEnabled(isPause);
    stepOutAction->setEnabled(isPause);
    continueAction->setEnabled(isPause);
    
    // Save actions
    CodeEditor* editor = mainWindow->getCurrentEditor();
    saveAction->setEnabled(editor != nullptr);
    closeFileAction->setEnabled(editor != nullptr);
}

void MainMenu::onActionTriggered() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        LOG_DEBUG("Menu action triggered: " + action->text());
    }
}

} // namespace proxima