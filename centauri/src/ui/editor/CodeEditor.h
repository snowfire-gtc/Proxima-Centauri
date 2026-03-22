#ifndef CENTAURI_CODEEDITOR_H
#define CENTAURI_CODEEDITOR_H

#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QVector>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QListWidget>
#include <QScrollBar>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextList>
#include <QTextListFormat>
#include <QTextFrame>
#include <QTextFrameFormat>
#include <QAbstractTextDocumentLayout>
#include <QPlainTextDocumentLayout>
#include <QStyle>
#include <QStyleOption>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QVariant>
#include <QVariantAnimation>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>
#include <QGesture>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QDial>
#include <QProgressBar>
#include <QProgressDialog>
#include <QErrorMessage>
#include <QInputDialog>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QWizard>
#include <QWizardPage>
#include <QCompleter>
#include <QValidator>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegExpValidator>
#include <QRegularExpressionValidator>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QTabletEvent>
#include <QTouchEvent>
#include <QInputMethodEvent>
#include <QActionEvent>
#include <QFileOpenEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QEnterEvent>
#include <QLeaveEvent>
#include <QWinEvent>
#include <QNativeGestureEvent>
#include <QScrollPrepareEvent>
#include <QScrollEvent>
#include <QWindowStateChangeEvent>
#include <QToolBar>
#include <QToolButton>
#include <QToolBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QShortcut>
#include <QKeySequence>
#include "editor/SyntaxHighlighter.h"
#include "editor/CodeFoldingManager.h"
#include "runtime/Runtime.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Информация об авторе правки
 */
struct AuthorInfo {
    QString name;           // Имя автора
    QString email;          // Email автора
    QColor color;           // Уникальный цвет автора
    int registrationOrder;  // Порядок регистрации (для генерации цвета)

    AuthorInfo() : registrationOrder(0) {}
    AuthorInfo(const QString& n, const QString& e, const QColor& c, int order)
        : name(n), email(e), color(c), registrationOrder(order) {}
};

/**
 * @brief Информация о правке строки
 */
struct LineEditInfo {
    QString author;         // Имя автора
    QDateTime editTime;     // Время правки
    QString commitHash;     // Хэш коммита (если из Git)
    QString commitMessage;  // Сообщение коммита

    LineEditInfo() {}
    LineEditInfo(const QString& a, const QDateTime& t, const QString& h = "", const QString& m = "")
        : author(a), editTime(t), commitHash(h), commitMessage(m) {}
};

/**
 * @brief Режимы отображения редактора
 *
 * Согласно требованию ide.txt пункт 11е:
 * - стандартный с подсветкой синтаксиса
 * - подсветка типов переменных
 * - подсветка размерности переменных
 * - подсветка времени выполнения
 * - подсветка частоты выполнения
 * - отображение изменений vs репозиторий
 * - автор правки (НОВОЕ)
 * - старость правки (НОВОЕ)
 */
enum class DisplayMode {
    Standard,           // Стандартный с подсветкой синтаксиса
    TypeHighlight,      // Подсветка типов переменных
    DimensionHighlight, // Подсветка размерности переменных
    TimingHighlight,    // Подсветка времени выполнения
    FrequencyHighlight, // Подсветка частоты выполнения
    GitDiff,           // Изменения vs репозиторий
    AuthorHighlight,    // Автор правки (НОВОЕ)
    AgeHighlight        // Старость правки (НОВОЕ)
};

/**
 * @brief Типы брейкпоинтов
 */
enum class BreakpointType {
    Unconditional,
    Conditional,
    Temporary
};

/**
 * @brief Структура точки останова
 */
struct Breakpoint {
    int id;
    int line;
    BreakpointType type;
    QString condition;
    bool enabled;
    int hitCount;
    int maxHits;
    QString lastHitTime;

    Breakpoint() : id(0), line(0), type(BreakpointType::Unconditional),
                   enabled(true), hitCount(0), maxHits(1) {}
};

/**
 * @brief Область сворачивания кода
 */
struct FoldRegion {
    int startLine;
    int endLine;
    int foldLine;
    QString name;
    QString type;  // method, class, region, comment
    bool isFolded;
    int nestingLevel;

    FoldRegion() : startLine(0), endLine(0), foldLine(0),
                   isFolded(false), nestingLevel(0) {}
};

/**
 * @brief Виджет номера строки
 */
class LineNumberArea : public QWidget {
    Q_OBJECT

public:
    explicit LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *editor;
};

/**
 * @brief Класс редактора кода для IDE Centauri
 *
 * Согласно требованиям:
 * - ide.txt пункт 11 - панель инструментов редактора
 * - ide.txt пункт 11е - 8 режимов отображения
 * - ide.txt пункт 12 - строка состояния
 * - ide.txt пункт 31 - сворачивание кода
 * - language.txt пункт 11.1 - индексация с 1
 */
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    // ========================================================================
    // Операции с файлами
    // ========================================================================

    bool loadFile(const QString& path);
    bool saveFile();
    bool saveFileAs(const QString& path);
    QString getFilePath() const { return filePath; }
    bool isModified() const { return modified; }
    void setModified(bool m);

    // ========================================================================
    // Позиция курсора
    // ========================================================================

    int getCurrentLine() const;
    int getCurrentColumn() const;
    int getCurrentPosition() const;
    void goToLine(int line);
    void goToPosition(int position);
    void selectLine(int line);
    void selectRange(int startLine, int startCol, int endLine, int endCol);

    // ========================================================================
    // Операции с кодом
    // ========================================================================

    QString getSelectedCode() const;
    QString getCurrentLineCode() const;
    QString getCodeInRange(int startLine, int endLine) const;
    int getSelectionStart() const;
    int getSelectionEnd() const;
    void formatCode();
    void indentSelection();
    void unindentSelection();
    void commentSelection();
    void uncommentSelection();
    void duplicateLine();
    void deleteLine();
    void moveLineUp();
    void moveLineDown();
    void insertSnippet(const QString& snippet);
    void surroundWith(const QString& before, const QString& after);

    // ========================================================================
    // Точки останова
    // ========================================================================

    void toggleBreakpoint(int line);
    bool hasBreakpoint(int line) const;
    void clearBreakpoints();
    QVector<int> getBreakpoints() const { return breakpoints; }
    void setBreakpoints(const QVector<int>& bps);
    void setBreakpointEnabled(int line, bool enabled);
    bool isBreakpointEnabled(int line) const;

    // ========================================================================
    // Режимы отображения
    // ========================================================================

    void setDisplayMode(DisplayMode mode);
    DisplayMode getDisplayMode() const { return displayMode; }

    // Применение данных для подсветки
    void applyTypeHighlighting(const QMap<int, QString>& lineTypes);
    void applyDimensionHighlighting(const QMap<int, QString>& lineDimensions);
    void applyTimingHighlighting(const QMap<int, double>& lineTimings);
    void applyFrequencyHighlighting(const QMap<int, int>& lineFrequencies);
    void applyGitDiff(const QString& diff);
    void applyAuthorHighlighting(const QMap<int, QString>& lineAuthors);
    void applyAgeHighlighting(const QMap<int, QDateTime>& lineAges);

    // ========================================================================
    // Информация об авторах и правках (НОВОЕ)
    // ========================================================================

   /**
     * @brief Отмена последней правки
     * @return true если успешно
     */
    bool undoLastEdit();

    /**
     * @brief Возврат правки
     * @return true если успешно
     */
    bool redoLastEdit();

    /**
     * @brief Отслеживание изменений текста
     * @param position Позиция изменения
     * @param charsRemoved Количество удалённых символов
     * @param charsAdded Количество добавленных символов
     */
    void trackTextEdit(int position, int charsRemoved, int charsAdded);


    /**
     * @brief Регистрация автора с уникальным цветом
     * @param name Имя автора
     * @param email Email автора
     * @return true если успешно
     */
    bool registerAuthor(const QString& name, const QString& email);

    /**
     * @brief Получение информации об авторе
     * @param name Имя автора
     * @return Информация об авторе
     */
    AuthorInfo getAuthorInfo(const QString& name) const;

    /**
     * @brief Получение всех зарегистрированных авторов
     * @return Список авторов
     */
    QVector<AuthorInfo> getAllAuthors() const;

    /**
     * @brief Установка информации о правке строки
     * @param line Номер строки (1-based)
     * @param info Информация о правке
     */
    void setLineEditInfo(int line, const LineEditInfo& info);

    /**
     * @brief Получение информации о правке строки
     * @param line Номер строки (1-based)
     * @return Информация о правке
     */
    LineEditInfo getLineEditInfo(int line) const;

    /**
     * @brief Установка информации о правках для всех строк
     * @param lineInfoMap Карта: номер строки -> информация о правке
     */
    void setAllLineEditInfo(const QMap<int, LineEditInfo>& lineInfoMap);

    /**
     * @brief Получение всей информации о правках
     * @return Карта информации о правках
     */
    QMap<int, LineEditInfo> getAllLineEditInfo() const;

    /**
     * @brief Очистка информации о правках
     */
    void clearLineEditInfo();

    /**
     * @brief Генерация уникального цвета для автора
     * @param registrationOrder Порядок регистрации
     * @return Уникальный цвет
     */
    static QColor generateUniqueAuthorColor(int registrationOrder);

    /**
     * @brief Получение цвета для возраста правки
     * @param ageMinutes Возраст в минутах
     * @return Цвет
     */
    static QColor getAgeColor(qint64 ageMinutes);

    /**
     * @brief Получение строкового представления возраста
     * @param ageMinutes Возраст в минутах
     * @return Строка с возрастом
     */
    static QString getAgeString(qint64 ageMinutes);

    // ========================================================================
    // Сворачивание кода
    // ========================================================================

    void toggleFold(int line);
    bool isFolded(int line) const;
    void foldAll();
    void unfoldAll();
    void foldLevel(int level);
    void foldMethods();
    void foldClasses();
    void foldRegions();
    void foldComments();
    QVector<FoldRegion> getFoldRegions() const { return foldRegions; }

    // ========================================================================
    // Выполнение и отладка
    // ========================================================================

    void setCurrentExecutionLine(int line);
    void clearExecutionLine();
    int getCurrentExecutionLine() const { return currentExecutionLine; }
    void markErrorLine(int line, const QString& message);
    void clearErrorLines();
    void markWarningLine(int line, const QString& message);
    void clearWarningLines();

    // ========================================================================
    // Визуализация
    // ========================================================================

    void showArrayVisualization(const QString& varName, const RuntimeValue& value);
    void showMatrixVisualization(const QString& varName, const RuntimeValue& value);
    void showLayerVisualization(const QString& varName, const RuntimeValue& value);
    void showCollectionVisualization(const QString& varName, const RuntimeValue& value);
    void showObjectVisualization(const QString& varName, const RuntimeValue& value);

    // ========================================================================
    // Автодополнение
    // ========================================================================

    void setAutoCompletionEnabled(bool enable);
    bool isAutoCompletionEnabled() const { return autoCompletionEnabled; }
    void triggerAutoCompletion();
    void hideCompletionPopup();
    void setCompletionItems(const QStringList& items);

    // ========================================================================
    // Инструменты
    // ========================================================================

    void setShowLineNumbers(bool show);
    bool getShowLineNumbers() const { return showLineNumbers; }
    void setShowWhitespace(bool show);
    bool getShowWhitespace() const { return showWhitespace; }
    void setShowIndentGuides(bool show);
    bool getShowIndentGuides() const { return showIndentGuides; }
    void setHighlightCurrentLine(bool highlight);
    bool getHighlightCurrentLine() const { return highlightCurrentLine; }
    void setWordWrap(bool wrap);
    bool getWordWrap() const { return wordWrap; }

    // ========================================================================
    // Статистика
    // ========================================================================

    int getLineCount() const { return blockCount(); }
    int getCharacterCount() const { return toPlainText().length(); }
    int getWordCount() const;
    QString getMimeType() const { return "text/x-proxima"; }

signals:
    void cursorPositionChanged(int line, int column);
    void fileModified(bool modified);
    void breakpointToggled(int line, bool enabled);
    void foldToggled(int line, bool folded);
    void executionLineChanged(int line);
    void codeFormatted();
    void snippetInserted(const QString& snippet);
    void completionRequested();
    void visualizationRequested(const QString& varName);
    void authorRegistered(const QString& name, const QColor& color);
    void lineEditInfoChanged(int line, const LineEditInfo& info);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void onCompletionItemActivated(const QString& item);
    void onFoldTimerTimeout();
    void onTextChanged(int position, int charsRemoved, int charsAdded);

private:
    // ========================================================================
    // Методы отрисовки
    // ========================================================================

    void setupEditor();
    void setupConnections();
    void setupShortcuts();
    void setupContextMenu();
    void updateLineNumbers();
    void drawBreakpoints(QPainter& painter);
    void drawExecutionLine(QPainter& painter);
    void drawFoldMarkers(QPainter& painter);
    void drawErrorMarkers(QPainter& painter);
    void drawWarningMarkers(QPainter& painter);
    void drawIndentGuides(QPainter& painter);
    void drawWhitespace(QPainter& painter);
    void drawCurrentLineHighlight(QPainter& painter);
    QString lineNumberAreaColor() const;
    QColor getBreakpointColor() const;
    QColor getExecutionLineColor() const;
    QColor getFoldMarkerColor() const;

    // ========================================================================
    // Методы сворачивания
    // ========================================================================

    void analyzeFoldRegions();
    void updateFoldRegions();
    void setLinesVisible(int start, int end, bool visible);
    bool isLineVisible(int line) const;
    int getFirstVisibleLine() const;
    int getLastVisibleLine() const;

    // ========================================================================
    // Методы автодополнения
    // ========================================================================

    void showCompletionPopup();
    void updateCompletionPopup();
    QString getWordUnderCursor() const;
    QStringList getCompletionSuggestions(const QString& prefix) const;
    void insertCompletion(const QString& completion);

    // ========================================================================
    // Методы визуализации
    // ========================================================================

    RuntimeValue parseArrayValue(const QString& code) const;
    RuntimeValue parseMatrixValue(const QString& code) const;
    RuntimeValue parseLayerValue(const QString& code) const;

    // ========================================================================
    // Методы авторов и правок (НОВОЕ)
    // ========================================================================

    int findAvailableColorSlot() const;
    bool isColorUsed(const QColor& color) const;
    void updateAuthorColors();

    // ========================================================================
    // Переменные
    // ========================================================================

    LineNumberArea *lineNumberArea;
    SyntaxHighlighter *highlighter;
    CodeFoldingManager *foldingManager;

    QString filePath;
    bool modified;
    DisplayMode displayMode;

    // Точки останова
    QVector<int> breakpoints;
    QMap<int, bool> breakpointEnabled;
    QMap<int, QString> breakpointConditions;

    // Сворачивание
    QVector<FoldRegion> foldRegions;
    QMap<int, bool> foldedLines;
    QTimer *foldTimer;

    // Выполнение
    int currentExecutionLine;
    QVector<int> errorLines;
    QVector<int> warningLines;
    QMap<int, QString> errorMessages;
    QMap<int, QString> warningMessages;

    // Подсветка режимов
    QMap<int, QString> typeHighlights;
    QMap<int, QString> dimensionHighlights;
    QMap<int, double> timingHighlights;
    QMap<int, int> frequencyHighlights;
    QMap<int, QString> authorHighlights;
    QMap<int, QDateTime> ageHighlights;
    QString gitDiffData;

    // Авторы и правки (НОВОЕ)
    QMap<QString, AuthorInfo> authors;          // Имя -> Информация об авторе
    QMap<int, LineEditInfo> lineEditInfo;       // Номер строки -> Информация о правке
    int nextAuthorOrder;                        // Следующий порядок регистрации

    // Автодополнение
    bool autoCompletionEnabled;
    QListWidget *completionPopup;
    QStringList completionItems;

    // Настройки отображения
    bool showLineNumbers;
    bool showWhitespace;
    bool showIndentGuides;
    bool highlightCurrentLine;
    bool wordWrap;

    // Статистика
    int wordCount;

    // Контекстное меню
    QMenu *contextMenu;
    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *selectAllAction;
    QAction *commentAction;
    QAction *uncommentAction;
    QAction *indentAction;
    QAction *unindentAction;
    QAction *duplicateLineAction;
    QAction *deleteLineAction;
    QAction *formatCodeAction;
    QAction *toggleBreakpointAction;
    QAction *foldAllAction;
    QAction *unfoldAllAction;
};

} // namespace proxima

#endif // CENTAURI_CODEEDITOR_H
