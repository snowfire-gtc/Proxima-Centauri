#ifndef CENTAURI_SETTINGS_H
#define CENTAURI_SETTINGS_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QColor>
#include <QFont>
#include <QByteArray>
#include <QSettings>

namespace proxima {

enum class Theme {
    Light,
    Dark,
    System
};

enum class Language {
    English,
    Russian,
    Chinese,
    Spanish
};

struct EditorSettings {
    QFont font;
    int fontSize;
    int tabWidth;
    bool useSpaces;
    bool showLineNumbers;
    bool showWhitespace;
    bool wordWrap;
    bool autoIndent;
    bool highlightCurrentLine;
    bool codeFolding;
    Theme theme;
};

struct DebuggerSettings {
    int verboseLevel;
    bool autoBreakOnError;
    bool showTooltips;
    bool inspectVariables;
    bool watchExpressions;
    int maxWatchExpressions;
};

struct BuildSettings {
    int optimizationLevel;
    bool debugSymbols;
    bool parallelCompilation;
    bool cacheEnabled;
    bool enableCUDA;
    bool enableAVX2;
    bool enableSSE4;
    size_t maxMemory;
};

struct LLMSettings {
    QString serverURL;
    QString model;
    int maxTokens;
    double temperature;
    bool autoSuggest;
    int timeout;
};

struct GitSettings {
    QString userName;
    QString userEmail;
    bool autoCommit;
    bool autoPush;
    int autoSaveInterval;
};

struct CollaborationSettings {
    bool enabled;
    QString hostServer;
    int port;
    bool autoSync;
    bool showUserCursors;
    QString username;
};

struct WindowSettings {
    QByteArray geometry;
    QByteArray state;
    int splitterPosition;
    bool maximized;
    QStringList recentProjects;
    QStringList recentFiles;
};

class Settings : public QObject {
    Q_OBJECT
    
public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();
    
    // Singleton
    static Settings& getInstance();
    
    // Load/Save
    bool load();
    bool save();
    void reset();
    
    // Editor
    EditorSettings getEditorSettings() const { return editor; }
    void setEditorSettings(const EditorSettings& settings);
    
    // Debugger
    DebuggerSettings getDebuggerSettings() const { return debugger; }
    void setDebuggerSettings(const DebuggerSettings& settings);
    
    // Build
    BuildSettings getBuildSettings() const { return build; }
    void setBuildSettings(const BuildSettings& settings);
    
    // LLM
    LLMSettings getLLMSettings() const { return llm; }
    void setLLMSettings(const LLMSettings& settings);
    
    // Git
    GitSettings getGitSettings() const { return git; }
    void setGitSettings(const GitSettings& settings);
    
    // Collaboration
    CollaborationSettings getCollaborationSettings() const { return collaboration; }
    void setCollaborationSettings(const CollaborationSettings& settings);
    
    // Window
    WindowSettings getWindowSettings() const { return window; }
    void setWindowSettings(const WindowSettings& settings);
    
    // Convenience methods
    Theme getTheme() const { return editor.theme; }
    void setTheme(Theme theme);
    
    Language getLanguage() const { return language; }
    void setLanguage(Language lang);
    
    QString getConfigPath() const { return configPath; }
    
    // Recent files/projects
    void addRecentProject(const QString& path);
    void addRecentFile(const QString& path);
    void clearRecentProjects();
    void clearRecentFiles();
    QStringList getRecentProjects() const { return window.recentProjects; }
    QStringList getRecentFiles() const { return window.recentFiles; }
    
signals:
    void settingsChanged();
    void themeChanged(Theme theme);
    void languageChanged(Language lang);
    
private:
    void loadEditorSettings(QSettings& settings);
    void saveEditorSettings(QSettings& settings);
    void loadDebuggerSettings(QSettings& settings);
    void saveDebuggerSettings(QSettings& settings);
    void loadBuildSettings(QSettings& settings);
    void saveBuildSettings(QSettings& settings);
    void loadLLMSettings(QSettings& settings);
    void saveLLMSettings(QSettings& settings);
    void loadGitSettings(QSettings& settings);
    void saveGitSettings(QSettings& settings);
    void loadCollaborationSettings(QSettings& settings);
    void saveCollaborationSettings(QSettings& settings);
    void loadWindowSettings(QSettings& settings);
    void saveWindowSettings(QSettings& settings);
    
    QString configPath;
    Language language;
    
    EditorSettings editor;
    DebuggerSettings debugger;
    BuildSettings build;
    LLMSettings llm;
    GitSettings git;
    CollaborationSettings collaboration;
    WindowSettings window;
    
    bool modified;
};

} // namespace proxima

#endif // CENTAURI_SETTINGS_H
