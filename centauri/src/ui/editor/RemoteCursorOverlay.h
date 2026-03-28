#pragma once

#include <QWidget>
#include <QMap>
#include <QString>
#include <QColor>
#include <QTextCursor>

namespace centauri::ui {

/**
 * @brief Информация о позиции удалённого курсора
 */
struct RemoteCursorInfo {
    QString userId;
    QString username;
    QColor color;
    int position;         // Позиция в документе
    int line;             // Номер строки
    int column;           // Позиция в строке
    qint64 lastUpdate;    // Время последнего обновления
    bool active;          // Активен ли курсор
};

/**
 * @brief Информация о выделении удалённого пользователя
 */
struct RemoteSelectionInfo {
    QString userId;
    QString username;
    QColor color;
    int start;            // Начало выделения
    int end;              // Конец выделения
    int startLine;
    int startColumn;
    int endLine;
    int endColumn;
};

/**
 * @brief Виджет для отображения удалённых курсоров и выделений в редакторе кода
 * 
 * Отрисовывает цветные индикаторы позиций других участников совместной сессии,
 * позволяя видеть, где они работают в реальном времени.
 */
class RemoteCursorOverlay : public QWidget {
    Q_OBJECT

public:
    explicit RemoteCursorOverlay(QWidget* parent = nullptr);
    ~RemoteCursorOverlay() override;

    /**
     * @brief Обновить позицию удалённого курсора
     * @param cursorInfo Информация о курсоре
     */
    void updateCursor(const RemoteCursorInfo& cursorInfo);

    /**
     * @brief Удалить курсор пользователя
     * @param userId ID пользователя
     */
    void removeCursor(const QString& userId);

    /**
     * @brief Обновить выделение пользователя
     * @param selectionInfo Информация о выделении
     */
    void updateSelection(const RemoteSelectionInfo& selectionInfo);

    /**
     * @brief Удалить выделение пользователя
     * @param userId ID пользователя
     */
    void removeSelection(const QString& userId);

    /**
     * @brief Очистить все курсоры и выделения
     */
    void clearAll();

    /**
     * @brief Получить информацию о курсоре пользователя
     * @param userId ID пользователя
     * @return Информация о курсоре или пустой объект если не найден
     */
    RemoteCursorInfo getCursor(const QString& userId) const;

    /**
     * @brief Получить все активные курсоры
     * @return Список курсоров
     */
    QMap<QString, RemoteCursorInfo> getAllCursors() const { return m_cursors; }

    /**
     * @brief Задать смещение относительно редактора
     * @param offset Смещение в пикселях
     */
    void setOffset(int offset) { m_offset = offset; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void drawCursor(QPainter& painter, const RemoteCursorInfo& cursor);
    void drawSelection(QPainter& painter, const RemoteSelectionInfo& selection);
    QRect cursorRect(const RemoteCursorInfo& cursor) const;
    QRect selectionRect(const RemoteSelectionInfo& selection) const;

    QMap<QString, RemoteCursorInfo> m_cursors;
    QMap<QString, RemoteSelectionInfo> m_selections;
    
    int m_offset = 2;  // Смещение для отрисовки
    int m_cursorWidth = 2;
    int m_labelHeight = 16;
};

} // namespace centauri::ui
