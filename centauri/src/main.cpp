#include <QApplication>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QDir>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "app/Application.h"
#include "app/MainWindow.h"
#include "app/Settings.h"
#include "utils/Logger.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

void setupHighDPI() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

void printVersion() {
    printf("Centauri IDE Version 1.0.0\n");
    printf("Proxima Programming Language Development Environment\n");
    printf("License: GPLv3\n");
    printf("Qt Version: %s\n", QT_VERSION_STR);
    printf("Build Date: " __DATE__ " " __TIME__ "\n");
}

int main(int argc, char *argv[]) {
    // Create application
    proxima::Application app(argc, argv);
    
    // Setup high DPI
    setupHighDPI();
    
    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Centauri IDE - Proxima Programming Language");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Options
    QCommandLineOption projectOption(QStringList() << "p" << "project",
                                    "Open project at <path>", "path");
    parser.addOption(projectOption);
    
    QCommandLineOption fileOption(QStringList() << "f" << "file",
                                 "Open file at <path>", "path");
    parser.addOption(fileOption);
    
    QCommandLineOption themeOption(QStringList() << "t" << "theme",
                                  "Set theme (dark, light, system)", "theme");
    parser.addOption(themeOption);
    
    QCommandLineOption verboseOption(QStringList() << "v" << "verbose",
                                    "Enable verbose logging");
    parser.addOption(verboseOption);
    
    QCommandLineOption safeModeOption(QStringList() << "safe-mode",
                                     "Start in safe mode (no plugins)");
    parser.addOption(safeModeOption);
    
    // Process arguments
    parser.process(app);
    
    // Initialize logger
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + 
                     "/centauri.log";
    proxima::Logger::getInstance().setLogFile(logPath.toStdString());
    
    if (parser.isSet(verboseOption)) {
        proxima::Logger::getInstance().setLogLevel(proxima::LogLevel::DEBUG);
    } else {
        proxima::Logger::getInstance().setLogLevel(proxima::LogLevel::INFO);
    }
    
    LOG_INFO("========================================");
    LOG_INFO("Centauri IDE Starting");
    LOG_INFO("========================================");
    LOG_INFO("Version: 1.0.0");
    LOG_INFO("License: GPLv3");
    LOG_INFO("Qt Version: " + std::string(QT_VERSION_STR));
    LOG_INFO("Arguments: " + app.arguments().join(" ").toStdString());
    
    // Apply theme from command line
    if (parser.isSet(themeOption)) {
        QString theme = parser.value(themeOption);
        if (theme == "dark") {
            proxima::Settings::getInstance().setTheme(proxima::Theme::Dark);
        } else if (theme == "light") {
            proxima::Settings::getInstance().setTheme(proxima::Theme::Light);
        } else if (theme == "system") {
            proxima::Settings::getInstance().setTheme(proxima::Theme::System);
        }
    }
    
    // Create main window
    proxima::MainWindow* mainWindow = new proxima::MainWindow();
    
    // Open project if specified
    if (parser.isSet(projectOption)) {
        QString projectPath = parser.value(projectOption);
        if (QFileInfo(projectPath).exists()) {
            LOG_INFO("Opening project: " + projectPath.toStdString());
            mainWindow->openProject(projectPath);
        }
    }
    
    // Open file if specified
    if (parser.isSet(fileOption)) {
        QString filePath = parser.value(fileOption);
        if (QFileInfo(filePath).exists()) {
            LOG_INFO("Opening file: " + filePath.toStdString());
            mainWindow->openFile(filePath);
        }
    }
    
    // Show main window
    mainWindow->show();
    
    LOG_INFO("Centauri IDE Started Successfully");
    LOG_INFO("========================================");
    
    // Execute application
    int result = app.run();
    
    LOG_INFO("========================================");
    LOG_INFO("Centauri IDE Shutting Down");
    LOG_INFO("========================================");
    
    delete mainWindow;
    
    return result;
}