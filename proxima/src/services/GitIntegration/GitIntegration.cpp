#include "GitIntegration.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <array>
#include <regex>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace proxima {

GitIntegration::GitIntegration() : initialized(false) {}

GitIntegration::~GitIntegration() {}

bool GitIntegration::initialize(const std::string& path) {
    repoPath = path;
    
    if (!checkGitInstalled()) {
        lastError = "Git is not installed or not in PATH";
        return false;
    }
    
    // Check if directory is a git repository
    std::string output = executeGitCommand("rev-parse --git-dir");
    
    if (output.find("fatal") != std::string::npos) {
        lastError = "Not a git repository: " + path;
        return false;
    }
    
    initialized = true;
    std::cout << "[Git] Initialized: " << repoPath << std::endl;
    return true;
}

bool GitIntegration::checkGitInstalled() const {
    std::string output = executeGitCommand("--version");
    return output.find("git version") != std::string::npos;
}

GitStatus GitIntegration::getStatus() const {
    GitStatus status;
    status.isRepository = initialized;
    
    if (!initialized) return status;
    
    // Get current branch
    status.branch = getCurrentBranch();
    
    // Get status output
    std::string output = executeGitCommand("status --porcelain");
    parseStatus(output, status);
    
    // Get last commit info
    std::string logOutput = executeGitCommand("log -1 --format=\"%H|%s|%an|%ai\"");
    std::istringstream iss(logOutput);
    std::string line;
    
    if (std::getline(iss, line)) {
        std::vector<std::string> parts;
        std::istringstream partStream(line);
        std::string part;
        
        while (std::getline(partStream, part, '|')) {
            parts.push_back(part);
        }
        
        if (parts.size() >= 4) {
            status.lastCommit = parts[0];
            status.lastCommitMessage = parts[1];
            status.lastCommitDate = parts[3];
        }
    }
    
    return status;
}

bool GitIntegration::hasChanges() const {
    GitStatus status = getStatus();
    return !status.modifiedFiles.empty() || 
           !status.untrackedFiles.empty() ||
           !status.stagedFiles.empty();
}

bool GitIntegration::add(const std::string& file) {
    std::string output = executeGitCommand("add \"" + file + "\"");
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::addAll() {
    std::string output = executeGitCommand("add .");
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::commit(const std::string& message) {
    std::string output = executeGitCommand("commit -m \"" + message + "\"");
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::push(const std::string& remote) {
    std::string output = executeGitCommand("push " + remote);
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::pull(const std::string& remote) {
    std::string output = executeGitCommand("pull " + remote);
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::fetch(const std::string& remote) {
    std::string output = executeGitCommand("fetch " + remote);
    return output.find("fatal") == std::string::npos;
}

std::string GitIntegration::getCurrentBranch() const {
    std::string output = executeGitCommand("rev-parse --abbrev-ref HEAD");
    
    if (output.find("fatal") != std::string::npos) {
        return "";
    }
    
    // Remove newline
    output.erase(output.find_last_not_of("\n\r") + 1);
    return output;
}

std::vector<std::string> GitIntegration::getBranches() const {
    std::vector<std::string> branches;
    
    std::string output = executeGitCommand("branch");
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Remove current branch marker
        if (line.find("*") == 0) {
            line = line.substr(2);
        }
        line.erase(0, line.find_first_not_of(" \t"));
        branches.push_back(line);
    }
    
    return branches;
}

bool GitIntegration::createBranch(const std::string& name) {
    std::string output = executeGitCommand("branch " + name);
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::checkoutBranch(const std::string& name) {
    std::string output = executeGitCommand("checkout " + name);
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::deleteBranch(const std::string& name) {
    std::string output = executeGitCommand("branch -d " + name);
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::mergeBranch(const std::string& name) {
    std::string output = executeGitCommand("merge " + name);
    return output.find("fatal") == std::string::npos;
}

std::vector<GitCommit> GitIntegration::getHistory(int count) const {
    std::vector<GitCommit> commits;
    
    std::string output = executeGitCommand(
        "log -" + std::to_string(count) + " --format=\"%H|%s|%an|%ai|%P\""
    );
    
    parseHistory(output, commits);
    
    return commits;
}

GitDiff GitIntegration::getDiff(const std::string& file) const {
    GitDiff diff;
    diff.filename = file;
    
    std::string output = executeGitCommand("diff HEAD -- \"" + file + "\"");
    parseDiff(output, diff);
    
    return diff;
}

GitDiff GitIntegration::getDiffWithBranch(const std::string& file, 
                                          const std::string& branch) const {
    GitDiff diff;
    diff.filename = file;
    
    std::string output = executeGitCommand(
        "diff " + branch + " HEAD -- \"" + file + "\""
    );
    parseDiff(output, diff);
    
    return diff;
}

bool GitIntegration::stash(const std::string& message) {
    std::string cmd = "stash";
    if (!message.empty()) {
        cmd += " push -m \"" + message + "\"";
    }
    
    std::string output = executeGitCommand(cmd);
    return output.find("fatal") == std::string::npos;
}

bool GitIntegration::stashPop() {
    std::string output = executeGitCommand("stash pop");
    return output.find("fatal") == std::string::npos;
}

std::string GitIntegration::getRemoteURL(const std::string& remote) const {
    std::string output = executeGitCommand("remote get-url " + remote);
    
    if (output.find("fatal") != std::string::npos) {
        return "";
    }
    
    output.erase(output.find_last_not_of("\n\r") + 1);
    return output;
}

bool GitIntegration::setRemoteURL(const std::string& url, const std::string& remote) {
    std::string output = executeGitCommand("remote set-url " + remote + " \"" + url + "\"");
    return output.find("fatal") == std::string::npos;
}

std::string GitIntegration::executeGitCommand(const std::string& command) const {
    std::array<char, 128> buffer;
    std::string result;
    
    std::string fullCommand = "cd \"" + repoPath + "\" && git " + command + " 2>&1";
    
    FILE* pipe = popen(fullCommand.c_str(), "r");
    if (!pipe) {
        return "fatal: Could not execute command";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    pclose(pipe);
    return result;
}

void GitIntegration::parseStatus(const std::string& output, GitStatus& status) const {
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.length() < 3) continue;
        
        char indexStatus = line[0];
        char worktreeStatus = line[1];
        std::string filename = line.substr(3);
        
        if (indexStatus != ' ' && indexStatus != '?') {
            status.stagedFiles.push_back(filename);
        }
        
        if (worktreeStatus != ' ' && worktreeStatus != '?') {
            status.modifiedFiles.push_back(filename);
        }
        
        if (indexStatus == '?' && worktreeStatus == '?') {
            status.untrackedFiles.push_back(filename);
        }
    }
}

void GitIntegration::parseHistory(const std::string& output, 
                                  std::vector<GitCommit>& commits) const {
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        GitCommit commit;
        std::istringstream partStream(line);
        std::string part;
        std::vector<std::string> parts;
        
        while (std::getline(partStream, part, '|')) {
            parts.push_back(part);
        }
        
        if (parts.size() >= 4) {
            commit.hash = parts[0];
            commit.message = parts[1];
            commit.author = parts[2];
            commit.date = parts[3];
            
            if (parts.size() > 4) {
                std::istringstream parentStream(parts[4]);
                std::string parent;
                while (parentStream >> parent) {
                    commit.parents.push_back(parent);
                }
            }
            
            commits.push_back(commit);
        }
    }
}

void GitIntegration::parseDiff(const std::string& output, GitDiff& diff) const {
    std::istringstream iss(output);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.find("+") == 0 && line.find("+++") != 0) {
            diff.addedLines.push_back(line.substr(1));
            diff.addedCount++;
        } else if (line.find("-") == 0 && line.find("---") != 0) {
            diff.removedLines.push_back(line.substr(1));
            diff.removedCount++;
        }
    }
}

} // namespace proxima