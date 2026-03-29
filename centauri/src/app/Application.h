#ifndef CENTAURI_APPLICATION_H
#define CENTAURI_APPLICATION_H

#include <QApplication>
#include <QString>

namespace proxima {

class MainWindow;

enum class RunMode {
    Normal,
    Debug,
    Test
};

enum class IDEMode {
    Development,
    Production,
    Testing
};

enum class BreakpointType {
    Line,
    Conditional,
    Exception
};

struct DebugEvent {
    QString message;
    int line;
    QString file;
};

struct StackFrame {
    QString function;
    QString file;
    int line;
};

enum class CompileStatus {
    Success,
    Error,
    Warning
};

struct CompileResult {
    CompileStatus status;
    QString output;
    QString errors;
    QString warnings;
};

struct AnalysisResponse {
    QString diagnostics;
    QString suggestions;
};

class Application : public QApplication {
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    ~Application();

    bool initialize();
    int run();
    MainWindow* createMainWindow();

private:
    void setupPaths();
    void setupLogger();
    void registerMetaTypes();
    void setupStyles();
    void applyDarkTheme();
    void applyLightTheme();
};

} // namespace proxima

#endif // CENTAURI_APPLICATION_H
