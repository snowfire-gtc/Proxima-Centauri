#ifndef CENTAURI_SNIPPETMANAGER_H
#define CENTAURI_SNIPPETMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

namespace proxima {

struct SnippetPlaceholder {
    int id;                     // Номер placeholder'а (1, 2, 3, ...)
    int startPosition;          // Позиция начала в документе
    int endPosition;            // Позиция конца в документе
    QString defaultText;        // Текст по умолчанию
    QString userText;           // Текст, введённый пользователем
    bool isActive;              // Активен ли сейчас этот placeholder
    bool isVisited;             // Посещал ли уже пользователь этот placeholder
    int tabIndex;               // Порядок обхода (для одинаковых номеров)
    
    SnippetPlaceholder() : id(0), startPosition(0), endPosition(0),
                           isActive(false), isVisited(false), tabIndex(0) {}
};

struct SnippetSession {
    bool isActive;              // Активна ли сессия сниппета
    int snippetStart;           // Начало сниппета в документе
    int snippetEnd;             // Конец сниппета в документе
    QVector<SnippetPlaceholder> placeholders;
    int currentPlaceholderIndex; // Текущий активный placeholder
    QString snippetId;          // Идентификатор сниппета
    QTextCursor originalCursor; // Курсор до вставки сниппета
    
    SnippetSession() : isActive(false), snippetStart(0), snippetEnd(0),
                       currentPlaceholderIndex(-1) {}
};

struct SnippetDefinition {
    QString trigger;            // Триггер для вставки (например, "for")
    QString name;               // Название сниппета
    QString description;        // Описание
    QString content;            // Содержимое с placeholder'ами
    QString language;           // Язык/контекст
    QStringList scopes;         // Области применения
    int relevance;              // Приоритет
    
    SnippetDefinition() : relevance(100) {}
};

class SnippetManager : public QObject {
    Q_OBJECT
    
public:
    static SnippetManager& getInstance();
    
    // Initialization
    void initialize();
    void loadSnippets(const QString& filePath);
    void saveSnippets(const QString& filePath);
    
    // Snippet management
    void registerSnippet(const SnippetDefinition& snippet);
    void unregisterSnippet(const QString& trigger);
    SnippetDefinition getSnippet(const QString& trigger) const;
    QVector<SnippetDefinition> getAllSnippets() const;
    QVector<SnippetDefinition> getSnippetsByScope(const QString& scope) const;
    
    // Session management
    void startSnippetSession(QTextDocument* document, const QString& content, int insertPosition);
    void endSnippetSession();
    bool isSessionActive() const { return session.isActive; }
    SnippetSession getCurrentSession() const { return session; }
    
    // Navigation
    void nextPlaceholder();
    void previousPlaceholder();
    void goToPlaceholder(int index);
    void selectCurrentPlaceholder();
    
    // Placeholder operations
    void updatePlaceholderText(int placeholderId, const QString& text);
    void synchronizeLinkedPlaceholders(int placeholderId);
    QString getPlaceholderText(int placeholderId) const;
    
    // Document integration
    void setDocument(QTextDocument* document);
    QTextDocument* getDocument() const { return document; }
    
    // Visual
    void updatePlaceholderHighlights();
    QRect getPlaceholderRect(int placeholderIndex) const;
    bool isCursorInPlaceholder(const QTextCursor& cursor) const;
    
    // Configuration
    void setTabNavigation(bool enable);
    bool getTabNavigation() const { return tabNavigationEnabled; }
    void setPlaceholderColor(const QColor& color);
    void setActivePlaceholderColor(const QColor& color);
    QColor getPlaceholderColor() const { return placeholderColor; }
    QColor getActivePlaceholderColor() const { return activePlaceholderColor; }
    
    // Parsing
    static QString parsePlaceholderSyntax(const QString& content);
    static QVector<SnippetPlaceholder> extractPlaceholders(const QString& content, int basePosition);
    static QString expandSnippet(const QString& content, const QMap<int, QString>& values);
    
signals:
    void sessionStarted(const SnippetSession& session);
    void sessionEnded();
    void placeholderChanged(int placeholderId, const QString& text);
    void placeholderNavigated(int currentIndex, int total);
    void snippetCompleted();
    
private slots:
    void onDocumentContentsChanged(int position, int charsRemoved, int charsAdded);
    void onCursorPositionChanged();
    
private:
    SnippetManager();
    ~SnippetManager();
    SnippetManager(const SnippetManager&) = delete;
    SnippetManager& operator=(const SnippetManager&) = delete;
    
    // Internal helpers
    void parseAndInsertSnippet(const QString& content, int position);
    void highlightPlaceholder(int placeholderIndex);
    void clearPlaceholderHighlights();
    void updateSessionBoundaries();
    bool isPlaceholderAtPosition(int position) const;
    int findPlaceholderAtPosition(int position) const;
    void commitPlaceholderChanges();
    
    // Placeholder syntax parsing
    QString parsePlaceholder(const QString& text, int& pos, SnippetPlaceholder& placeholder);
    QString parseVariable(const QString& text, int& pos);
    QString parseChoice(const QString& text, int& pos);
    
    QTextDocument* document;
    SnippetSession session;
    QVector<SnippetDefinition> snippets;
    QMap<QString, SnippetDefinition> snippetsByTrigger;
    
    // Visual formatting
    QTextCharFormat placeholderFormat;
    QTextCharFormat activePlaceholderFormat;
    QColor placeholderColor;
    QColor activePlaceholderColor;
    
    // Configuration
    bool tabNavigationEnabled;
    bool autoCloseBrackets;
    int placeholderTimeout;
    
    // State tracking
    bool isUpdating;
    int lastPlaceholderCount;
    QTimer* sessionTimeoutTimer;
    
    // Linked placeholders (same id should update together)
    QMap<int, QVector<int>> linkedPlaceholders;
};

} // namespace proxima

#endif // CENTAURI_SNIPPETMANAGER_H