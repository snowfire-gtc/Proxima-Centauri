#include "Settings.h"
#include <QDir>
#include <QStandardPaths>
#include "utils/Logger.h"

namespace proxima {

Settings::Settings(QObject *parent)
    : QObject(parent)
    , language(Language::English)
    , modified(false) {
    
    // Default config path
    configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + 
                 "/centauri.config";
    
    // Initialize with defaults
    editor.font = QFont("Consolas", 11);
    editor.fontSize = 11;
    editor.tabWidth = 4;
    editor.useSpaces = true;
    editor.showLineNumbers = true;
    editor.showWhitespace = false;
    editor.wordWrap = false;
    editor.autoIndent = true;
    editor.highlightCurrentLine = true;
    editor.codeFolding = true;
    editor.theme = Theme::Dark;
    
    debugger.verboseLevel = 2;
    debugger.autoBreakOnError = true;
    debugger.showTooltips = true;
    debugger.inspectVariables = true;
    debugger.watchExpressions = true;
    debugger.maxWatchExpressions = 10;
    
    build.optimizationLevel = 2;
    build.debugSymbols = true;
    build.parallelCompilation = true;
    build.cacheEnabled = true;
    build.enableCUDA = false;
    build.enableAVX2 = true;
    build.enableSSE4 = true;
    build.maxMemory = 4 * 1024 * 1024 * 1024; // 4GB
    
    llm.serverURL = "http://localhost:1234";
    llm.model = "local-model";
    llm.maxTokens = 2048;
    llm.temperature = 0.7;
    llm.autoSuggest = false;
    llm.timeout = 30000;
    
    git.userName = "";
    git.userEmail = "";
    git.autoCommit = false;
    git.autoPush = false;
    git.autoSaveInterval = 5;
    
    collaboration.enabled = false;
    collaboration.hostServer = "localhost";
    collaboration.port = 8080;
    collaboration.autoSync = true;
    collaboration.showUserCursors = true;
    collaboration.username = "";
    
    window.maximized = false;
    window.splitterPosition = 250;
}

Settings& Settings::getInstance() {
    static Settings instance;
    return instance;
}

bool Settings::load() {
    LOG_INFO("Loading settings from: " + configPath);
    
    QSettings settings(configPath, QSettings::IniFormat);
    
    if (!settings.contains("editor/font")) {
        // First run - use defaults
        save();
        return true;
    }
    
    loadEditorSettings(settings);
    loadDebuggerSettings(settings);
    loadBuildSettings(settings);
    loadLLMSettings(settings);
    loadGitSettings(settings);
    loadCollaborationSettings(settings);
    loadWindowSettings(settings);
    
    LOG_INFO("Settings loaded successfully");
    return true;
}

bool Settings::save() {
    LOG_INFO("Saving settings to: " + configPath);
    
    // Ensure directory exists
    QDir dir = QFileInfo(configPath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QSettings settings(configPath, QSettings::IniFormat);
    
    saveEditorSettings(settings);
    saveDebuggerSettings(settings);
    saveBuildSettings(settings);
    saveLLMSettings(settings);
    saveGitSettings(settings);
    saveCollaborationSettings(settings);
    saveWindowSettings(settings);
    
    settings.sync();
    
    LOG_INFO("Settings saved successfully");
    return true;
}

void Settings::reset() {
    // Reset all to defaults
    Settings newSettings;
    *this = newSettings;
    
    emit settingsChanged();
}

void Settings::setEditorSettings(const EditorSettings& settings) {
    editor = settings;
    modified = true;
    emit settingsChanged();
}

void Settings::setDebuggerSettings(const DebuggerSettings& settings) {
    debugger = settings;
    modified = true;
    emit settingsChanged();
}

void Settings::setBuildSettings(const BuildSettings& settings) {
    build = settings;
    modified = true;
    emit settingsChanged();
}

void Settings::setLLMSettings(const LLMSettings& settings) {
    llm = settings;
    modified = true;
    emit settingsChanged();
}

void Settings::setGitSettings(const GitSettings& settings) {
    git = settings;
    modified = true;
    emit settingsChanged();
}

void Settings::setCollaborationSettings(const CollaborationSettings& settings) {
    collaboration = settings;
    modified = true;
    emit settingsChanged();
}

void Settings::setWindowSettings(const WindowSettings& settings) {
    window = settings;
    modified = true;
}

void Settings::setTheme(Theme theme) {
    editor.theme = theme;
    modified = true;
    emit themeChanged(theme);
    emit settingsChanged();
}

void Settings::setLanguage(Language lang) {
    language = lang;
    modified = true;
    emit languageChanged(lang);
    emit settingsChanged();
}

void Settings::addRecentProject(const QString& path) {
    if (!window.recentProjects.contains(path)) {
        window.recentProjects.prepend(path);
        if (window.recentProjects.size() > 10) {
            window.recentProjects.removeLast();
        }
        modified = true;
    }
}

void Settings::addRecentFile(const QString& path) {
    if (!window.recentFiles.contains(path)) {
        window.recentFiles.prepend(path);
        if (window.recentFiles.size() > 20) {
            window.recentFiles.removeLast();
        }
        modified = true;
    }
}

void Settings::clearRecentProjects() {
    window.recentProjects.clear();
    modified = true;
}

void Settings::clearRecentFiles() {
    window.recentFiles.clear();
    modified = true;
}

// Load/Save methods implementation
void Settings::loadEditorSettings(QSettings& settings) {
    settings.beginGroup("editor");
    editor.font = settings.value("font", QFont("Consolas", 11)).value<QFont>();
    editor.fontSize = settings.value("fontSize", 11).toInt();
    editor.tabWidth = settings.value("tabWidth", 4).toInt();
    editor.useSpaces = settings.value("useSpaces", true).toBool();
    editor.showLineNumbers = settings.value("showLineNumbers", true).toBool();
    editor.showWhitespace = settings.value("showWhitespace", false).toBool();
    editor.wordWrap = settings.value("wordWrap", false).toBool();
    editor.autoIndent = settings.value("autoIndent", true).toBool();
    editor.highlightCurrentLine = settings.value("highlightCurrentLine", true).toBool();
    editor.codeFolding = settings.value("codeFolding", true).toBool();
    editor.theme = static_cast<Theme>(settings.value("theme", 1).toInt());
    settings.endGroup();
}

void Settings::saveEditorSettings(QSettings& settings) {
    settings.beginGroup("editor");
    settings.setValue("font", editor.font);
    settings.setValue("fontSize", editor.fontSize);
    settings.setValue("tabWidth", editor.tabWidth);
    settings.setValue("useSpaces", editor.useSpaces);
    settings.setValue("showLineNumbers", editor.showLineNumbers);
    settings.setValue("showWhitespace", editor.showWhitespace);
    settings.setValue("wordWrap", editor.wordWrap);
    settings.setValue("autoIndent", editor.autoIndent);
    settings.setValue("highlightCurrentLine", editor.highlightCurrentLine);
    settings.setValue("codeFolding", editor.codeFolding);
    settings.setValue("theme", static_cast<int>(editor.theme));
    settings.endGroup();
}

void Settings::loadDebuggerSettings(QSettings& settings) {
    settings.beginGroup("debugger");
    debugger.verboseLevel = settings.value("verboseLevel", 2).toInt();
    debugger.autoBreakOnError = settings.value("autoBreakOnError", true).toBool();
    debugger.showTooltips = settings.value("showTooltips", true).toBool();
    debugger.inspectVariables = settings.value("inspectVariables", true).toBool();
    debugger.watchExpressions = settings.value("watchExpressions", true).toBool();
    debugger.maxWatchExpressions = settings.value("maxWatchExpressions", 10).toInt();
    settings.endGroup();
}

void Settings::saveDebuggerSettings(QSettings& settings) {
    settings.beginGroup("debugger");
    settings.setValue("verboseLevel", debugger.verboseLevel);
    settings.setValue("autoBreakOnError", debugger.autoBreakOnError);
    settings.setValue("showTooltips", debugger.showTooltips);
    settings.setValue("inspectVariables", debugger.inspectVariables);
    settings.setValue("watchExpressions", debugger.watchExpressions);
    settings.setValue("maxWatchExpressions", debugger.maxWatchExpressions);
    settings.endGroup();
}

// Similar implementations for other settings groups...

} // namespace proxima
