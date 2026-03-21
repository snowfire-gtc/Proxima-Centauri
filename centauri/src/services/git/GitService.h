#ifndef CENTAURI_GITSERVICE_H
#define CENTAURI_GITSERVICE_H

#include <QObject>
#include <QProcess>
#include <QMap>
#include <QVector>

namespace proxima {

struct GitStatus {
    QString branch;
    QString remote;
    QString lastCommit;
    QString lastCommitMessage;
    QString lastCommitDate;
    QString lastCommitAuthor;
    QVector<QString> modifiedFiles;
    QVector<QString> stagedFiles;
    QVector<QString> untrackedFiles;
    QVector<QString> deletedFiles;
    bool hasChanges;
    bool isRepository;
};

struct GitCommit {
    QString hash;
    QString shortHash;
    QString message;
    QString author;
    QString email;
    QString date;
    QVector<QString> parents;
};

struct GitDiff {
    QString file;
    QString oldContent;
    QString newContent;
    struct Line {
        int number;
        QString content;
        bool added;
        bool removed;
    };
    QVector<Line> lines;
    int additions;
    int deletions;
};

struct GitBranch {
    QString name;
    bool isCurrent;
    bool isRemote;
    QString upstream;
    QString lastCommit;
};

class GitService : public QObject {
    Q_OBJECT
    
public:
    explicit GitService(QObject *parent = nullptr);
    ~GitService();
    
    // Initialization
    bool initialize(const QString& repoPath);
    bool isInitialized() const { return initialized; }
    QString getRepoPath() const { return repoPath; }
    
    // Status
    GitStatus getStatus() const;
    bool hasChanges() const;
    bool hasUncommittedChanges() const;
    bool hasUnpushedChanges() const;
    
    // Operations
    bool add(const QString& file);
    bool addAll();
    bool addPattern(const QString& pattern);
    bool commit(const QString& message);
    bool commitWithFiles(const QString& message, const QVector<QString>& files);
    bool push(const QString& remote = "origin", const QString& branch = "");
    bool pull(const QString& remote = "origin", const QString& branch = "");
    bool fetch(const QString& remote = "origin");
    bool revert(const QString& file);
    bool revertAll();
    bool stash(const QString& message = "");
    bool stashPop();
    bool stashApply(const QString& stashRef = "");
    
    // Branches
    QString getCurrentBranch() const;
    QVector<GitBranch> getBranches() const;
    QVector<GitBranch> getRemoteBranches() const;
    bool createBranch(const QString& name, const QString& startPoint = "");
    bool checkoutBranch(const QString& name);
    bool deleteBranch(const QString& name, bool force = false);
    bool mergeBranch(const QString& name, bool noCommit = false);
    bool rebaseBranch(const QString& name);
    
    // History
    QVector<GitCommit> getHistory(int count = 20) const;
    GitCommit getCommit(const QString& hash) const;
    GitDiff getDiff(const QString& file) const;
    GitDiff getDiffBetweenCommits(const QString& commit1, const QString& commit2, 
                                  const QString& file = "") const;
    QString getFileAtCommit(const QString& file, const QString& commit) const;
    
    // Remote
    QVector<QString> getRemotes() const;
    QString getRemoteURL(const QString& remote) const;
    bool setRemoteURL(const QString& remote, const QString& url);
    bool addRemote(const QString& name, const QString& url);
    bool removeRemote(const QString& name);
    
    // Ignore
    bool addToIgnore(const QString& pattern);
    QVector<QString> getIgnorePatterns() const;
    
    // Error handling
    QString getLastError() const { return lastError; }
    bool hasErrors() const { return !lastError.isEmpty(); }
    
    // Async operations
    void asyncStatus();
    void asyncCommit(const QString& message);
    void asyncPush();
    void asyncPull();
    
signals:
    void statusUpdated(const GitStatus& status);
    void commitCompleted(bool success, const QString& hash);
    void pushCompleted(bool success);
    void pullCompleted(bool success);
    void branchChanged(const QString& branch);
    void errorOccurred(const QString& error);
    void operationStarted(const QString& operation);
    void operationFinished(const QString& operation);
    
private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();
    
private:
    void executeGit(const QStringList& args);
    QString executeGitSync(const QStringList& args) const;
    bool checkGitInstalled() const;
    bool isGitRepository(const QString& path) const;
    
    GitStatus parseStatus(const QString& output) const;
    QVector<GitCommit> parseHistory(const QString& output) const;
    GitDiff parseDiff(const QString& output, const QString& file) const;
    QVector<GitBranch> parseBranches(const QString& output) const;
    
    QString repoPath;
    bool initialized;
    QString lastError;
    
    QProcess* gitProcess;
    bool isAsyncOperation;
    QString currentOperation;
    
    GitStatus cachedStatus;
    qint64 statusCacheTime;
};

} // namespace proxima

#endif // CENTAURI_GITSERVICE_H