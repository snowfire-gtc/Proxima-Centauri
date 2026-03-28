#pragma once

#include <QObject>
#include <QTextDocument>
#include <QPlainTextEdit>
#include <QMap>
#include <QString>
#include <QColor>
#include "services/collaboration/CollaborationService.h"

namespace centauri::ui {

class RemoteCursorOverlay;

/**
 * @brief Менеджер для управления совместным редактированием текста
 * 
 * Интегрирует CollaborationService с редактором кода, обеспечивая:
 * - Отправку изменений текста другим участникам
 * - Получение и применение удалённых изменений
 * - Отображение курсоров и выделений других пользователей
 * - Синхронизацию версий документа
 */
class CollaborativeTextManager : public QObject {
    Q_OBJECT

public:
    explicit CollaborativeTextManager(QPlainTextEdit* editor, QObject* parent = nullptr);
    ~CollaborativeTextManager() override;

    /**
     * @brief Установить сервис совместной работы
     * @param service Сервис CollaborationService
     */
    void setCollaborationService(proxima::CollaborationService* service);

    /**
     * @brief Начать отслеживание изменений в документе
     * @param filePath Путь к файлу
     */
    void startTracking(const QString& filePath);

    /**
     * @brief Остановить отслеживание изменений
     */
    void stopTracking();

    /**
     * @brief Отправить текущее содержимое документа
     */
    void sendContent();

    /**
     * @brief Применить удалённое изменение
     * @param change Изменение документа
     */
    void applyRemoteChange(const proxima::DocumentChange& change);

    /**
     * @brief Обновить позицию локального курсора
     * @param line Номер строки
     * @param column Позиция в строке
     */
    void updateLocalCursor(int line, int column);

    /**
     * @brief Обновить локальное выделение
     * @param startLine Начало строки
     * @param startCol Начало колонки
     * @param endLine Конец строки
     * @param endCol Конец колонки
     */
    void updateLocalSelection(int startLine, int startCol, int endLine, int endCol);

    /**
     * @brief Получить оверлей для отображения удалённых курсоров
     * @return Указатель на виджет оверлея
     */
    RemoteCursorOverlay* getCursorOverlay() const { return m_cursorOverlay; }

    /**
     * @brief Проверить, активен ли режим совместной работы
     */
    bool isCollaborative() const { return m_collaborationService != nullptr && m_tracking; }

    /**
     * @brief Получить путь к отслеживаемому файлу
     */
    QString getCurrentFile() const { return m_currentFile; }

    /**
     * @brief Задать цвет для локального пользователя
     * @param color Цвет
     */
    void setUserColor(const QColor& color);

signals:
    /**
     * @brief Испущено при получении удалённого изменения
     * @param filePath Путь к файлу
     * @param change Изменение
     */
    void remoteChangeReceived(const QString& filePath, const proxima::DocumentChange& change);

    /**
     * @brief Испущено при обновлении удалённого курсора
     * @param userId ID пользователя
     * @param line Строка
     * @param column Колонка
     */
    void remoteCursorUpdated(const QString& userId, int line, int column);

    /**
     * @brief Испущено при обновлении удалённого выделения
     * @param userId ID пользователя
     * @param startLine Начало строки
     * @param startCol Начало колонки
     * @param endLine Конец строки
     * @param endCol Конец колонки
     */
    void remoteSelectionUpdated(const QString& userId, int startLine, int startCol, int endLine, int endCol);

private slots:
    void onTextChanged();
    void onCursorPositionChanged();
    void onSelectionChanged();
    void onRemoteChangeReceived(const proxima::DocumentChange& change);
    void onRemoteCursorUpdated(const QString& userId, const QString& file, int line, int column);
    void onRemoteSelectionUpdated(const QString& userId, const QString& file,
                                  int startLine, int startCol, int endLine, int endCol);

private:
    void setupConnections();
    void disconnectSignals();
    QString extractChangedText(int startLine, int endLine, const QString& oldContent, const QString& newContent);
    int positionToLine(int position) const;
    int positionToColumn(int position) const;
    int lineToPosition(int line, int column) const;

    QPlainTextEdit* m_editor;
    RemoteCursorOverlay* m_cursorOverlay;
    proxima::CollaborationService* m_collaborationService;
    
    QString m_currentFile;
    QString m_lastContent;
    bool m_tracking;
    bool m_applyingRemoteChange;  // Флаг для предотвращения циклических обновлений
    
    int m_changeCounter;
    qint64 m_lastChangeTime;
};

} // namespace centauri::ui
