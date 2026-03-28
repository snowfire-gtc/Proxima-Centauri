#include "CollaborativeTextManager.h"
#include "RemoteCursorOverlay.h"
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextDocument>
#include <QScrollBar>

namespace centauri::ui {

CollaborativeTextManager::CollaborativeTextManager(QPlainTextEdit* editor, QObject* parent)
    : QObject(parent)
    , m_editor(editor)
    , m_cursorOverlay(nullptr)
    , m_collaborationService(nullptr)
    , m_tracking(false)
    , m_applyingRemoteChange(false)
    , m_changeCounter(0)
    , m_lastChangeTime(0)
{
    m_cursorOverlay = new RemoteCursorOverlay(m_editor);
    m_cursorOverlay->setParent(m_editor);
    m_cursorOverlay->resize(m_editor->viewport()->size());
    
    setupConnections();
}

CollaborativeTextManager::~CollaborativeTextManager() {
    disconnectSignals();
}

void CollaborativeTextManager::setCollaborationService(proxima::CollaborationService* service) {
    if (m_collaborationService == service) return;
    
    disconnectSignals();
    
    m_collaborationService = service;
    
    if (m_collaborationService) {
        connect(m_collaborationService, &proxima::CollaborationService::changeReceived,
                this, &CollaborativeTextManager::onRemoteChangeReceived);
        connect(m_collaborationService, &proxima::CollaborationService::cursorUpdated,
                this, &CollaborativeTextManager::onRemoteCursorUpdated);
        connect(m_collaborationService, &proxima::CollaborationService::selectionUpdated,
                this, &CollaborativeTextManager::onRemoteSelectionUpdated);
        
        // Установить цвет пользователя
        auto user = m_collaborationService->getCurrentUser();
        if (!user.color.isEmpty()) {
            setUserColor(QColor(user.color));
        }
    }
}

void CollaborativeTextManager::startTracking(const QString& filePath) {
    m_currentFile = filePath;
    m_lastContent = m_editor->toPlainText();
    m_tracking = true;
    
    if (m_collaborationService) {
        m_collaborationService->openFile(filePath);
    }
}

void CollaborativeTextManager::stopTracking() {
    if (m_tracking && m_collaborationService && !m_currentFile.isEmpty()) {
        m_collaborationService->closeFile(m_currentFile);
    }
    
    m_tracking = false;
    m_currentFile.clear();
    m_lastContent.clear();
    
    if (m_cursorOverlay) {
        m_cursorOverlay->clearAll();
    }
}

void CollaborativeTextManager::sendContent() {
    if (!m_tracking || !m_collaborationService) return;
    
    QString currentContent = m_editor->toPlainText();
    if (currentContent == m_lastContent) return;
    
    // Создаём изменение для всего документа
    proxima::DocumentChange change;
    change.id = ++m_changeCounter;
    change.file = m_currentFile;
    change.startLine = 0;
    change.endLine = m_editor->blockCount();
    change.oldContent = m_lastContent;
    change.newContent = currentContent;
    
    m_collaborationService->sendChange(change);
    m_lastContent = currentContent;
    m_lastChangeTime = QDateTime::currentMSecsSinceEpoch();
}

void CollaborativeTextManager::applyRemoteChange(const proxima::DocumentChange& change) {
    if (!m_editor) return;
    
    m_applyingRemoteChange = true;
    
    // Сохраняем текущую позицию курсора
    QTextCursor cursor = m_editor->textCursor();
    int cursorPosition = cursor.position();
    
    // Применяем изменение
    QString content = m_editor->toPlainText();
    
    if (change.startLine >= 0 && change.endLine >= 0) {
        // Замена по строкам
        QStringList lines = content.split('\n');
        QStringList newLines = change.newContent.split('\n');
        
        if (change.startLine < lines.size()) {
            int removeCount = change.endLine - change.startLine + 1;
            
            // Удаляем старые строки
            for (int i = 0; i < removeCount && change.startLine < lines.size(); ++i) {
                lines.removeAt(change.startLine);
            }
            
            // Вставляем новые строки
            for (int i = 0; i < newLines.size(); ++i) {
                lines.insert(change.startLine + i, newLines[i]);
            }
            
            content = lines.join('\n');
        }
    } else {
        // Полная замена содержимого
        content = change.newContent;
    }
    
    m_editor->setPlainText(content);
    m_lastContent = content;
    
    // Восстанавливаем курсор (с коррекцией если нужно)
    if (cursorPosition <= content.size()) {
        cursor.setPosition(cursorPosition);
        m_editor->setTextCursor(cursor);
    }
    
    m_applyingRemoteChange = false;
    
    emit remoteChangeReceived(m_currentFile, change);
}

void CollaborativeTextManager::updateLocalCursor(int line, int column) {
    if (!m_collaborationService || !m_tracking) return;
    
    m_collaborationService->updateCursor(m_currentFile, line, column);
}

void CollaborativeTextManager::updateLocalSelection(int startLine, int startCol, int endLine, int endCol) {
    if (!m_collaborationService || !m_tracking) return;
    
    m_collaborationService->updateSelection(m_currentFile, startLine, startCol, endLine, endCol);
}

void CollaborativeTextManager::setUserColor(const QColor& color) {
    if (m_collaborationService) {
        m_collaborationService->setUserColor(color.name());
    }
}

void CollaborativeTextManager::setupConnections() {
    if (!m_editor) return;
    
    connect(m_editor->document(), &QTextDocument::contentsChanged,
            this, &CollaborativeTextManager::onTextChanged);
    connect(&m_editor->cursorChanged(), SIGNAL(),
            this, &CollaborativeTextManager::onCursorPositionChanged());
}

void CollaborativeTextManager::disconnectSignals() {
    if (m_collaborationService) {
        disconnect(m_collaborationService, nullptr, this, nullptr);
    }
}

void CollaborativeTextManager::onTextChanged() {
    if (m_applyingRemoteChange || !m_tracking) return;
    
    // Отправляем изменения с небольшой задержкой для пакетирования
    // В реальной реализации здесь нужен таймер
    sendContent();
}

void CollaborativeTextManager::onCursorPositionChanged() {
    if (!m_tracking || !m_editor) return;
    
    QTextCursor cursor = m_editor->textCursor();
    int position = cursor.position();
    int line = positionToLine(position);
    int column = positionToColumn(position);
    
    updateLocalCursor(line, column);
    
    // Проверяем выделение
    if (cursor.hasSelection()) {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        updateLocalSelection(
            positionToLine(start), positionToColumn(start),
            positionToLine(end), positionToColumn(end)
        );
    }
}

void CollaborativeTextManager::onSelectionChanged() {
    onCursorPositionChanged();
}

void CollaborativeTextManager::onRemoteChangeReceived(const proxima::DocumentChange& change) {
    if (change.file != m_currentFile) return;
    
    applyRemoteChange(change);
}

void CollaborativeTextManager::onRemoteCursorUpdated(const QString& userId, const QString& file, 
                                                      int line, int column) {
    if (file != m_currentFile || !m_cursorOverlay) return;
    
    // Получаем информацию о пользователе
    proxima::UserInfo userInfo = m_collaborationService->getUserInfo(userId);
    
    RemoteCursorInfo cursorInfo;
    cursorInfo.userId = userId;
    cursorInfo.username = userInfo.username;
    cursorInfo.color = QColor(userInfo.color);
    cursorInfo.line = line;
    cursorInfo.column = column;
    cursorInfo.position = lineToPosition(line, column);
    cursorInfo.lastUpdate = QDateTime::currentMSecsSinceEpoch();
    cursorInfo.active = true;
    
    m_cursorOverlay->updateCursor(cursorInfo);
    
    emit remoteCursorUpdated(userId, line, column);
}

void CollaborativeTextManager::onRemoteSelectionUpdated(const QString& userId, const QString& file,
                                                         int startLine, int startCol, 
                                                         int endLine, int endCol) {
    if (file != m_currentFile || !m_cursorOverlay) return;
    
    proxima::UserInfo userInfo = m_collaborationService->getUserInfo(userId);
    
    RemoteSelectionInfo selectionInfo;
    selectionInfo.userId = userId;
    selectionInfo.username = userInfo.username;
    selectionInfo.color = QColor(userInfo.color);
    selectionInfo.startLine = startLine;
    selectionInfo.startColumn = startCol;
    selectionInfo.endLine = endLine;
    selectionInfo.endColumn = endCol;
    selectionInfo.start = lineToPosition(startLine, startCol);
    selectionInfo.end = lineToPosition(endLine, endCol);
    
    m_cursorOverlay->updateSelection(selectionInfo);
    
    emit remoteSelectionUpdated(userId, startLine, startCol, endLine, endCol);
}

QString CollaborativeTextManager::extractChangedText(int startLine, int endLine, 
                                                      const QString& oldContent, 
                                                      const QString& newContent) {
    Q_UNUSED(startLine)
    Q_UNUSED(endLine)
    Q_UNUSED(oldContent)
    
    // Упрощённая реализация - возвращаем новое содержимое
    return newContent;
}

int CollaborativeTextManager::positionToLine(int position) const {
    if (!m_editor) return 0;
    
    QTextCursor cursor(m_editor->document());
    cursor.setPosition(position);
    return cursor.blockNumber();
}

int CollaborativeTextManager::positionToColumn(int position) const {
    if (!m_editor) return 0;
    
    QTextCursor cursor(m_editor->document());
    cursor.setPosition(position);
    return cursor.positionInBlock();
}

int CollaborativeTextManager::lineToPosition(int line, int column) const {
    if (!m_editor) return 0;
    
    QTextCursor cursor(m_editor->document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
    
    return cursor.position();
}

} // namespace centauri::ui
