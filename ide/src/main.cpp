#include <QApplication>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QDir>
#include <QStandardPaths>
#include "app/MainWindow.h"
#include "app/Settings.h"
#include "utils/Logger.h"
#include "utils/Config.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

void setupHighDPI() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

void setupApplicationStyle() {
    QApplication::setStyle(QStyleFactory::create("Fusion"));
}

void setupDarkTheme() {
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
    QApplication::setPalette(darkPalette);
}

void setupLightTheme() {
    QApplication::setPalette(QApplication::style()->standardPalette());
}

void setupFonts() {
    // Load custom fonts
    QString fontsDir = QCoreApplication::applicationDirPath() + "/fonts";
    if (QDir(fontsDir).exists()) {
        QStringList fontFiles = QDir(fontsDir).entryList(QStringList() << "*.ttf" << "*.otf",
                                                         QDir::Files);
        for (const QString& fontFile : fontFiles) {
            QFontDatabase::addApplicationFont(fontsDir + "/" + fontFile);
        }
    }
}

void setupPaths() {
    // Set organization and application name
    QApplication::setOrganizationName("Proxima Development Team");
    QApplication::setApplicationName("Centauri IDE");
    QApplication::setApplicationVersion("1.0.0");

    // Ensure config directory exists
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);

    // Ensure data directory exists
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
}

int main(int argc, char *argv[]) {
    // Initialize logger early
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                     "/centauri.log";
    proxima::Logger::getInstance().setLogFile(logPath.toStdString());
    proxima::Logger::getInstance().setLogLevel(proxima::LogLevel::INFO);

    LOG_INFO("========================================");
    LOG_INFO("Centauri IDE Starting");
    LOG_INFO("========================================");
    LOG_INFO("Version: 1.0.0");
    LOG_INFO("License: GPLv3");
    LOG_INFO("Qt Version: " + std::string(QT_VERSION_STR));

    // Create application
    QApplication app(argc, argv);

    // Setup
    setupHighDPI();
    setupApplicationStyle();
    setupFonts();
    setupPaths();

    // Load settings
    proxima::Settings& settings = proxima::Settings::getInstance();
    settings.load();

    // Apply theme
    switch (settings.getTheme()) {
        case proxima::Theme::Dark:
            setupDarkTheme();
            LOG_INFO("Theme: Dark");
            break;
        case proxima::Theme::Light:
            setupLightTheme();
            LOG_INFO("Theme: Light");
            break;
        case proxima::Theme::System:
            // Use system theme
            LOG_INFO("Theme: System");
            break;
    }

    // Set application metadata
    app.setWindowIcon(QIcon(":/icons/centauri.ico"));
    app.setApplicationDisplayName("Centauri IDE");

    // Check for command line arguments
    QStringList args = app.arguments();
    QString projectToOpen;

    for (int i = 1; i < args.size(); i++) {
        QString arg = args[i];
        if (!arg.startsWith("-") && QFileInfo(arg).exists()) {
            projectToOpen = arg;
            break;
        }
    }

    // Create main window
    proxima::MainWindow mainWindow;

    // Open project if specified
    if (!projectToOpen.isEmpty()) {
        LOG_INFO("Opening project: " + projectToOpen.toStdString());
        mainWindow.openProject(projectToOpen);
    }

    // Show main window
    mainWindow.show();

    LOG_INFO("Centauri IDE Started Successfully");
    LOG_INFO("========================================");

    // Execute application
    int result = app.exec();

    LOG_INFO("========================================");
    LOG_INFO("Centauri IDE Shutting Down");
    LOG_INFO("========================================");

    // Save settings
    settings.save();

    LOG_INFO("Settings saved");
    LOG_INFO("Exit code: " + std::to_string(result));

    return result;
}
