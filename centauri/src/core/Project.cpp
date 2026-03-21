#include "Project.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QCryptographicHash>
#include "utils/Logger.h"

namespace proxima {

Project::Project(QObject *parent)
    : QObject(parent)
    , status(ProjectStatus::New)
    , gitInitialized(false)
    , autoSaveEnabled(true)
    , lastModified(0) {
    
    buildConfig = Config();
}

Project::~Project() {
    close();
}

bool Project::create(const QString& path, const QString& name) {
    LOG_INFO("Creating project: " + name + " at " + path);
    
    projectPath = path + "/" + name;
    
    // Create directory structure
    setupProjectStructure();
    
    // Initialize info
    info.name = name;
    info.version = "1.0.0";
    info.entryPoint = "src/main.prx";
    info.requiredIDEVersion = "Centauri >= 0.9.0";
    info.capabilities = QStringList();
    info.license = "GPLv3";
    info.authors = QStringList();
    info.buildHash = "auto";
    info.buildTimestamp = "auto";
    
    // Create manifest
    saveManifest();
    
    // Create default build rules
    buildConfig.outputPath = "build/" + name;
    buildConfig.optimizationLevel = 2;
    buildConfig.debugSymbols = true;
    saveBuildRules();
    
    // Create main module
    QDir(projectPath + "/src").mkdir(".");
    QFile mainFile(projectPath + "/src/main.prx");
    if (mainFile.open(QIODevice::WriteOnly)) {
        QTextStream out(&mainFile);
        out << "// Main module for " << name << "\n\n";
        out << "namespace(\"" << name.toLower() << "\");\n\n";
        out << "main(): int32\n";
        out << "    print(\"Hello from " << name << "!\\n\");\n";
        out << "    return 0;\n";
        out << "end\n\n";
        out << "end  // namespace\n";
        mainFile.close();
        
        addModule("src/main.prx");
    }
    
    lastModified = QDateTime::currentMSecsSinceEpoch();
    updateStatus(ProjectStatus::Open);
    
    emit projectCreated(projectPath);
    
    LOG_INFO("Project created successfully: " + projectPath);
    return true;
}

bool Project::load(const QString& path) {
    LOG_INFO("Loading project: " + path);
    
    projectPath = path;
    
    // Load manifest
    if (!loadManifest()) {
        LOG_ERROR("Failed to load manifest");
        return false;
    }
    
    // Load build rules
    loadBuildRules();
    
    // Load modules
    loadModules();
    
    lastModified = QDateTime::currentMSecsSinceEpoch();
    updateStatus(ProjectStatus::Open);
    
    // Check if git repository
    gitInitialized = QDir(projectPath + "/.git").exists();
    
    emit projectLoaded(projectPath);
    
    LOG_INFO("Project loaded successfully: " + projectPath);
    return true;
}

bool Project::save() {
    LOG_INFO("Saving project: " + info.name);
    
    // Save manifest
    saveManifest();
    
    // Save build rules
    saveBuildRules();
    
    // Save all modules
    for (Module* module : modules) {
        if (module->isModified()) {
            module->save();
        }
    }
    
    // Update build hash
    calculateBuildHash();
    buildTimestamp = generateBuildTimestamp();
    
    lastModified = QDateTime::currentMSecsSinceEpoch();
    
    emit projectSaved();
    
    LOG_INFO("Project saved successfully");
    return true;
}

bool Project::close() {
    LOG_INFO("Closing project: " + info.name);
    
    // Save if modified
    if (status == ProjectStatus::Modified) {
        save();
    }
    
    // Clean up modules
    for (Module* module : modules) {
        delete module;
    }
    modules.clear();
    modulesByPath.clear();
    
    projectPath.clear();
    info = ProjectInfo();
    
    updateStatus(ProjectStatus::New);
    
    emit projectClosed();
    
    LOG_INFO("Project closed");
    return true;
}

void Project::setupProjectStructure() {
    QDir dir(projectPath);
    
    // Create main directories
    dir.mkdir("src");
    dir.mkdir("rules");
    dir.mkdir("assets");
    dir.mkdir("autosave");
    dir.mkdir("build");
    
    // Create rules directory structure
    QDir rulesDir(projectPath + "/rules");
    rulesDir.mkdir(".");
    
    // Create assets subdirectories
    QDir assetsDir(projectPath + "/assets");
    assetsDir.mkdir("config");
    assetsDir.mkdir("data");
}

bool Project::loadManifest() {
    QFile manifest(projectPath + "/manifest");
    if (!manifest.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    // Parse collection format
    // Simplified parser for manifest
    QTextStream in(&manifest);
    QString content = in.readAll();
    manifest.close();
    
    // Extract values (simplified)
    // In production, use proper collection parser
    info.name = "Project"; // Default
    info.version = "1.0.0";
    info.entryPoint = "src/main.prx";
    
    return true;
}

bool Project::saveManifest() {
    QFile manifest(projectPath + "/manifest");
    if (!manifest.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QTextStream out(&manifest);
    out << "[\n";
    out << "    \"name\", \"" << info.name << "\",\n";
    out << "    \"version\", \"" << info.version << "\",\n";
    out << "    \"entry_point\", \"" << info.entryPoint << "\",\n";
    out << "    \"required_ide_version\", \"" << info.requiredIDEVersion << "\",\n";
    out << "    \"capabilities\", " << info.capabilities.join(", ") << ",\n";
    out << "    \"license\", \"" << info.license << "\",\n";
    out << "    \"authors\", " << info.authors.join(", ") << ",\n";
    out << "    \"build_hash\", \"" << (buildHash.isEmpty() ? "auto" : buildHash) << "\",\n";
    out << "    \"build_timestamp\", \"" << (buildTimestamp.isEmpty() ? "auto" : buildTimestamp) << "\"\n";
    out << "]\n";
    
    manifest.close();
    return true;
}

bool Project::loadModules() {
    QDir srcDir(projectPath + "/src");
    if (!srcDir.exists()) {
        return false;
    }
    
    // Find all .prx files
    QStringList filters;
    filters << "*.prx";
    srcDir.setNameFilters(filters);
    srcDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    QFileInfoList files = srcDir.entryInfoList(filters, QDir::Files, QDir::Name);
    
    for (const QFileInfo& fileInfo : files) {
        QString relativePath = fileInfo.absoluteFilePath().replace(projectPath + "/", "");
        addModule(relativePath);
    }
    
    // Recursively search subdirectories
    QFileInfoList dirs = srcDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& dirInfo : dirs) {
        // Recursively load modules from subdirectories
        // Implementation would recurse into subdirectories
    }
    
    return true;
}

bool Project::addModule(const QString& path) {
    if (hasModule(path)) {
        return false;
    }
    
    Module* module = new Module(this);
    if (!module->load(projectPath + "/" + path)) {
        delete module;
        return false;
    }
    
    modules.append(module);
    modulesByPath[path] = module;
    
    connect(module, &Module::modified, this, [this, module]() {
        emit moduleModified(module);
        updateStatus(ProjectStatus::Modified);
        lastModified = QDateTime::currentMSecsSinceEpoch();
    });
    
    emit moduleAdded(module);
    
    LOG_INFO("Module added: " + path);
    return true;
}

bool Project::removeModule(const QString& path) {
    if (!modulesByPath.contains(path)) {
        return false;
    }
    
    Module* module = modulesByPath[path];
    modules.removeOne(module);
    modulesByPath.remove(path);
    
    emit moduleRemoved(path);
    
    delete module;
    
    LOG_INFO("Module removed: " + path);
    return true;
}

Module* Project::getModule(const QString& path) const {
    return modulesByPath.value(path, nullptr);
}

QVector<Module*> Project::getSubprojectModules(const QString& subproject) const {
    QVector<Module*> result;
    
    for (Module* module : modules) {
        if (module->getPath().startsWith(subproject)) {
            result.append(module);
        }
    }
    
    return result;
}

bool Project::hasModule(const QString& path) const {
    return modulesByPath.contains(path);
}

bool Project::loadBuildRules() {
    QFile rules(projectPath + "/rules/build.rules");
    if (!rules.open(QIODevice::ReadOnly)) {
        // Create default rules
        return saveBuildRules();
    }
    
    // Parse rules file
    // Implementation would parse collection format
    
    rules.close();
    return true;
}

bool Project::saveBuildRules() {
    QFile rules(projectPath + "/rules/build.rules");
    if (!rules.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QTextStream out(&rules);
    out << "// Build rules for " << info.name << "\n\n";
    out << "[\n";
    out << "    \"compiler_version\", \"1.0.0\",\n";
    out << "    \"optimization_level\", \"" << buildConfig.optimizationLevel << "\",\n";
    out << "    \"target_arch\", \"" << buildConfig.targetArch << "\",\n";
    out << "    \"enable_cuda\", " << (buildConfig.enableCUDA ? "true" : "false") << ",\n";
    out << "    \"enable_avx2\", " << (buildConfig.enableAVX2 ? "true" : "false") << ",\n";
    out << "    \"max_memory\", \"4GB\",\n";
    out << "    \"debug_symbols\", " << (buildConfig.debugSymbols ? "true" : "false") << ",\n";
    out << "    \"output_format\", \"" << buildConfig.outputFormat << "\"\n";
    out << "]\n";
    
    rules.close();
    return true;
}

bool Project::build() {
    updateStatus(ProjectStatus::Building);
    emit buildStarted();
    
    // Save all modules first
    save();
    
    // Trigger build through compiler connector
    // This would be done by MainWindow
    
    LOG_INFO("Build initiated for: " + info.name);
    return true;
}

bool Project::clean() {
    // Remove build directory
    QDir buildDir(projectPath + "/build");
    if (buildDir.exists()) {
        buildDir.removeRecursively();
    }
    
    LOG_INFO("Build cleaned");
    return true;
}

bool Project::run() {
    if (status == ProjectStatus::Building) {
        LOG_ERROR("Cannot run while building");
        return false;
    }
    
    updateStatus(ProjectStatus::Running);
    
    // Trigger run through compiler connector
    
    LOG_INFO("Run initiated for: " + info.name);
    return true;
}

void Project::initializeGit() {
    // Initialize git repository
    // Would use GitService
    
    gitInitialized = true;
    LOG_INFO("Git repository initialized");
}

QString Project::getAutoSavePath(const QString& modulePath) const {
    return projectPath + "/autosave/" + modulePath;
}

int Project::getTotalLines() const {
    int total = 0;
    for (Module* module : modules) {
        total += module->getLineCount();
    }
    return total;
}

void Project::updateStatus(ProjectStatus newStatus) {
    if (status == newStatus) return;
    
    status = newStatus;
    emit statusChanged(status);
}

void Project::calculateBuildHash() {
    QCryptographicHash hash(QCryptographicHash::MD5);
    
    for (Module* module : modules) {
        hash.addData(module->getContent().toUtf8());
    }
    
    buildHash = QString::fromUtf8(hash.result().toHex());
}

QString Project::generateBuildTimestamp() const {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

} // namespace proxima
