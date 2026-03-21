#include "GitService.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTime>
#include "utils/Logger.h"

namespace proxima {

GitService::GitService(QObject *parent)
    : QObject(parent)
    , initialized(false)
    , isAsyncOperation(false)
    , statusCacheTime(0) {
    
    gitProcess = new QProcess(this);
    
    connect(gitProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GitService::onProcessFinished);
    connect(gitProcess, &QProcess::errorOccurred, this, &GitService::onProcessError);
    connect(gitProcess, &QProcess::readyReadStandardOutput, this, &GitService::onProcessReadyReadStandardOutput);
    connect(gitProcess, &QProcess::readyReadStandardError, this, &GitService::onProcessReadyReadStandardError);
}

GitService::~GitService() {
    if (gitProcess->state() != QProcess::NotRunning) {
        gitProcess->kill();
    }
}

bool GitService::initialize(const QString& path) {
    LOG_INFO("Initializing Git service: " + path);
    
    repoPath = path;
    
    if (!checkGitInstalled()) {
        lastError = "Git is not installed or not in PATH";
        LOG_ERROR(lastError);
        return false;
    }
    
    if (!isGitRepository(path)) {
        lastError = "Not a git repository: " + path;
        LOG_ERROR(lastError);
        return false;
    }
    
    initialized = true;
    statusCacheTime = 0; // Invalidate cache
    
    LOG_INFO("Git service initialized successfully");
    return true;
}

bool GitService::checkGitInstalled() const {
    QProcess process;
    process.start("git", QStringList() << "--version");
    process.waitForFinished(5000);
    
    QString output = process.readAllStandardOutput();
    return output.contains("git version");
}

bool GitService::isGitRepository(const QString& path) const {
    QDir dir(path);
    return dir.exists(".git");
}

GitStatus GitService::getStatus() const {
    if (!initialized) {
        GitStatus status;
        status.isRepository = false;
        return status;
    }
    
    // Return cached status if recent (within 5 seconds)
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (statusCacheTime > 0 && (now - statusCacheTime) < 5000) {
        return cachedStatus;
    }
    
    GitStatus status;
    status.isRepository = true;
    
    // Get current branch
    status.branch = getCurrentBranch();
    
    // Get status output
    QString output = executeGitSync(QStringList() << "status" << "--porcelain");
    
    // Parse modified files
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.length() >= 3) {
            char indexStatus = line[0];
            char worktreeStatus = line[1];
            QString filename = line.mid(3).trimmed();
            
            if (indexStatus != ' ' && indexStatus != '?') {
                status.stagedFiles.append(filename);
            }
            
            if (worktreeStatus != ' ' && worktreeStatus != '?') {
                status.modifiedFiles.append(filename);
            }
            
            if (indexStatus == '?' && worktreeStatus == '?') {
                status.untrackedFiles.append(filename);
            }
            
            if (worktreeStatus == 'D') {
                status.deletedFiles.append(filename);
            }
        }
    }
    
    status.hasChanges = !status.modifiedFiles.isEmpty() || 
                       !status.untrackedFiles.isEmpty() ||
                       !status.stagedFiles.isEmpty() ||
                       !status.deletedFiles.isEmpty();
    
    // Get last commit info
    QString logOutput = executeGitSync(QStringList() << "log" << "-1" << "--format=%H|%s|%an|%ai");
    QStringList parts = logOutput.split("|");
    if (parts.size() >= 4) {
        status.lastCommit = parts[0];
        status.lastCommitMessage = parts[1];
        status.lastCommitAuthor = parts[2];
        status.lastCommitDate = parts[3];
    }
    
    // Get remote
    status.remote = executeGitSync(QStringList() << "config" << "--get" << "remote.origin.url").trimmed();
    
    cachedStatus = status;
    statusCacheTime = now;
    
    return status;
}

bool GitService::hasChanges() const {
    return getStatus().hasChanges;
}

bool GitService::hasUncommittedChanges() const {
    GitStatus status = getStatus();
    return !status.modifiedFiles.isEmpty() || !status.untrackedFiles.isEmpty();
}

bool GitService::hasUnpushedChanges() const {
    QString output = executeGitSync(QStringList() << "rev-list" << "HEAD" << "@{u}" << "--count");
    return output.trimmed().toInt() > 0;
}

bool GitService::add(const QString& file) {
    emit operationStarted("add");
    
    QString output = executeGitSync(QStringList() << "add" << file);
    bool success = !output.contains("fatal");
    
    emit operationFinished("add");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::addAll() {
    emit operationStarted("add_all");
    
    QString output = executeGitSync(QStringList() << "add" << ".");
    bool success = !output.contains("fatal");
    
    emit operationFinished("add_all");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::addPattern(const QString& pattern) {
    emit operationStarted("add_pattern");
    
    QString output = executeGitSync(QStringList() << "add" << "-u" << pattern);
    bool success = !output.contains("fatal");
    
    emit operationFinished("add_pattern");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::commit(const QString& message) {
    emit operationStarted("commit");
    
    QString output = executeGitSync(QStringList() << "commit" << "-m" << message);
    bool success = !output.contains("fatal");
    
    if (success) {
        // Extract commit hash
        QRegularExpression re("\\[.*?\\]\\s+([a-f0-9]+)");
        QRegularExpressionMatch match = re.match(output);
        QString hash = match.hasMatch() ? match.captured(1) : "";
        
        emit commitCompleted(success, hash);
    }
    
    emit operationFinished("commit");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::commitWithFiles(const QString& message, const QVector<QString>& files) {
    // Add specified files first
    for (const QString& file : files) {
        add(file);
    }
    
    return commit(message);
}

bool GitService::push(const QString& remote, const QString& branch) {
    emit operationStarted("push");
    
    QStringList args;
    args << "push";
    if (!remote.isEmpty()) args << remote;
    if (!branch.isEmpty()) args << branch;
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal") && !output.contains("error");
    
    emit pushCompleted(success);
    emit operationFinished("push");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::pull(const QString& remote, const QString& branch) {
    emit operationStarted("pull");
    
    QStringList args;
    args << "pull";
    if (!remote.isEmpty()) args << remote;
    if (!branch.isEmpty()) args << branch;
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal") && !output.contains("error");
    
    emit pullCompleted(success);
    emit operationFinished("pull");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::fetch(const QString& remote) {
    emit operationStarted("fetch");
    
    QStringList args;
    args << "fetch";
    if (!remote.isEmpty()) args << remote;
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal");
    
    emit operationFinished("fetch");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::revert(const QString& file) {
    emit operationStarted("revert");
    
    QString output = executeGitSync(QStringList() << "checkout" << "--" << file);
    bool success = !output.contains("fatal");
    
    emit operationFinished("revert");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::revertAll() {
    emit operationStarted("revert_all");
    
    QString output = executeGitSync(QStringList() << "reset" << "--hard" << "HEAD");
    bool success = !output.contains("fatal");
    
    emit operationFinished("revert_all");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::stash(const QString& message) {
    emit operationStarted("stash");
    
    QStringList args;
    args << "stash" << "push";
    if (!message.isEmpty()) {
        args << "-m" << message;
    }
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal");
    
    emit operationFinished("stash");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::stashPop() {
    emit operationStarted("stash_pop");
    
    QString output = executeGitSync(QStringList() << "stash" << "pop");
    bool success = !output.contains("fatal");
    
    emit operationFinished("stash_pop");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::stashApply(const QString& stashRef) {
    emit operationStarted("stash_apply");
    
    QStringList args;
    args << "stash" << "apply";
    if (!stashRef.isEmpty()) {
        args << stashRef;
    }
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal");
    
    emit operationFinished("stash_apply");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

QString GitService::getCurrentBranch() const {
    QString output = executeGitSync(QStringList() << "rev-parse" << "--abbrev-ref" << "HEAD");
    return output.trimmed();
}

QVector<GitBranch> GitService::getBranches() const {
    QVector<GitBranch> branches;
    
    QString output = executeGitSync(QStringList() << "branch" << "-v");
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    
    QString currentBranch = getCurrentBranch();
    
    for (const QString& line : lines) {
        GitBranch branch;
        
        // Parse branch line
        // Format: "* branch_name commit_hash commit_message" or "  branch_name commit_hash commit_message"
        bool isCurrent = line.startsWith("*");
        QString cleaned = line.trimmed();
        if (cleaned.startsWith("*")) {
            cleaned = cleaned.mid(1).trimmed();
        }
        
        QStringList parts = cleaned.split(" ", Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            branch.name = parts[0];
            branch.isCurrent = (branch.name == currentBranch);
            branch.isRemote = false;
            branch.lastCommit = parts[1];
            
            branches.append(branch);
        }
    }
    
    return branches;
}

QVector<GitBranch> GitService::getRemoteBranches() const {
    QVector<GitBranch> branches;
    
    QString output = executeGitSync(QStringList() << "branch" << "-r" << "-v");
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        GitBranch branch;
        
        QString cleaned = line.trimmed();
        QStringList parts = cleaned.split(" ", Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            branch.name = parts[0];
            branch.isCurrent = false;
            branch.isRemote = true;
            branch.lastCommit = parts[1];
            
            branches.append(branch);
        }
    }
    
    return branches;
}

bool GitService::createBranch(const QString& name, const QString& startPoint) {
    emit operationStarted("create_branch");
    
    QStringList args;
    args << "branch" << name;
    if (!startPoint.isEmpty()) {
        args << startPoint;
    }
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal");
    
    if (success) {
        emit branchChanged(name);
    }
    
    emit operationFinished("create_branch");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::checkoutBranch(const QString& name) {
    emit operationStarted("checkout_branch");
    
    QString output = executeGitSync(QStringList() << "checkout" << name);
    bool success = !output.contains("fatal");
    
    if (success) {
        emit branchChanged(name);
    }
    
    emit operationFinished("checkout_branch");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::deleteBranch(const QString& name, bool force) {
    emit operationStarted("delete_branch");
    
    QStringList args;
    args << "branch";
    if (force) {
        args << "-D";
    } else {
        args << "-d";
    }
    args << name;
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal");
    
    emit operationFinished("delete_branch");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::mergeBranch(const QString& name, bool noCommit) {
    emit operationStarted("merge_branch");
    
    QStringList args;
    args << "merge";
    if (noCommit) {
        args << "--no-commit";
    }
    args << name;
    
    QString output = executeGitSync(args);
    bool success = !output.contains("fatal") && !output.contains("conflict");
    
    emit operationFinished("merge_branch");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::rebaseBranch(const QString& name) {
    emit operationStarted("rebase_branch");
    
    QString output = executeGitSync(QStringList() << "rebase" << name);
    bool success = !output.contains("fatal");
    
    emit operationFinished("rebase_branch");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

QVector<GitCommit> GitService::getHistory(int count) const {
    QVector<GitCommit> commits;
    
    QString output = executeGitSync(QStringList() 
        << "log" 
        << "-" << QString::number(count)
        << "--format=%H|%h|%s|%an|%ae|%ai|%P");
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QStringList parts = line.split("|");
        if (parts.size() >= 6) {
            GitCommit commit;
            commit.hash = parts[0];
            commit.shortHash = parts[1];
            commit.message = parts[2];
            commit.author = parts[3];
            commit.email = parts[4];
            commit.date = parts[5];
            
            if (parts.size() > 6 && !parts[6].isEmpty()) {
                commit.parents = parts[6].split(" ");
            }
            
            commits.append(commit);
        }
    }
    
    return commits;
}

GitCommit GitService::getCommit(const QString& hash) const {
    GitCommit commit;
    
    QString output = executeGitSync(QStringList() 
        << "show" 
        << "-s"
        << "--format=%H|%h|%s|%an|%ae|%ai|%P"
        << hash);
    
    QStringList parts = output.split("|");
    if (parts.size() >= 6) {
        commit.hash = parts[0];
        commit.shortHash = parts[1];
        commit.message = parts[2];
        commit.author = parts[3];
        commit.email = parts[4];
        commit.date = parts[5];
        
        if (parts.size() > 6 && !parts[6].isEmpty()) {
            commit.parents = parts[6].split(" ");
        }
    }
    
    return commit;
}

GitDiff GitService::getDiff(const QString& file) const {
    return getDiffBetweenCommits("HEAD", "", file);
}

GitDiff GitService::getDiffBetweenCommits(const QString& commit1, const QString& commit2, const QString& file) const {
    GitDiff diff;
    diff.file = file;
    diff.additions = 0;
    diff.deletions = 0;
    
    QStringList args;
    args << "diff";
    if (!commit2.isEmpty()) {
        args << commit2;
    }
    args << commit1;
    if (!file.isEmpty()) {
        args << "--" << file;
    }
    
    QString output = executeGitSync(args);
    diff.newContent = output;
    
    // Parse diff
    QStringList lines = output.split("\n");
    for (const QString& line : lines) {
        if (line.startsWith("+") && !line.startsWith("+++")) {
            GitDiff::Line diffLine;
            diffLine.content = line.mid(1);
            diffLine.added = true;
            diffLine.removed = false;
            diff.lines.append(diffLine);
            diff.additions++;
        } else if (line.startsWith("-") && !line.startsWith("---")) {
            GitDiff::Line diffLine;
            diffLine.content = line.mid(1);
            diffLine.added = false;
            diffLine.removed = true;
            diff.lines.append(diffLine);
            diff.deletions++;
        }
    }
    
    return diff;
}

QString GitService::getFileAtCommit(const QString& file, const QString& commit) const {
    QString ref = commit + ":" + file;
    return executeGitSync(QStringList() << "show" << ref);
}

QVector<QString> GitService::getRemotes() const {
    QVector<QString> remotes;
    
    QString output = executeGitSync(QStringList() << "remote");
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        remotes.append(line.trimmed());
    }
    
    return remotes;
}

QString GitService::getRemoteURL(const QString& remote) const {
    return executeGitSync(QStringList() << "remote" << "get-url" << remote).trimmed();
}

bool GitService::setRemoteURL(const QString& remote, const QString& url) {
    emit operationStarted("set_remote_url");
    
    QString output = executeGitSync(QStringList() << "remote" << "set-url" << remote << url);
    bool success = !output.contains("fatal");
    
    emit operationFinished("set_remote_url");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::addRemote(const QString& name, const QString& url) {
    emit operationStarted("add_remote");
    
    QString output = executeGitSync(QStringList() << "remote" << "add" << name << url);
    bool success = !output.contains("fatal");
    
    emit operationFinished("add_remote");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::removeRemote(const QString& name) {
    emit operationStarted("remove_remote");
    
    QString output = executeGitSync(QStringList() << "remote" << "remove" << name);
    bool success = !output.contains("fatal");
    
    emit operationFinished("remove_remote");
    
    if (!success) {
        lastError = output;
    }
    
    return success;
}

bool GitService::addToIgnore(const QString& pattern) {
    QString gitignorePath = repoPath + "/.gitignore";
    
    QFile file(gitignorePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << pattern << "\n";
    file.close();
    
    return true;
}

QVector<QString> GitService::getIgnorePatterns() const {
    QVector<QString> patterns;
    
    QString gitignorePath = repoPath + "/.gitignore";
    QFile file(gitignorePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith("#")) {
                patterns.append(line);
            }
        }
        file.close();
    }
    
    return patterns;
}

void GitService::asyncStatus() {
    executeGit(QStringList() << "status" << "--porcelain");
}

void GitService::asyncCommit(const QString& message) {
    executeGit(QStringList() << "commit" << "-m" << message);
}

void GitService::asyncPush() {
    executeGit(QStringList() << "push");
}

void GitService::asyncPull() {
    executeGit(QStringList() << "pull");
}

void GitService::executeGit(const QStringList& args) {
    isAsyncOperation = true;
    currentOperation = args.isEmpty() ? "" : args[0];
    
    gitProcess->start("git", args);
}

QString GitService::executeGitSync(const QStringList& args) const {
    QProcess process;
    process.setWorkingDirectory(repoPath);
    process.start("git", args);
    process.waitForFinished(30000);
    
    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    
    if (!error.isEmpty() && error.contains("fatal")) {
        return "fatal: " + error;
    }
    
    return output;
}

void GitService::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QString output = gitProcess->readAllStandardOutput();
    QString error = gitProcess->readAllStandardError();
    
    if (exitStatus == QProcess::CrashExit) {
        lastError = "Git process crashed";
        emit errorOccurred(lastError);
    } else if (exitCode != 0 && !error.isEmpty()) {
        lastError = error;
        emit errorOccurred(lastError);
    }
    
    // Emit appropriate signal based on operation
    if (currentOperation == "commit") {
        bool success = (exitCode == 0);
        QString hash;
        if (success) {
            QRegularExpression re("\\[.*?\\]\\s+([a-f0-9]+)");
            QRegularExpressionMatch match = re.match(output);
            hash = match.hasMatch() ? match.captured(1) : "";
        }
        emit commitCompleted(success, hash);
    } else if (currentOperation == "push") {
        emit pushCompleted(exitCode == 0);
    } else if (currentOperation == "pull") {
        emit pullCompleted(exitCode == 0);
    }
    
    emit operationFinished(currentOperation);
    isAsyncOperation = false;
}

void GitService::onProcessError(QProcess::ProcessError error) {
    switch (error) {
        case QProcess::FailedToStart:
            lastError = "Failed to start git process";
            break;
        case QProcess::Crashed:
            lastError = "Git process crashed";
            break;
        case QProcess::Timedout:
            lastError = "Git process timed out";
            break;
        default:
            lastError = "Unknown git process error";
            break;
    }
    
    emit errorOccurred(lastError);
}

void GitService::onProcessReadyReadStandardOutput() {
    // Handle async output if needed
}

void GitService::onProcessReadyReadStandardError() {
    // Handle async error output if needed
}

GitStatus GitService::parseStatus(const QString& output) const {
    GitStatus status;
    status.isRepository = true;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.length() >= 3) {
            char indexStatus = line[0];
            char worktreeStatus = line[1];
            QString filename = line.mid(3).trimmed();
            
            if (indexStatus != ' ' && indexStatus != '?') {
                status.stagedFiles.append(filename);
            }
            
            if (worktreeStatus != ' ' && worktreeStatus != '?') {
                status.modifiedFiles.append(filename);
            }
            
            if (indexStatus == '?' && worktreeStatus == '?') {
                status.untrackedFiles.append(filename);
            }
        }
    }
    
    status.hasChanges = !status.modifiedFiles.isEmpty() || 
                       !status.untrackedFiles.isEmpty() ||
                       !status.stagedFiles.isEmpty();
    
    return status;
}

QVector<GitCommit> GitService::parseHistory(const QString& output) const {
    QVector<GitCommit> commits;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QStringList parts = line.split("|");
        if (parts.size() >= 6) {
            GitCommit commit;
            commit.hash = parts[0];
            commit.shortHash = parts[1];
            commit.message = parts[2];
            commit.author = parts[3];
            commit.email = parts[4];
            commit.date = parts[5];
            
            if (parts.size() > 6 && !parts[6].isEmpty()) {
                commit.parents = parts[6].split(" ");
            }
            
            commits.append(commit);
        }
    }
    
    return commits;
}

GitDiff GitService::parseDiff(const QString& output, const QString& file) const {
    GitDiff diff;
    diff.file = file;
    diff.additions = 0;
    diff.deletions = 0;
    
    QStringList lines = output.split("\n");
    for (const QString& line : lines) {
        if (line.startsWith("+") && !line.startsWith("+++")) {
            GitDiff::Line diffLine;
            diffLine.content = line.mid(1);
            diffLine.added = true;
            diffLine.removed = false;
            diff.lines.append(diffLine);
            diff.additions++;
        } else if (line.startsWith("-") && !line.startsWith("---")) {
            GitDiff::Line diffLine;
            diffLine.content = line.mid(1);
            diffLine.added = false;
            diffLine.removed = true;
            diff.lines.append(diffLine);
            diff.deletions++;
        }
    }
    
    return diff;
}

QVector<GitBranch> GitService::parseBranches(const QString& output) const {
    QVector<GitBranch> branches;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    QString currentBranch = getCurrentBranch();
    
    for (const QString& line : lines) {
        GitBranch branch;
        
        bool isCurrent = line.startsWith("*");
        QString cleaned = line.trimmed();
        if (cleaned.startsWith("*")) {
            cleaned = cleaned.mid(1).trimmed();
        }
        
        QStringList parts = cleaned.split(" ", Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            branch.name = parts[0];
            branch.isCurrent = (branch.name == currentBranch);
            branch.isRemote = false;
            branch.lastCommit = parts[1];
            
            branches.append(branch);
        }
    }
    
    return branches;
}

} // namespace proxima