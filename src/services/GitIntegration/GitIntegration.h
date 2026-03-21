#ifndef PROXIMA_GIT_INTEGRATION_H
#define PROXIMA_GIT_INTEGRATION_H

#include <string>
#include <vector>
#include <map>

namespace proxima {

struct GitStatus {
    std::string branch;
    std::string lastCommit;
    std::string lastCommitMessage;
    std::string lastCommitDate;
    std::vector<std::string> modifiedFiles;
    std::vector<std::string> untrackedFiles;
    std::vector<std::string> stagedFiles;
    bool isRepository;
};

struct GitCommit {
    std::string hash;
    std::string message;
    std::string author;
    std::string date;
    std::vector<std::string> parents;
};

struct GitDiff {
    std::string filename;
    std::string oldContent;
    std::string newContent;
    std::vector<std::string> addedLines;
    std::vector<std::string> removedLines;
    int addedCount;
    int removedCount;
};

class GitIntegration {
public:
    GitIntegration();
    ~GitIntegration();
    
    bool initialize(const std::string& repoPath);
    bool isInitialized() const { return initialized; }
    
    // Status
    GitStatus getStatus() const;
    bool hasChanges() const;
    
    // Operations
    bool add(const std::string& file);
    bool addAll();
    bool commit(const std::string& message);
    bool push(const std::string& remote = "origin");
    bool pull(const std::string& remote = "origin");
    bool fetch(const std::string& remote = "origin");
    
    // Branches
    std::string getCurrentBranch() const;
    std::vector<std::string> getBranches() const;
    bool createBranch(const std::string& name);
    bool checkoutBranch(const std::string& name);
    bool deleteBranch(const std::string& name);
    bool mergeBranch(const std::string& name);
    
    // History
    std::vector<GitCommit> getHistory(int count = 10) const;
    GitDiff getDiff(const std::string& file) const;
    GitDiff getDiffWithBranch(const std::string& file, const std::string& branch) const;
    
    // Stash
    bool stash(const std::string& message = "");
    bool stashPop();
    bool stashList() const;
    
    // Remote
    std::string getRemoteURL(const std::string& remote = "origin") const;
    bool setRemoteURL(const std::string& url, const std::string& remote = "origin");
    
    // Error handling
    std::string getLastError() const { return lastError; }
    bool hasErrors() const { return !lastError.empty(); }
    
private:
    std::string repoPath;
    bool initialized;
    std::string lastError;
    
    std::string executeGitCommand(const std::string& command) const;
    bool checkGitInstalled() const;
    void parseStatus(const std::string& output, GitStatus& status) const;
    void parseHistory(const std::string& output, std::vector<GitCommit>& commits) const;
    void parseDiff(const std::string& output, GitDiff& diff) const;
};

} // namespace proxima

#endif // PROXIMA_GIT_INTEGRATION_H