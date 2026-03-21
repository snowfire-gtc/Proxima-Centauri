#ifndef CENTAURI_CONSOLETAB_H
#define CENTAURI_CONSOLETAB_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QScrollBar>
#include <QFont>
#include <QColor>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QInputDialog>
#include <QMap>
#include <QVector>
#include <QStackFrame>
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Типы вкладок консоли
 * 
 * Согласно требованию ide.txt пункт 12:
 * - процесс сборки
 * - ошибки сборки приложения
 * - стандартный вывод приложения
 * - call-stack со значениями параметров
 * - REPL (интерактивная консоль Proxima)
 */
enum class ConsoleTabType {
    Build,          // Процесс сборки
    Errors,         // Ошибки сборки
    Output,         // Стандартный вывод
    CallStack,      // Call-stack
    REPL            // REPL консоль (интерактивная)
};

/**
 * @brief Типы сообщений консоли
 */
enum class MessageType {
    Info,           // Информация
    Warning,        // Предупреждение
    Error,          // Ошибка
    Success,        // Успех
    Debug,          // Отладочное сообщение
    Command,        // Команда
    Output,         // Вывод программы
    Input,          // Ввод пользователя
    System          // Системное сообщение
};

/**
 * @brief Структура кадра стека вызовов
 */
struct StackFrame {
    QString function;       // Имя функции
    QString file;           // Файл
    int line;               // Номер строки
    QMap<QString, QString> parameters;  // Параметры
    QMap<QString, QString> localVariables;  // Локальные переменные
};

/**
 * @brief Базовый класс вкладки консоли
 * 
 * Предоставляет базовую функциональность для всех типов вкладок консоли.
 * Согласно требованию ide.txt пункт 12 и 24 (простой, интуитивно понятный код).
 */
class ConsoleTab : public QWidget {
    Q_OBJECT
    
public:
    explicit ConsoleTab(ConsoleTabType type, QWidget *parent = nullptr);
    ~ConsoleTab();
    
    // Тип вкладки
    ConsoleTabType getTabType() const { return tabType; }
    void setTabType(ConsoleTabType type) { tabType = type; }
    
    // Работа с текстом
    void appendText(const QString& text, MessageType type = MessageType::Info);
    void appendHtml(const QString& html);
    void clear();
    void setPlaceholderText(const QString& text);
    QString getPlainText() const;
    QString getHtml() const;
    int getLineCount() const;
    int getCharacterCount() const;
    
    // Прокрутка
    void scrollToBottom();
    void scrollToTop();
    void scrollToLine(int line);
    bool isAtBottom() const;
    
    // Конфигурация
    void setMaxLines(int count);
    int getMaxLines() const { return maxLines; }
    void setAutoScroll(bool enable);
    bool getAutoScroll() const { return autoScroll; }
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;
    void setLineWrap(bool enable);
    bool getLineWrap() const;
    
    // Фильтрация
    void setFilter(const QString& filter);
    QString getFilter() const { return currentFilter; }
    void clearFilter();
    int getFilteredLineCount() const;
    
    // Форматирование
    void setFont(const QFont& font);
    QFont getFont() const;
    void setFontSize(int size);
    int getFontSize() const;
    void setColorScheme(const QString& scheme);
    QString getColorScheme() const { return colorScheme; }
    
    // Выделение
    void selectAll();
    void copy();
    void cut();
    void paste();
    bool hasSelection() const;
    QString getSelectedText() const;
    
    // Поиск
    int findText(const QString& text, bool caseSensitive = false, bool backward = false);
    int findTextNext(const QString& text);
    void highlightText(const QString& text, const QColor& color);
    
    // Состояние
    bool isModified() const { return modified; }
    void setModified(bool m) { modified = m; }
    qint64 getLastModified() const { return lastModified; }
    
    // Экспорт/Импорт
    bool saveToFile(const QString& path);
    bool loadFromFile(const QString& path);
    void exportToHtml(const QString& path);
    void exportToPdf(const QString& path);
    
    // Статистика
    int getMessageCount(MessageType type) const;
    QMap<MessageType, int> getMessageStatistics() const;
    void resetStatistics();
    
signals:
    void textChanged();
    void textSelected(const QString& text);
    void linkClicked(const QString& link);
    void contextMenuRequested(const QPoint& pos);
    void filterChanged(const QString& filter);
    void maxLinesReached();
    
protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    
protected slots:
    virtual void onTextChanged();
    virtual void onSelectionChanged();
    
private slots:
    void onContextMenuAction();
    void onFindText();
    void onClearFilter();
    
private:
    void setupUI();
    void setupConnections();
    void setupContextMenu();
    void applyFilter();
    void updateStatusBar();
    void limitLines();
    QColor getMessageColor(MessageType type) const;
    QTextCharFormat getMessageFormat(MessageType type) const;
    QString formatTimestamp() const;
    void initializeColorSchemes();
    void applyColorScheme();
    int countLines(const QString& text) const;
    QString escapeHtml(const QString& text) const;
    
    // UI компоненты
    QTextEdit* textEdit;              // Основное поле текста
    QLabel* statusLabel;              // Строка состояния
    QWidget* statusWidget;            // Виджет строки состояния
    
    // Конфигурация
    ConsoleTabType tabType;           // Тип вкладки
    int maxLines;                     // Максимальное количество строк
    bool autoScroll;                  // Автопрокрутка
    bool lineWrap;                    // Перенос строк
    QString currentFilter;            // Текущий фильтр
    QString colorScheme;              // Цветовая схема
    
    // Статистика
    QMap<MessageType, int> messageStats;  // Статистика сообщений
    int totalMessages;                      // Всего сообщений
    bool modified;                          // Изменён ли текст
    qint64 lastModified;                    // Время последнего изменения
    
    // Контекстное меню
    QMenu* contextMenu;
    QAction* copyAction;
    QAction* pasteAction;
    QAction* cutAction;
    QAction* selectAllAction;
    QAction* clearAction;
    QAction* saveAction;
    QAction* exportAction;
    QAction* findAction;
    QAction* filterAction;
    QAction* clearFilterAction;
    QAction* scrollToBottomAction;
    QAction* scrollToTopAction;
    
    // Поиск
    QString searchText;
    int searchIndex;
    QList<int> searchResults;
    
    // Стили
    QFont defaultFont;
    QMap<QString, QMap<MessageType, QColor>> colorSchemes;
};

} // namespace proxima

#endif // CENTAURI_CONSOLETAB_H