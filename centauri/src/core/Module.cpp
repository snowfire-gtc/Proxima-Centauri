#include "Module.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDir>
#include "utils/Logger.h"

namespace proxima {

Module::Module(QObject *parent)
    : QObject(parent) {
    
    info.isModified = false;
    info.isReadOnly = false;
    info.lineCount = 0;
    info.size = 0;
}

Module::~Module() {}

bool Module::load(const QString& path) {
    LOG_INFO("Loading module: " + path);
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("Failed to open module: " + path);
        return false;
    }
    
    content = QString::fromUtf8(file.readAll());
    file.close();
    
    info.path = path;
    info.name = QFileInfo(path).fileName();
    info.size = content.size();
    info.lineCount = countLines(content);
    info.lastModified = QFileInfo(file).lastModified();
    info.lastSaved = info.lastModified;
    info.isModified = false;
    
    // Parse content
    parseContent();
    
    emit loaded(path);
    
    LOG_INFO("Module loaded: " + path + " (" + 
            QString::number(info.lineCount) + " lines)");
    return true;
}

bool Module::save() {
    if (info.path.isEmpty()) {
        return saveAs("");
    }
    
    LOG_INFO("Saving module: " + info.path);
    
    QFile file(info.path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR("Failed to save module: " + info.path);
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;
    file.close();
    
    info.size = content.size();
    info.lineCount = countLines(content);
    info.lastSaved = QDateTime::currentDateTime();
    info.lastModified = info.lastSaved;
    info.isModified = false;
    
    markSaved();
    emit saved(info.path);
    
    LOG_INFO("Module saved: " + info.path);
    return true;
}

bool Module::saveAs(const QString& path) {
    if (path.isEmpty()) {
        // Show save dialog (would be handled by UI)
        return false;
    }
    
    info.path = path;
    info.name = QFileInfo(path).fileName();
    
    return save();
}

void Module::setContent(const QString& newContent) {
    if (content == newContent) return;
    
    content = newContent;
    info.lineCount = countLines(content);
    info.size = content.size();
    info.isModified = true;
    info.lastModified = QDateTime::currentDateTime();
    
    parseContent();
    
    emit contentChanged();
    emit modified();
}

void Module::insertText(int position, const QString& text) {
    if (position < 0 || position > content.length()) return;
    
    content.insert(position, text);
    setContent(content);
}

void Module::removeText(int start, int end) {
    if (start < 0 || end > content.length() || start >= end) return;
    
    content.remove(start, end - start);
    setContent(content);
}

QString Module::getLine(int line) const {
    if (line < 1 || line > info.lineCount) return "";
    
    QStringList lines = content.split('\n');
    if (line <= lines.size()) {
        return lines[line - 1];
    }
    return "";
}

QString Module::getLines(int start, int end) const {
    if (start < 1 || end > info.lineCount || start > end) return "";
    
    QStringList lines = content.split('\n');
    QStringList result;
    
    for (int i = start - 1; i < end && i < lines.size(); i++) {
        result.append(lines[i]);
    }
    
    return result.join('\n');
}

void Module::analyze() {
    extractDependencies();
    extractFunctions();
    extractClasses();
    
    emit dependenciesChanged(dependencies);
}

void Module::parseContent() {
    extractDependencies();
    extractFunctions();
    extractClasses();
    updateInfo();
}

void Module::extractDependencies() {
    dependencies.clear();
    
    // Find include statements
    QRegularExpression includeRe(R"(include\(["']([^"']+)["']\))");
    QRegularExpressionMatchIterator i = includeRe.globalMatch(content);
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        dependencies.append(match.captured(1));
    }
}

void Module::extractFunctions() {
    functions.clear();
    
    // Find function declarations
    QRegularExpression funcRe(R"(\b(\w+)\s*\([^)]*\)\s*:\s*\w+)");
    QRegularExpressionMatchIterator i = funcRe.globalMatch(content);
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        functions.append(match.captured(1));
    }
}

void Module::extractClasses() {
    classes.clear();
    
    // Find class declarations
    QRegularExpression classRe(R"(\bclass\s+(\w+))");
    QRegularExpressionMatchIterator i = classRe.globalMatch(content);
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        classes.append(match.captured(1));
    }
}

void Module::markSaved() {
    info.isModified = false;
    info.lastSaved = QDateTime::currentDateTime();
}

void Module::markModified() {
    info.isModified = true;
    info.lastModified = QDateTime::currentDateTime();
    emit modified();
}

void Module::updateInfo() {
    // Extract namespace
    QRegularExpression nsRe(R"(namespace\(["']([^"']+)["']\))");
    QRegularExpressionMatch match = nsRe.match(content);
    if (match.hasMatch()) {
        info.namespace_ = match.captured(1);
    }
    
    info.lineCount = countLines(content);
    info.size = content.size();
}

int Module::countLines(const QString& text) const {
    if (text.isEmpty()) return 0;
    return text.count('\n') + 1;
}

} // namespace proxima
