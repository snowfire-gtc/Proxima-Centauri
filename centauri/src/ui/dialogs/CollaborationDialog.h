#ifndef CENTAURI_COLLABORATIONDIALOG_H
#define CENTAURI_COLLABORATIONDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include "services/collaboration/CollaborationService.h"

namespace proxima {

class CollaborationDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit CollaborationDialog(QWidget *parent = nullptr);
    ~CollaborationDialog();
    
    // Session management
    bool hostSession(const QString& projectName, int port);
    bool joinSession(const QString& hostAddress, int port, const QString& password);
    bool leaveSession();
    bool isInSession() const { return inSession; }
    
    // User management
    void setCurrentUser(const QString& username, const QString& color);
    QString getCurrentUsername() const { return currentUsername; }
    
    // File operations
    void openFile(const QString& file);
    void closeFile(const QString& file);
    void lockFile(const QString& file);
    void unlockFile(const QString& file);
    
    // Cursor tracking
    void updateCursor(const QString& file, int line, int column);
    void updateSelection(const QString& file, int startLine, int startCol, int endLine, int endCol);
    
signals:
    void sessionStarted(const QString& sessionId);
    void sessionJoined(const QString& sessionId);
    void sessionEnded();
    void changeReceived(const DocumentChange& change);
    void cursorUpdated(const QString& userId, const QString& file, int line, int column);
    void selectionUpdated(const QString& userId, const QString& file, int startLine, int startCol, int endLine, int endCol);
    
private slots:
    void onHostSession();
    void onJoinSession();
    void onLeaveSession();
    void onRefresh();
    void onSendChange();
    void onRollbackSelected();
    void onRollbackAll();
    void onParticipantSelected(int row);
    void onConnectionStatusChanged(bool connected);
    void onParticipantJoined(const UserInfo& user);
    void onParticipantLeft(const UserInfo& user);
    void onChangeReceived(const DocumentChange& change);
    void onVersionUpdated(int version);
    
private:
    void setupUI();
    void setupSessionGroup();
    void setupParticipantsGroup();
    void setupHistoryGroup();
    void setupButtonGroup();
    void updateParticipantList();
    void updateHistoryList();
    void updateStatus();
    
    // Session group
    QGroupBox* sessionGroup;
    QLineEdit* hostEdit;
    QSpinBox* portSpin;
    QLineEdit* passwordEdit;
    QLineEdit* usernameEdit;
    QCheckBox* enableEncryptionCheck;
    QPushButton* hostButton;
    QPushButton* joinButton;
    QPushButton* leaveButton;
    QLabel* statusLabel;
    QLabel* sessionIdLabel;
    
    // Participants group
    QGroupBox* participantsGroup;
    QTableWidget* participantsTable;
    QPushButton* refreshButton;
    
    // History group
    QGroupBox* historyGroup;
    QListWidget* historyList;
    QPushButton* rollbackButton;
    QPushButton* rollbackAllButton;
    
    // Buttons
    QPushButton* closeButton;
    
    // Service
    CollaborationService* collaborationService;
    
    // State
    bool inSession;
    bool isHosting;
    QString currentUsername;
    QString currentUserColor;
    QString sessionId;
    int currentVersion;
};

} // namespace proxima

#endif // CENTAURI_COLLABORATIONDIALOG_H