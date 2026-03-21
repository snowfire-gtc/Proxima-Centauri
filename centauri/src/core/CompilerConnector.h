#ifndef CENTAURI_COMPILERCONNECTOR_H
#define CENTAURI_COMPILERCONNECTOR_H

#include <QObject>
#include <QProcess>
#include <QMap>
#include "compiler/Protocol.h"

namespace proxima {

struct CompilerConfig {
    RunMode mode;
    int verboseLevel;
    bool enableInterpreter;      // Новый флаг
    bool enableJIT;              // Существующий
    size_t maxMemory;
    bool enableCUDA;
    bool enableAVX2;
    bool enableSSE4;
    bool debugSymbols;
    int optimizationLevel;
    QString outputPath;
};

struct CompileResult {
    bool success;
    QString errorMessage;
    QString outputPath;
    double compileTime;
    QStringList warnings;
    QStringList errors;
};

struct DebugEvent {
    DebugEventType type;
    int breakpointId;
    QString file;
    int line;
    QString message;
    int exitCode;
    QVector<StackFrame> callStack;
    QMap<QString, QString> variables;
};

class CompilerConnector : public QObject {
    Q_OBJECT
    
public:
    explicit CompilerConnector(QObject *parent = nullptr);
    ~CompilerConnector();
    
    // Connection
    bool connectToCompiler(const QString& path);
    void disconnect();
    bool isConnected() const { return connected; }
    
    // Compilation
    void compile(Project* project, const CompilerConfig& config);
    void compileFile(const QString& file, const CompilerConfig& config);
    void cancelCompilation();
    bool isCompiling() const { return compiling; }
    
    // Execution
    void execute(const QString& executable);
    void stop();
    void pause();
    void resume();
    bool isRunning() const { return running; }
    bool isPaused() const { return paused; }
    
    void interpret(Project* project, const CompilerConfig& config);
    
    // Debugging
    void stepOver();
    void stepInto();
    void stepOut();
    void continueExecution();
    void runToLine(const QString& file, int line);
    void runToFunctionEnd();
    
    // Breakpoints
    int addBreakpoint(const QString& file, int line, 
                     BreakpointType type = BreakpointType::Unconditional,
                     const QString& condition = "");
    void removeBreakpoint(int id);
    void toggleBreakpoint(const QString& file, int line);
    void clearBreakpoints();
    void clearFileBreakpoints(const QString& file);
    QVector<int> getBreakpoints() const { return breakpointIds; }
    
    // Analysis
    void analyzeFile(const QString& file, const AnalysisOptions& options);
    void generateDocumentation(Project* project);
    void formatCode(const QString& file);
    
    // Variables and state
    void requestVariables();
    void requestCallStack();
    void requestMemoryMap();
    QMap<QString, QString> getVariables() const { return currentVariables; }
    QVector<StackFrame> getCallStack() const { return currentCallStack; }
    
    // Configuration
    void setCompilerPath(const QString& path);
    QString getCompilerPath() const { return compilerPath; }
    void setWorkingDirectory(const QString& dir);
    QString getWorkingDirectory() const { return workingDirectory; }
    
    // Output
    QString getCompilerOutput() const { return compilerOutput; }
    QString getCompilerError() const { return compilerError; }
    
signals:
    void connected();
    void disconnected();
    void compilationStarted();
    void compilationProgress(int percent, const QString& message);
    void compileComplete(const CompileResult& result);
    void executionStarted();
    void executionPaused();
    void executionResumed();
    void executionStopped();
    void debugEvent(const DebugEvent& event);
    void breakpointHit(int id, const QString& file, int line);
    void analysisComplete(const AnalysisResponse& response);
    void variablesUpdated(const QMap<QString, QString>& variables);
    void callStackUpdated(const QVector<StackFrame>& stack);
    void outputReceived(const QString& output);
    void errorReceived(const QString& error);
    void connectionError(const QString& error);
    
private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onError(QProcess::ProcessError error);
    void onStateChange(QProcess::ProcessState state);
    
private:
    void setupProcess();
    void sendRequest(const Message& request);
    Message receiveResponse();
    void parseResponse(const QString& data);
    void handleCompileResponse(const QJsonObject& response);
    void handleDebugEvent(const QJsonObject& response);
    void handleAnalysisResponse(const QJsonObject& response);
    void handleVariablesResponse(const QJsonObject& response);
    void handleError(const QString& error);
    
    QProcess* process;
    QString compilerPath;
    QString workingDirectory;
    bool connected;
    bool compiling;
    bool running;
    bool paused;
    
    CompilerConfig currentConfig;
    QString compilerOutput;
    QString compilerError;
    
    QVector<int> breakpointIds;
    QMap<int, BreakpointInfo> breakpoints;
    
    QMap<QString, QString> currentVariables;
    QVector<StackFrame> currentCallStack;
    
    int requestCounter;
    QMap<int, MessageType> pendingRequests;
};

} // namespace proxima

#endif // CENTAURI_COMPILERCONNECTOR_H
