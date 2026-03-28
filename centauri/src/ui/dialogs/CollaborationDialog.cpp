#include "CollaborationDialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QUuid>

namespace centauri::ui {

CollaborationDialog::CollaborationDialog(QWidget* parent)
    : QDialog(parent)
    , m_connected(false)
{
    setWindowTitle("Совместная работа");
    setMinimumSize(700, 500);
    resize(800, 600);
    
    setupUI();
    
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &CollaborationDialog::updateStatus);
    m_statusTimer->start(5000); // Обновление статуса каждые 5 секунд
}

CollaborationDialog::~CollaborationDialog() = default;

void CollaborationDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Статус бар
    auto* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Не подключено");
    m_statusLabel->setStyleSheet("font-weight: bold; color: red;");
    statusLayout->addWidget(m_statusLabel);
    
    m_sessionInfoLabel = new QLabel("");
    m_sessionInfoLabel->setStyleSheet("color: gray;");
    statusLayout->addWidget(m_sessionInfoLabel);
    
    statusLayout->addStretch();
    
    m_refreshButton = new QPushButton("Обновить");
    connect(m_refreshButton, &QPushButton::clicked, this, &CollaborationDialog::onRefreshClicked);
    statusLayout->addWidget(m_refreshButton);
    
    layout->addLayout(statusLayout);
    
    // Основная часть с двумя колонками
    auto* mainLayout = new QHBoxLayout();
    
    // Левая колонка - сессии и участники
    auto* leftLayout = new QVBoxLayout();
    
    auto* sessionLabel = new QLabel("Доступные сессии:");
    sessionLabel->setFont(QFont("", 10, QFont::Bold));
    leftLayout->addWidget(sessionLabel);
    
    m_sessionList = new QListWidget();
    m_sessionList->setMinimumHeight(150);
    connect(m_sessionList, &QListWidget::itemDoubleClicked, 
            this, &CollaborationDialog::onSessionDoubleClicked);
    leftLayout->addWidget(m_sessionList);
    
    auto* participantLabel = new QLabel("Участники:");
    participantLabel->setFont(QFont("", 10, QFont::Bold));
    leftLayout->addWidget(participantLabel);
    
    m_participantList = new QListWidget();
    m_participantList->setMinimumHeight(100);
    leftLayout->addWidget(m_participantList);
    
    // Кнопки управления
    auto* buttonLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton("Создать сессию");
    connect(m_startButton, &QPushButton::clicked, this, &CollaborationDialog::onStartClicked);
    buttonLayout->addWidget(m_startButton);
    
    m_joinButton = new QPushButton("Присоединиться");
    connect(m_joinButton, &QPushButton::clicked, this, &CollaborationDialog::onJoinClicked);
    buttonLayout->addWidget(m_joinButton);
    
    m_leaveButton = new QPushButton("Покинуть");
    m_leaveButton->setEnabled(false);
    connect(m_leaveButton, &QPushButton::clicked, this, &CollaborationDialog::onLeaveClicked);
    buttonLayout->addWidget(m_leaveButton);
    
    buttonLayout->addStretch();
    leftLayout->addLayout(buttonLayout);
    
    mainLayout->addLayout(leftLayout);
    
    // Правая колонка - чат
    auto* rightLayout = new QVBoxLayout();
    
    auto* chatLabel = new QLabel("Чат:");
    chatLabel->setFont(QFont("", 10, QFont::Bold));
    rightLayout->addWidget(chatLabel);
    
    m_chatText = new QTextEdit();
    m_chatText->setReadOnly(true);
    rightLayout->addWidget(m_chatText);
    
    m_messageInput = new QTextEdit();
    m_messageInput->setMaximumHeight(80);
    m_messageInput->setPlaceholderText("Введите сообщение...");
    rightLayout->addWidget(m_messageInput);
    
    m_sendButton = new QPushButton("Отправить");
    connect(m_sendButton, &QPushButton::clicked, this, &CollaborationDialog::onSendClicked);
    rightLayout->addWidget(m_sendButton);
    
    mainLayout->addLayout(rightLayout);
    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 1);
    
    layout->addLayout(mainLayout);
}

QString CollaborationDialog::startSession(const QString& projectName) {
    m_sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    m_username = "Host";
    
    m_sessions[m_sessionId] = projectName;
    m_participants.clear();
    m_participants.append(m_username + " (хост)");
    
    setConnected(true);
    updateParticipantList();
    
    addSystemMessage(QString("Сессия '%1' создана для проекта: %2").arg(m_sessionId, projectName));
    emit sessionStarted(m_sessionId);
    
    return m_sessionId;
}

bool CollaborationDialog::joinSession(const QString& sessionId, const QString& username) {
    if (!m_sessions.contains(sessionId)) {
        QMessageBox::warning(this, "Ошибка", "Сессия не найдена");
        return false;
    }
    
    m_sessionId = sessionId;
    m_username = username;
    m_participants.append(username);
    
    setConnected(true);
    updateParticipantList();
    
    addSystemMessage(QString("Пользователь '%1' присоединился к сессии").arg(username));
    emit sessionJoined(sessionId);
    emit userJoined(username);
    
    return true;
}

void CollaborationDialog::leaveSession() {
    if (!m_connected) return;
    
    QString user = m_username;
    addSystemMessage(QString("Пользователь '%1' покинул сессию").arg(user));
    
    m_participants.removeAll(user);
    setConnected(false);
    
    m_sessionId.clear();
    m_username.clear();
    m_participants.clear();
    
    updateParticipantList();
    emit sessionLeft();
    emit userLeft(user);
}

void CollaborationDialog::sendChatMessage(const QString& message) {
    if (message.trimmed().isEmpty() || !m_connected) return;
    
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString formattedMessage = QString("[%1] %2: %3").arg(timestamp, m_username, message);
    
    m_chatText->append(formattedMessage);
    emit messageReceived(m_username, message);
    
    m_messageInput->clear();
}

void CollaborationDialog::syncFileChange(const QString& filePath, const QString& content, const QString& author) {
    Q_UNUSED(content)
    if (!m_connected) return;
    
    addSystemMessage(QString("Файл изменён: %1 (автор: %2)").arg(filePath, author));
    emit fileChanged(filePath, content, author);
}

void CollaborationDialog::onStartClicked() {
    bool ok = false;
    QString projectName = QInputDialog::getText(
        this,
        "Создать сессию",
        "Название проекта:",
        QLineEdit::Normal,
        "MyProject",
        &ok
    );
    
    if (ok && !projectName.trimmed().isEmpty()) {
        startSession(projectName.trimmed());
    }
}

void CollaborationDialog::onJoinClicked() {
    bool ok = false;
    QString sessionId = QInputDialog::getText(
        this,
        "Присоединиться к сессии",
        "ID сессии:",
        QLineEdit::Normal,
        "",
        &ok
    );
    
    if (!ok || sessionId.trimmed().isEmpty()) return;
    
    ok = false;
    QString username = QInputDialog::getText(
        this,
        "Ваше имя",
        "Введите ваше имя:",
        QLineEdit::Normal,
        "User",
        &ok
    );
    
    if (ok && !username.trimmed().isEmpty()) {
        joinSession(sessionId.trimmed(), username.trimmed());
    }
}

void CollaborationDialog::onLeaveClicked() {
    leaveSession();
}

void CollaborationDialog::onSendClicked() {
    sendChatMessage(m_messageInput->toPlainText());
}

void CollaborationDialog::onRefreshClicked() {
    m_sessionList->clear();
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        auto* item = new QListWidgetItem(QString("%1 - %2").arg(it.key(), it.value()));
        item->setData(Qt::UserRole, it.key());
        m_sessionList->addItem(item);
    }
}

void CollaborationDialog::updateStatus() {
    if (m_connected) {
        m_statusLabel->setText("Подключено");
        m_statusLabel->setStyleSheet("font-weight: bold; color: green;");
        m_sessionInfoLabel->setText(QString("Сессия: %1").arg(m_sessionId));
    } else {
        m_statusLabel->setText("Не подключено");
        m_statusLabel->setStyleSheet("font-weight: bold; color: red;");
        m_sessionInfoLabel->clear();
    }
}

void CollaborationDialog::onSessionDoubleClicked(QListWidgetItem* item) {
    QString sessionId = item->data(Qt::UserRole).toString();
    
    bool ok = false;
    QString username = QInputDialog::getText(
        this,
        "Ваше имя",
        "Введите ваше имя:",
        QLineEdit::Normal,
        "User",
        &ok
    );
    
    if (ok && !username.trimmed().isEmpty()) {
        joinSession(sessionId, username.trimmed());
    }
}

void CollaborationDialog::updateParticipantList() {
    m_participantList->clear();
    for (const auto& participant : m_participants) {
        m_participantList->addItem(participant);
    }
}

void CollaborationDialog::addSystemMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString formattedMessage = QString("[%1] <SYSTEM>: %2").arg(timestamp, message);
    
    m_chatText->append(formattedMessage);
    m_chatText->scrollToBottom();
}

void CollaborationDialog::setConnected(bool connected) {
    m_connected = connected;
    m_startButton->setEnabled(!connected);
    m_joinButton->setEnabled(!connected);
    m_leaveButton->setEnabled(connected);
    m_sendButton->setEnabled(connected);
    m_messageInput->setEnabled(connected);
}

} // namespace centauri::ui
