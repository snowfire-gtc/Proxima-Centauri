#include "RemoteCursorOverlay.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QDateTime>

namespace centauri::ui {

RemoteCursorOverlay::RemoteCursorOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::NoFocus);
}

RemoteCursorOverlay::~RemoteCursorOverlay() = default;

void RemoteCursorOverlay::updateCursor(const RemoteCursorInfo& cursorInfo) {
    m_cursors[cursorInfo.userId] = cursorInfo;
    update();
}

void RemoteCursorOverlay::removeCursor(const QString& userId) {
    m_cursors.remove(userId);
    update();
}

void RemoteCursorOverlay::updateSelection(const RemoteSelectionInfo& selectionInfo) {
    m_selections[selectionInfo.userId] = selectionInfo;
    update();
}

void RemoteCursorOverlay::removeSelection(const QString& userId) {
    m_selections.remove(userId);
    update();
}

void RemoteCursorOverlay::clearAll() {
    m_cursors.clear();
    m_selections.clear();
    update();
}

RemoteCursorInfo RemoteCursorOverlay::getCursor(const QString& userId) const {
    return m_cursors.value(userId);
}

void RemoteCursorOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Отрисовка выделений (на заднем плане)
    for (auto it = m_selections.begin(); it != m_selections.end(); ++it) {
        drawSelection(painter, it.value());
    }
    
    // Отрисовка курсоров
    for (auto it = m_cursors.begin(); it != m_cursors.end(); ++it) {
        drawCursor(painter, it.value());
    }
}

void RemoteCursorOverlay::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update();
}

void RemoteCursorOverlay::drawCursor(QPainter& painter, const RemoteCursorInfo& cursor) {
    if (!cursor.active) return;
    
    QRect rect = cursorRect(cursor);
    if (!rect.isValid()) return;
    
    // Рисуем цветную линию курсора
    painter.fillRect(rect, cursor.color);
    
    // Рисуем метку с именем пользователя
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    
    QFontMetrics fm(font);
    QString label = cursor.username;
    int labelWidth = fm.horizontalAdvance(label) + 4;
    int labelHeight = fm.height() + 2;
    
    QRect labelRect(rect.left(), rect.top() - labelHeight - 2, labelWidth, labelHeight);
    
    // Фон метки
    painter.setPen(cursor.color.darker(120));
    painter.setBrush(cursor.color);
    painter.drawRoundedRect(labelRect, 3, 3);
    
    // Текст метки
    painter.setPen(Qt::white);
    painter.drawText(labelRect, Qt::AlignCenter, label);
}

void RemoteCursorOverlay::drawSelection(QPainter& painter, const RemoteSelectionInfo& selection) {
    QRect rect = selectionRect(selection);
    if (!rect.isValid()) return;
    
    // Полупрозрачное выделение
    QColor color = selection.color;
    color.setAlpha(60);
    painter.fillRect(rect, color);
}

QRect RemoteCursorOverlay::cursorRect(const RemoteCursorInfo& cursor) const {
    // Преобразование позиции в координаты виджета
    // В реальной реализации здесь будет конвертация из позиции документа
    // в экранные координаты через QTextEdit/QPlainTextEdit
    
    if (cursor.position < 0 || cursor.line < 0) {
        return QRect();
    }
    
    // Примерная реализация для демонстрации
    int lineHeight = 16;  // Высота строки
    int charWidth = 8;    // Ширина символа
    
    int x = cursor.column * charWidth + m_offset;
    int y = cursor.line * lineHeight;
    int height = lineHeight;
    
    return QRect(x, y, m_cursorWidth, height);
}

QRect RemoteCursorOverlay::selectionRect(const RemoteSelectionInfo& selection) const {
    if (selection.start < 0 || selection.end < 0) {
        return QRect();
    }
    
    int lineHeight = 16;
    int charWidth = 8;
    
    int startX = selection.startColumn * charWidth;
    int startY = selection.startLine * lineHeight;
    int endX = selection.endColumn * charWidth;
    int endY = selection.endLine * lineHeight;
    
    if (startLine == endLine) {
        // Выделение в одной строке
        return QRect(startX, startY, endX - startX, lineHeight);
    } else {
        // Многострочное выделение (упрощённо)
        return QRect(startX, startY, width() - startX, (endLine - startLine + 1) * lineHeight);
    }
}

} // namespace centauri::ui
