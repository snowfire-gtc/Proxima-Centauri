#ifndef CENTAURI_MAINMENU_H
#define CENTAURI_MAINMENU_H

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include "app/MainWindow.h"

namespace proxima {

class MainMenu : public QMenuBar {
    Q_OBJECT
    
public:
    explicit MainMenu(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~MainMenu();
    
    // Menu access
    QMenu* getFileMenu() const { return fileMenu; }
    QMenu* getProjectMenu() const { return projectMenu; }
    QMenu* getRunMenu() const { return runMenu; }
    QMenu* getGenerateMenu() const { return generateMenu; }
    QMenu* getSettingsMenu() const { return settingsMenu; }
    QMenu* getInfoMenu() const { return infoMenu; }
    QMenu* getHelpMenu() const { return helpMenu; }
    
    // Action access
    QAction* getNewFileAction() const { return newFileAction; }
    QAction* getOpenFileAction() const { return openFileAction; }
    QAction* getSaveAction() const { return saveAction; }
    QAction* getRunAction() const { return runAction; }
    QAction* getDebugAction() const { return debugAction; }
    
signals:
    void menuActionTriggered(const QString& actionName);
    
private slots:
    void onActionTriggered();
    
private:
    void createFileMenu();
    void createProjectMenu();
    void createRunMenu();
    void createGenerateMenu();
    void createSettingsMenu();
    void createInfoMenu();
    void createHelpMenu();
    void setupShortcuts();
    void updateMenuState();
    
    MainWindow* mainWindow;
    
    // Menus
    QMenu* fileMenu;
    QMenu* projectMenu;
    QMenu* runMenu;
    QMenu* generateMenu;
    QMenu* settingsMenu;
    QMenu* infoMenu;
    QMenu* helpMenu;
    
    // File actions
    QAction* newFileAction;
    QAction* openFileAction;
    QAction* saveAction;
    QAction* saveAllAction;
    QAction* closeFileAction;
    QAction* newProjectAction;
    QAction* openProjectAction;
    QAction* exitAction;
    
    // Project actions
    QAction* projectSettingsAction;
    QAction* manifestAction;
    QAction* dependenciesAction;
    
    // Run actions
    QAction* runAction;
    QAction* debugAction;
    QAction* pauseAction;
    QAction* stopAction;
    QAction* verboseLevelAction;
    QAction* maxMemoryAction;
    QAction* deviceConfigAction;
    
    // Debug actions
    QAction* stepOverAction;
    QAction* stepIntoAction;
    QAction* stepOutAction;
    QAction* continueAction;
    QAction* runToCursorAction;
    QAction* runToExitAction;
    QAction* runToFirstMethodAction;
    QAction* toggleBreakpointAction;
    QAction* clearBreakpointsAction;
    
    // Generate actions
    QAction* llmAssistAction;
    QAction* codeSuggestionsAction;
    QAction* applyChangesAction;
    
    // Settings actions
    QAction* ideSettingsAction;
    QAction* editorSettingsAction;
    QAction* debuggerSettingsAction;
    QAction* buildSettingsAction;
    QAction* llmSettingsAction;
    QAction* gitSettingsAction;
    QAction* collaborationSettingsAction;
    
    // Info actions
    QAction* buildReportAction;
    QAction* compatibilityReportAction;
    QAction* projectStatisticsAction;
    
    // Help actions
    QAction* ideHelpAction;
    QAction* languageHelpAction;
    QAction* buildHelpAction;
    QAction* projectHelpAction;
    QAction* aboutAction;
    QAction* aboutAuthorsAction;
};

} // namespace proxima

#endif // CENTAURI_MAINMENU_H