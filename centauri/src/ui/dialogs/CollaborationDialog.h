#pragma once

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QStringList>
#include <QMap>

namespace centauri::ui {

/**
 * @brief Диалог для совместной работы над кодом в реальном времени
 * 
 * Позволяет нескольким разработчикам работать над одним проектом одновременно,
 * видеть изменения друг друга, обмениваться сообщениями и управлять сессиями.
 */
class CollaborationDialog : public QDialog {
    Q_OBJECT

public:
    explicit CollaborationDialog(QWidget* parent = nullptr);
    ~CollaborationDialog() override;

    /**
     * @brief Начать новую сессию совместной работы
     * @param projectName Название проекта
     * @return ID сессии
     */
    QString startSession(const QString& projectName);

    /**
     * @brief Присоединиться к существующей сессии
     * @param sessionId ID сессии
     * @param username Имя пользователя
     * @return true если успешно
     */
    bool joinSession(const QString& sessionId, const QString& username);

    /**
     * @brief Покинуть текущую сессию
     */
    void leaveSession();

    /**
     * @brief Отправить сообщение в чат
     * @param message Текст сообщения
     */
    void sendChatMessage(const QString& message);

    /**
     * @brief Синхронизировать изменения файла
     * @param filePath Путь к файлу
     * @param content Новое содержимое
     * @param author Автор изменений
     */
    void syncFileChange(const QString& filePath, const QString& content, const QString& author);

    /**
     * @brief Получить текущий статус подключения
     */
    bool isConnected() const { return m_connected; }

    /**
     * @brief Получить ID текущей сессии
     */
    QString getSessionId() const { return m_sessionId; }

signals:
    void sessionStarted(const QString& sessionId);
    void sessionJoined(const QString& sessionId);
    void sessionLeft();
    void messageReceived(const QString& author, const QString& message);
    void fileChanged(const QString& filePath, const QString& content, const QString& author);
    void userJoined(const QString& username);
    void userLeft(const QString& username);
    void connectionLost();

private slots:
    void onStartClicked();
    void onJoinClicked();
    void onLeaveClicked();
    void onSendClicked();
    void onRefreshClicked();
    void updateStatus();
    void onSessionDoubleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void updateParticipantList();
    void addSystemMessage(const QString& message);
    void setConnected(bool connected);

    // UI элементы
    QListWidget* m_sessionList;
    QListWidget* m_participantList;
    QTextEdit* m_chatText;
    QTextEdit* m_messageInput;
    QPushButton* m_startButton;
    QPushButton* m_joinButton;
    QPushButton* m_leaveButton;
    QPushButton* m_sendButton;
    QPushButton* m_refreshButton;
    QLabel* m_statusLabel;
    QLabel* m_sessionInfoLabel;
    
    // Состояние
    bool m_connected;
    QString m_sessionId;
    QString m_username;
    QStringList m_participants;
    QMap<QString, QString> m_sessions; // sessionId -> projectName
    
    QTimer* m_statusTimer;
};

} // namespace centauri::ui
