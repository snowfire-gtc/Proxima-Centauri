#include "Application.h"
#include "MainWindow.h"
#include "Settings.h"
#include "utils/Logger.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>

namespace proxima {

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv) {
    
    setApplicationName("Centauri IDE");
    setApplicationVersion("1.0.0");
    setOrganizationName("Proxima Development Team");
    setOrganizationDomain("proxima-lang.org");
}

Application::~Application() {
    LOG_INFO("Application shutting down");
}

bool Application::initialize() {
    LOG_INFO("Initializing Centauri IDE");
    
    // Setup paths
    setupPaths();
    
    // Initialize logger
    setupLogger();
    
    // Load settings
    Settings::getInstance().load();
    
    // Check for updates
    // checkForUpdates();
    
    // Register meta types
    registerMetaTypes();
    
    // Setup styles
    setupStyles();
    
    LOG_INFO("Initialization completed");
    
    return true;
}

void Application::setupPaths() {
    // Ensure config directory exists
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    
    // Ensure data directory exists
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    // Ensure cache directory exists
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cacheDir);
}

void Application::setupLogger() {
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + 
                     "/centauri.log";
    Logger::getInstance().setLogFile(logPath.toStdString());
    Logger::getInstance().setLogLevel(LogLevel::INFO);
}

void Application::registerMetaTypes() {
    // Register custom types for Qt meta-object system
    qRegisterMetaType<RunMode>("RunMode");
    qRegisterMetaType<IDEMode>("IDEMode");
    qRegisterMetaType<BreakpointType>("BreakpointType");
    qRegisterMetaType<DebugEvent>("DebugEvent");
    qRegisterMetaType<StackFrame>("StackFrame");
    qRegisterMetaType<CompileResult>("CompileResult");
    qRegisterMetaType<AnalysisResponse>("AnalysisResponse");
}

void Application::setupStyles() {
    // Set application style
    setStyle(QStyleFactory::create("Fusion"));
    
    // Apply theme based on settings
    Theme theme = Settings::getInstance().getTheme();
    switch (theme) {
        case Theme::Dark:
            applyDarkTheme();
            break;
        case Theme::Light:
            applyLightTheme();
            break;
        case Theme::System:
            // Use system theme
            break;
    }
}

void Application::applyDarkTheme() {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    setPalette(darkPalette);
}

void Application::applyLightTheme() {
    setPalette(style()->standardPalette());
}

MainWindow* Application::createMainWindow() {
    MainWindow* mainWindow = new MainWindow();
    
    // Connect application signals
    connect(this, &Application::aboutToQuit, mainWindow, [mainWindow]() {
        mainWindow->saveSettings();
        mainWindow->closeProject();
    });
    
    return mainWindow;
}

int Application::run() {
    if (!initialize()) {
        LOG_ERROR("Failed to initialize application");
        return -1;
    }
    
    // Create main window
    MainWindow* mainWindow = createMainWindow();
    
    // Check for command line arguments
    QStringList args = arguments();
    QString projectToOpen;
    
    for (int i = 1; i < args.size(); i++) {
        QString arg = args[i];
        if (!arg.startsWith("-") && QFileInfo(arg).exists()) {
            if (arg.endsWith(".prx")) {
                // Open file
                mainWindow->openFile(arg);
            } else {
                // Open project
                projectToOpen = arg;
            }
        }
    }
    
    if (!projectToOpen.isEmpty()) {
        mainWindow->openProject(projectToOpen);
    }
    
    // Show main window
    mainWindow->show();
    
    LOG_INFO("Application started successfully");
    
    // Execute
    int result = exec();
    
    LOG_INFO("Application exited with code: " + std::to_string(result));
    
    return result;
}

} // namespace proxima