#include "CompilerConnector.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include "utils/Logger.h"

namespace proxima {

CompilerConnector::CompilerConnector(QObject *parent)
    : QObject(parent)
    , process(nullptr)
    , connected(false)
    , compiling(false)
    , running(false)
    , paused(false)
    , requestCounter(0) {
    
    setupProcess();
}

CompilerConnector::~CompilerConnector() {
    disconnect();
    delete process;
}

void CompilerConnector::setupProcess() {
    process = new QProcess(this);
    
    connect(process, &QProcess::started, this, &CompilerConnector::onProcessStarted);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CompilerConnector::onProcessFinished);
    connect(process, &QProcess::readyReadStandardOutput,
            this, &CompilerConnector::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError,
            this, &CompilerConnector::onReadyReadStandardError);
    connect(process, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, &CompilerConnector::onError);
    connect(process, &QProcess::stateChanged, this, &CompilerConnector::onStateChange);
}

bool CompilerConnector::connectToCompiler(const QString& path) {
    compilerPath = path;
    
    if (!QFile::exists(compilerPath)) {
        handleError("Compiler not found: " + path);
        return false;
    }
    
    // Start compiler in server mode
    QStringList args;
    args << "--ide-mode";
    args << "--protocol=collection";
    
    process->start(compilerPath, args);
    
    if (!process->waitForStarted(5000)) {
        handleError("Failed to start compiler");
        return false;
    }
    
    connected = true;
    emit connected();
    
    LOG_INFO("Connected to compiler: " + path);
    return true;
}

void CompilerConnector::disconnect() {
    if (process) {
        process->kill();
        process->waitForFinished(1000);
    }
    
    connected = false;
    compiling = false;
    running = false;
    paused = false;
    
    emit disconnected();
    
    LOG_INFO("Disconnected from compiler");
}

void CompilerConnector::compile(Project* project, const CompilerConfig& config) {
    if (!connected) {
        handleError("Not connected to compiler");
        return;
    }
    
    if (compiling) {
        handleError("Compilation already in progress");
        return;
    }
    
    currentConfig = config;
    compiling = true;
    compilerOutput.clear();
    compilerError.clear();
    
    emit compilationStarted();
    
    // Build compile request
    Message request(MessageType::COMPILE_MODULE);
    request.data.addRow({{"project_path", project->getPath()}});
    request.data.addRow({{"mode", config.mode == RunMode::Debug ? "debug" : "release"}});
    request.data.addRow({{"verbose", QString::number(config.verboseLevel)}});
    request.data.addRow({{"optimization", QString::number(config.optimizationLevel)}});
    
    if (config.enableCUDA) request.data.addRow({{"cuda", "true"}});
    if (config.enableAVX2) request.data.addRow({{"avx2", "true"}});
    if (config.enableSSE4) request.data.addRow({{"sse4", "true"}});
    if (config.debugSymbols) request.data.addRow({{"debug_symbols", "true"}});
    if (!config.outputPath.isEmpty()) request.data.addRow({{"output", config.outputPath}});
    
    sendRequest(request);
    
    LOG_INFO("Compilation started for project: " + project->getName());
}

void CompilerConnector::compileFile(const QString& file, const CompilerConfig& config) {
    // Similar to compile() but for single file
    currentConfig = config;
    compiling = true;
    
    Message request(MessageType::COMPILE_MODULE);
    request.data.addRow({{"file", file}});
    request.data.addRow({{"mode", config.mode == RunMode::Debug ? "debug" : "release"}});
    
    sendRequest(request);
}

void CompilerConnector::cancelCompilation() {
    if (compiling) {
        process->kill();
        compiling = false;
        LOG_INFO("Compilation cancelled");
    }
}

void CompilerConnector::execute(const QString& executable) {
    if (!QFile::exists(executable)) {
        handleError("Executable not found: " + executable);
        return;
    }
    
    running = true;
    paused = false;
    
    // Start execution
    QProcess* execProcess = new QProcess(this);
    execProcess->start(executable);
    
    connect(execProcess, &QProcess::readyReadStandardOutput, this, [this, execProcess]() {
        emit outputReceived(QString::fromUtf8(execProcess->readAllStandardOutput()));
    });
    
    connect(execProcess, &QProcess::readyReadStandardError, this, [this, execProcess]() {
        emit errorReceived(QString::fromUtf8(execProcess->readAllStandardError()));
    });
    
    connect(execProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, execProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        running = false;
        paused = false;
        
        DebugEvent event;
        event.type = DebugEventType::ProgramExited;
        event.exitCode = exitCode;
        
        emit debugEvent(event);
        emit executionStopped();
        
        execProcess->deleteLater();
    });
    
    emit executionStarted();
    
    LOG_INFO("Execution started: " + executable);
}

void CompilerConnector::stop() {
    if (running) {
        process->kill();
        running = false;
        paused = false;
        emit executionStopped();
        LOG_INFO("Execution stopped");
    }
}

void CompilerConnector::pause() {
    if (running && !paused) {
        // Send pause signal to debugger
        Message request(MessageType::DEBUG_START);
        request.data.addRow({{"action", "pause"}});
        sendRequest(request);
        
        paused = true;
        emit executionPaused();
        LOG_INFO("Execution paused");
    }
}

void CompilerConnector::resume() {
    if (paused) {
        Message request(MessageType::DEBUG_CONTINUE);
        sendRequest(request);
        
        paused = false;
        emit executionResumed();
        LOG_INFO("Execution resumed");
    }
}

void CompilerConnector::stepOver() {
    if (!paused) return;
    
    Message request(MessageType::DEBUG_STEP);
    request.data.addRow({{"type", "over"}});
    sendRequest(request);
    
    LOG_INFO("Step over");
}

void CompilerConnector::stepInto() {
    if (!paused) return;
    
    Message request(MessageType::DEBUG_STEP);
    request.data.addRow({{"type", "into"}});
    sendRequest(request);
    
    LOG_INFO("Step into");
}

void CompilerConnector::stepOut() {
    if (!paused) return;
    
    Message request(MessageType::DEBUG_STEP);
    request.data.addRow({{"type", "out"}});
    sendRequest(request);
    
    LOG_INFO("Step out");
}

void CompilerConnector::continueExecution() {
    if (!paused) return;
    
    Message request(MessageType::DEBUG_CONTINUE);
    sendRequest(request);
    
    paused = false;
    LOG_INFO("Continue execution");
}

void CompilerConnector::runToLine(const QString& file, int line) {
    if (!paused) return;
    
    Message request(MessageType::DEBUG_START);
    request.data.addRow({{"action", "run_to_line"}});
    request.data.addRow({{"file", file}});
    request.data.addRow({{"line", QString::number(line)}});
    sendRequest(request);
    
    LOG_INFO("Run to line: " + file + ":" + QString::number(line));
}

int CompilerConnector::addBreakpoint(const QString& file, int line,
                                     BreakpointType type, const QString& condition) {
    int id = breakpointIds.isEmpty() ? 1 : breakpointIds.last() + 1;
    breakpointIds.append(id);
    
    BreakpointInfo bp;
    bp.id = id;
    bp.file = file;
    bp.line = line;
    bp.type = type;
    bp.condition = condition;
    bp.enabled = true;
    bp.hitCount = 0;
    
    breakpoints[id] = bp;
    
    // Send to compiler
    Message request(MessageType::DEBUG_START);
    request.data.addRow({{"action", "add_breakpoint"}});
    request.data.addRow({{"id", QString::number(id)}});
    request.data.addRow({{"file", file}});
    request.data.addRow({{"line", QString::number(line)}});
    request.data.addRow({{"type", typeToString(type)}});
    if (!condition.isEmpty()) {
        request.data.addRow({{"condition", condition}});
    }
    
    sendRequest(request);
    
    emit breakpointAdded(id, file, line);
    
    LOG_INFO("Breakpoint added: " + file + ":" + QString::number(line));
    
    return id;
}

void CompilerConnector::removeBreakpoint(int id) {
    if (!breakpoints.contains(id)) return;
    
    breakpoints.remove(id);
    breakpointIds.removeAll(id);
    
    Message request(MessageType::DEBUG_START);
    request.data.addRow({{"action", "remove_breakpoint"}});
    request.data.addRow({{"id", QString::number(id)}});
    sendRequest(request);
    
    emit breakpointRemoved(id);
    
    LOG_INFO("Breakpoint removed: " + QString::number(id));
}

void CompilerConnector::toggleBreakpoint(const QString& file, int line) {
    // Find existing breakpoint
    for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
        if (it->file == file && it->line == line) {
            enableBreakpoint(it->id, !it->enabled);
            return;
        }
    }
    
    // Add new breakpoint
    addBreakpoint(file, line);
}

void CompilerConnector::clearBreakpoints() {
    for (int id : breakpointIds) {
        removeBreakpoint(id);
    }
    
    breakpoints.clear();
    breakpointIds.clear();
    
    LOG_INFO("All breakpoints cleared");
}

void CompilerConnector::clearFileBreakpoints(const QString& file) {
    QVector<int> toRemove;
    
    for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
        if (it->file == file) {
            toRemove.append(it->id);
        }
    }
    
    for (int id : toRemove) {
        removeBreakpoint(id);
    }
}

void CompilerConnector::analyzeFile(const QString& file, const AnalysisOptions& options) {
    Message request(MessageType::ANALYZE_FILE);
    request.data.addRow({{"file", file}});
    request.data.addRow({{"infer_types", options.inferTypes ? "true" : "false"}});
    request.data.addRow({{"highlight_timing", options.highlightTiming ? "true" : "false"}});
    if (!options.branch.isEmpty()) {
        request.data.addRow({{"branch", options.branch}});
    }
    
    sendRequest(request);
    
    LOG_INFO("File analysis requested: " + file);
}

void CompilerConnector::generateDocumentation(Project* project) {
    Message request(MessageType::GENERATE_DOC);
    request.data.addRow({{"project_path", project->getPath()}});
    request.data.addRow({{"format", "html"}});
    
    sendRequest(request);
    
    LOG_INFO("Documentation generation requested for: " + project->getName());
}

void CompilerConnector::formatCode(const QString& file) {
    Message request(MessageType::FORMAT_CODE);
    request.data.addRow({{"file", file}});
    
    sendRequest(request);
}

void CompilerConnector::requestVariables() {
    if (!paused) return;
    
    Message request(MessageType::GET_VARIABLES);
    sendRequest(request);
}

void CompilerConnector::requestCallStack() {
    if (!paused) return;
    
    Message request(MessageType::GET_CALLSTACK);
    sendRequest(request);
}

void CompilerConnector::requestMemoryMap() {
    if (!paused) return;
    
    Message request(MessageType::GET_VARIABLES);
    request.data.addRow({{"include_memory", "true"}});
    sendRequest(request);
}

void CompilerConnector::setCompilerPath(const QString& path) {
    compilerPath = path;
}

void CompilerConnector::setWorkingDirectory(const QString& dir) {
    workingDirectory = dir;
    process->setWorkingDirectory(workingDirectory);
}

void CompilerConnector::sendRequest(const Message& request) {
    if (!connected) {
        handleError("Not connected to compiler");
        return;
    }
    
    requestCounter++;
    pendingRequests[requestCounter] = request.type;
    
    QString data = request.serialize();
    process->write(data.toUtf8());
    process->write("\n");
    
    LOG_DEBUG("Request sent: " + QString::number(requestCounter));
}

Message CompilerConnector::receiveResponse() {
    // Read response from compiler
    QByteArray data = process->readAllStandardOutput();
    QString responseStr = QString::fromUtf8(data);
    
    Message response = Message::deserialize(responseStr);
    return response;
}

void CompilerConnector::parseResponse(const QString& data) {
    Message response = Message::deserialize(data);
    
    switch (response.type) {
        case MessageType::COMPILE_RESULT:
            handleCompileResponse(response.data.toJsonObject());
            break;
        case MessageType::DEBUG_EVENT:
            handleDebugEvent(response.data.toJsonObject());
            break;
        case MessageType::ANALYSIS_RESULT:
            handleAnalysisResponse(response.data.toJsonObject());
            break;
        case MessageType::VARIABLES_DATA:
            handleVariablesResponse(response.data.toJsonObject());
            break;
        case MessageType::ERROR_RESPONSE:
            handleError(response.data.get("message", ""));
            break;
        default:
            break;
    }
}

void CompilerConnector::handleCompileResponse(const QJsonObject& response) {
    compiling = false;
    
    CompileResult result;
    result.success = response["status"].toString() == "ok";
    result.errorMessage = response["error"].toString();
    result.outputPath = response["output_path"].toString();
    result.compileTime = response["compile_time"].toDouble();
    
    // Parse warnings
    QJsonArray warnings = response["warnings"].toArray();
    for (const QJsonValue& w : warnings) {
        result.warnings.append(w.toString());
    }
    
    // Parse errors
    QJsonArray errors = response["errors"].toArray();
    for (const QJsonValue& e : errors) {
        result.errors.append(e.toString());
    }
    
    emit compileComplete(result);
    
    if (result.success) {
        LOG_INFO("Compilation completed successfully in " + 
                QString::number(result.compileTime) + "s");
    } else {
        LOG_ERROR("Compilation failed: " + result.errorMessage);
    }
}

void CompilerConnector::handleDebugEvent(const QJsonObject& response) {
    DebugEvent event;
    
    QString typeStr = response["type"].toString();
    if (typeStr == "breakpoint") {
        event.type = DebugEventType::BreakpointHit;
        event.breakpointId = response["breakpoint_id"].toInt();
        event.file = response["file"].toString();
        event.line = response["line"].toInt();
        
        emit breakpointHit(event.breakpointId, event.file, event.line);
    } else if (typeStr == "exit") {
        event.type = DebugEventType::ProgramExited;
        event.exitCode = response["exit_code"].toInt();
    } else if (typeStr == "error") {
        event.type = DebugEventType::Error;
        event.message = response["message"].toString();
    }
    
    // Parse call stack
    QJsonArray stack = response["call_stack"].toArray();
    for (const QJsonValue& frame : stack) {
        QJsonObject frameObj = frame.toObject();
        StackFrame sf;
        sf.function = frameObj["function"].toString();
        sf.filename = frameObj["file"].toString();
        sf.line = frameObj["line"].toInt();
        event.callStack.append(sf);
    }
    
    // Parse variables
    QJsonObject vars = response["variables"].toObject();
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        event.variables[it.key()] = it.value().toString();
    }
    
    currentVariables = event.variables;
    currentCallStack = event.callStack;
    
    emit debugEvent(event);
    emit variablesUpdated(event.variables);
    emit callStackUpdated(event.callStack);
    
    if (event.type == DebugEventType::BreakpointHit) {
        paused = true;
        emit executionPaused();
    }
}

void CompilerConnector::handleAnalysisResponse(const QJsonObject& response) {
    AnalysisResponse analysis;
    analysis.status = response["status"].toString();
    
    // Parse symbols
    QJsonArray symbols = response["symbols"].toArray();
    for (const QJsonValue& sym : symbols) {
        QJsonObject symObj = sym.toObject();
        SymbolInfo info;
        info.name = symObj["name"].toString();
        info.type = symObj["type"].toString();
        info.line = symObj["line"].toInt();
        info.returnType = symObj["return_type"].toString();
        analysis.symbols.append(info);
    }
    
    // Parse timing hints
    QJsonObject timing = response["timing_hints"].toObject();
    QJsonArray hotspots = timing["hotspot_lines"].toArray();
    for (const QJsonValue& h : hotspots) {
        analysis.timingHints["hotspot_lines"].append(h.toInt());
    }
    
    emit analysisComplete(analysis);
    
    LOG_INFO("Analysis completed: " + QString::number(analysis.symbols.size()) + " symbols");
}

void CompilerConnector::handleVariablesResponse(const QJsonObject& response) {
    QMap<QString, QString> vars;
    
    QJsonObject varsObj = response["variables"].toObject();
    for (auto it = varsObj.begin(); it != varsObj.end(); ++it) {
        vars[it.key()] = it.value().toString();
    }
    
    currentVariables = vars;
    emit variablesUpdated(vars);
}

void CompilerConnector::handleError(const QString& error) {
    LOG_ERROR("Compiler error: " + error);
    emit connectionError(error);
}

// Process slots
void CompilerConnector::onProcessStarted() {
    LOG_INFO("Compiler process started");
}

void CompilerConnector::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    compiling = false;
    running = false;
    paused = false;
    
    if (exitStatus == QProcess::CrashExit) {
        handleError("Compiler crashed");
    }
    
    LOG_INFO("Compiler process finished with code: " + QString::number(exitCode));
}

void CompilerConnector::onReadyReadStandardOutput() {
    QByteArray data = process->readAllStandardOutput();
    compilerOutput += QString::fromUtf8(data);
    
    // Parse response
    parseResponse(QString::fromUtf8(data));
    
    emit outputReceived(QString::fromUtf8(data));
}

void CompilerConnector::onReadyReadStandardError() {
    QByteArray data = process->readAllStandardError();
    compilerError += QString::fromUtf8(data);
    
    emit errorReceived(QString::fromUtf8(data));
}

void CompilerConnector::onError(QProcess::ProcessError error) {
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start compiler";
            break;
        case QProcess::Crashed:
            errorMsg = "Compiler crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "Compiler timed out";
            break;
        case QProcess::WriteError:
            errorMsg = "Compiler write error";
            break;
        case QProcess::ReadError:
            errorMsg = "Compiler read error";
            break;
        default:
            errorMsg = "Unknown compiler error";
            break;
    }
    
    handleError(errorMsg);
}

void CompilerConnector::onStateChange(QProcess::ProcessState state) {
    switch (state) {
        case QProcess::NotRunning:
            connected = false;
            break;
        case QProcess::Starting:
            break;
        case QProcess::Running:
            connected = true;
            break;
    }
}

QString CompilerConnector::typeToString(BreakpointType type) const {
    switch (type) {
        case BreakpointType::Unconditional: return "unconditional";
        case BreakpointType::Conditional: return "conditional";
        case BreakpointType::Temporary: return "temporary";
        default: return "unknown";
    }
}

} // namespace proxima