#ifndef CENTAURI_DOCGENERATOR_H
#define CENTAURI_DOCGENERATOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QDateTime>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QTextEdit>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QIcon>
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
#include <QPlainTextEdit>
#include <QRichTextItemDelegate>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeView>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QToolBox>
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QShortcut>
#include <QKeySequence>
#include <QGesture>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QAbstractAnimation>
#include <QAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QStyleOptionViewItem>
#include <QStyleOptionHeader>
#include <QStyleOptionToolBar>
#include <QStyleOptionMenuItem>
#include <QStyleOptionComplex>
#include <QStyleOptionSpinBox>
#include <QStyleOptionComboBox>
#include <QStyleOptionSlider>
#include <QStyleOptionProgressBar>
#include <QStyleOptionButton>
#include <QStyleOptionTab>
#include <QStyleOptionFrame>
#include <QStyleOptionGroupBox>
#include <QStyleOptionDockWidget>
#include <QStyleOptionFocusRect>
#include <QStyleOptionToolBox>
#include <QStyleOptionRubberBand>
#include <QStyleOptionGraphicsItem>
#include <QStyleOptionTitleBar>
#include <QStyleOptionToolButton>
#include <QStyleOptionHeaderV2>
#include <QStyleOptionSizeGrip>
#include <QStyleOptionSplitter>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QIcon>
#include <QPixmap>
#include <QBitmap>
#include <QMaskedPixmap>
#include <QSvgRenderer>
#include <QSvgGenerator>
#include <QMovie>
#include <QScroller>
#include <QScrollerProperties>
#include <QGesture>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <QTapGesture>
#include <QTapAndHoldGesture>
#include <QAbstractScrollArea>
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
#include <QProgressDialog>
#include <QWizard>
#include <QWizardPage>
#include <QCompleter>
#include <QValidator>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegExpValidator>
#include <QRegularExpressionValidator>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextListFormat>
#include <QTextTableFormat>
#include <QTextFrameFormat>
#include <QTextImageFormat>
#include <QTextObject>
#include <QTextObjectInterface>
#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>
#include <QPdfWriter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPageSetupDialog>
#include <QFontInfo>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QStyle>
#include <QStyleFactory>
#include <QStylePainter>
#include <QStyleOption>
#include <QVariantAnimation>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>
#include "utils/CollectionParser.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Форматы выходной документации
 */
enum class DocFormat {
    HTML,       // HTML документация
    Markdown,   // Markdown документация
    PDF,        // PDF документация (через LaTeX)
    LaTeX,      // LaTeX исходники
    XML,        // XML для машинной обработки
    CHM,        // Compiled HTML Help
    ePub        // Электронная книга
};

/**
 * @brief Тип элемента документации
 */
enum class DocItemType {
    Unknown,
    Module,
    Class,
    Interface,
    Function,
    Method,
    Variable,
    Constant,
    Enum,
    Typedef,
    Namespace,
    Template,
    Macro,
    Example,
    Tutorial,
    FAQ
};

/**
 * @brief Структура элемента документации
 */
struct DocItem {
    QString name;                   // Имя элемента
    QString type;                   // Тип элемента (function, class, etc.)
    QString description;            // Описание
    QString file;                   // Файл исходного кода
    int line;                       // Номер строки
    QStringList parameters;         // Параметры
    QString returnType;             // Тип возврата
    QStringList examples;           // Примеры использования
    QStringList seeAlso;            // См. также
    QMap<QString, QString> options; // Дополнительные опции
    QString namespace_;             // Пространство имён
    bool isPublic;                  // Публичный доступ
    bool isDeprecated;              // Устаревший
    QString deprecatedMessage;      // Сообщение об устаревании
    QStringList inherits;           // Наследование
    QStringList inheritedBy;        // Наследуется кем
    QStringList relatedClasses;     // Связанные классы
    QString templateParams;         // Параметры шаблона
    QString since;                  // Начиная с версии
    QString author;                 // Автор
    QString version;                // Версия
    QString copyright;              // Copyright
    QString license;                // Лицензия
    QStringList keywords;           // Ключевые слова
    QStringList categories;         // Категории
    QString group;                  // Группа документации
    int complexity;                 // Сложность (1-5)
    QString difficulty;             // Уровень сложности
    QStringList prerequisites;      // Требования
    QStringList learningOutcomes;   // Результаты обучения
    QString estimatedTime;          // Примерное время изучения
    QStringList tags;               // Теги
    QString summary;                // Краткое описание
    QString detailedDescription;    // Подробное описание
    QStringList notes;              // Заметки
    QStringList warnings;           // Предупреждения
    QStringList todos;              // TODO
    QString platform;               // Платформа
    QString architecture;           // Архитектура
    QString language;               // Язык документации
    QStringList translations;       // Переводы
    QString lastModified;           // Последнее изменение
    QString created;                // Дата создания
    QStringList contributors;       // Участники
    QString status;                 // Статус (draft, review, published)
    int priority;                   // Приоритет
    QString reviewStatus;           // Статус проверки
    QString reviewDate;             // Дата проверки
    QString reviewer;               // Рецензент
    
    DocItem() : line(0), isPublic(true), isDeprecated(false), 
                complexity(1), priority(1) {}
};

/**
 * @brief Структура модуля документации
 */
struct DocModule {
    QString name;                   // Имя модуля
    QString path;                   // Путь к файлу
    QString description;            // Описание
    QVector<DocItem> items;         // Элементы документации
    QStringList dependencies;       // Зависимости
    QString author;                 // Автор
    QString version;                // Версия
    QString license;                // Лицензия
    QString copyright;              // Copyright
    QString created;                // Дата создания
    QString modified;               // Дата изменения
    QStringList keywords;           // Ключевые слова
    QString group;                  // Группа
    QString status;                 // Статус
    QString notes;                  // Заметки
    
    DocModule() {}
};

/**
 * @brief Структура проекта документации
 */
struct DocProject {
    QString name;                   // Имя проекта
    QString version;                // Версия
    QString description;            // Описание
    QVector<DocModule> modules;     // Модули
    QVector<DocItem> items;         // Все элементы
    QDateTime generatedAt;          // Дата генерации
    QString generatorVersion;       // Версия генератора
    QString outputFormat;           // Формат вывода
    QString outputDir;              // Директория вывода
    QString templatePath;           // Путь к шаблону
    QString language;               // Язык документации
    QString title;                  // Заголовок
    QString subtitle;               // Подзаголовок
    QString author;                 // Автор
    QString copyright;              // Copyright
    QString license;                // Лицензия
    QString logo;                   // Логотип
    QString favicon;                // Favicon
    QString stylesheet;             // Таблица стилей
    QString header;                 // Заголовок страницы
    QString footer;                 // Подвал страницы
    bool includePrivate;            // Включать приватные элементы
    bool includeExamples;           // Включать примеры
    bool includeInherited;          // Включать унаследованные
    bool generateIndex;             // Генерировать индекс
    bool generateTOC;               // Генерировать содержание
    bool generateSearch;            // Генерировать поиск
    bool generateGraphs;            // Генерировать графы
    bool generateTimeline;          // Генерировать временную шкалу
    bool generateStatistics;        // Генерировать статистику
    bool validateLinks;             // Проверять ссылки
    bool minimizeOutput;            // Минимизировать вывод
    bool compressOutput;            // Сжимать вывод
    bool encryptOutput;             // Шифровать вывод
    QString encryptionKey;          // Ключ шифрования
    QStringList excludePatterns;    // Шаблоны исключения
    QStringList includePatterns;    // Шаблоны включения
    QMap<QString, QString> metadata; // Метаданные
    QStringList customTemplates;    // Пользовательские шаблоны
    QStringList plugins;            // Плагины
    QMap<QString, QVariant> settings; // Настройки
    
    DocProject() : generatedAt(QDateTime::currentDateTime()),
                   generatorVersion("1.0.0"), includePrivate(false),
                   includeExamples(true), includeInherited(true),
                   generateIndex(true), generateTOC(true),
                   generateSearch(true), generateGraphs(false),
                   generateTimeline(false), generateStatistics(true),
                   validateLinks(true), minimizeOutput(false),
                   compressOutput(false), encryptOutput(false) {}
};

/**
 * @brief Класс генератора документации для IDE Centauri
 * 
 * Согласно требованиям:
 * - language.txt пункт 50 - генерация документации из комментариев
 * - language.txt пункт 52 - команда proxima doc
 * - ide.txt пункт 20ж - "Помощь по проекту"
 */
class DocGenerator : public QObject {
    Q_OBJECT
    
public:
    explicit DocGenerator(QObject *parent = nullptr);
    ~DocGenerator();
    
    // ========================================================================
    // Конфигурация
    // ========================================================================
    
    void setProjectName(const QString& name);
    void setProjectVersion(const QString& version);
    void setOutputDir(const QString& dir);
    void setFormat(DocFormat format);
    void setIncludePrivate(bool include);
    void setIncludeExamples(bool include);
    void setTemplate(const QString& templatePath);
    void setLanguage(const QString& language);
    void setTitle(const QString& title);
    void setAuthor(const QString& author);
    void setCopyright(const QString& copyright);
    void setLicense(const QString& license);
    
    // ========================================================================
    // Добавление исходных файлов
    // ========================================================================
    
    void addSourceFile(const QString& path, const QString& content);
    void addSourceDirectory(const QString& path);
    void addSymbolTable(const SymbolTable& symbols);
    void addAST(ProgramNodePtr ast, const QString& filename);
    
    // ========================================================================
    // Генерация документации
    // ========================================================================
    
    bool generate();
    bool generateHTML();
    bool generateMarkdown();
    bool generatePDF();
    bool generateLaTeX();
    bool generateXML();
    bool generateCHM();
    bool generateEPub();
    
    // ========================================================================
    // Вывод
    // ========================================================================
    
    QString getOutputDir() const { return outputDir; }
    DocFormat getFormat() const { return format; }
    DocProject getProject() const { return project; }
    
    // ========================================================================
    // Статистика
    // ========================================================================
    
    int getModuleCount() const { return project.modules.size(); }
    int getItemCount() const;
    int getFunctionCount() const;
    int getClassCount() const;
    int getVariableCount() const;
    int getExampleCount() const;
    int getPageCount() const;
    int getWordCount() const;
    int getCharacterCount() const;
    qint64 getGenerationTime() const { return generationTime; }
    
    // ========================================================================
    // Поиск и фильтрация
    // ========================================================================
    
    QVector<DocItem> search(const QString& query) const;
    DocItem getItemByName(const QString& name) const;
    QVector<DocItem> getItemsByType(const QString& type) const;
    QVector<DocItem> getItemsByNamespace(const QString& ns) const;
    QVector<DocItem> getItemsByGroup(const QString& group) const;
    QVector<DocItem> getDeprecatedItems() const;
    QVector<DocItem> getPublicItems() const;
    QVector<DocItem> getPrivateItems() const;
    
    // ========================================================================
    // Экспорт/Импорт
    // ========================================================================
    
    bool exportToHTML(const QString& path);
    bool exportToMarkdown(const QString& path);
    bool exportToPDF(const QString& path);
    bool exportToXML(const QString& path);
    bool exportToJSON(const QString& path);
    bool exportToCHM(const QString& path);
    bool exportToEPub(const QString& path);
    
    bool importFromXML(const QString& path);
    bool importFromJSON(const QString& path);
    
    // ========================================================================
    // Валидация
    // ========================================================================
    
    bool validateDocumentation();
    QStringList getValidationErrors() const { return validationErrors; }
    QStringList getValidationWarnings() const { return validationWarnings; }
    
    // ========================================================================
    // Утилиты
    // ========================================================================
    
    static QString escapeHTML(const QString& text);
    static QString escapeXML(const QString& text);
    static QString escapeLaTeX(const QString& text);
    static QString escapeMarkdown(const QString& text);
    static QString processLatexFormulas(const QString& text);
    static QString highlightCode(const QString& code, const QString& language);
    static QString generateTOC(const QVector<DocItem>& items);
    static QString generateIndex(const QVector<DocItem>& items);
    static QString generateSearchIndex(const QVector<DocItem>& items);
    
signals:
    void generationStarted();
    void generationProgress(int current, int total, const QString& message);
    void generationCompleted(bool success);
    void errorOccurred(const QString& error);
    void warningOccurred(const QString& warning);
    void itemAdded(const DocItem& item);
    void moduleAdded(const DocModule& module);
    void fileProcessed(const QString& file);
    
private:
    // ========================================================================
    // Парсинг
    // ========================================================================
    
    void parseComments(const QString& content, DocModule& module);
    void parseDocComment(const QString& comment, DocItem& item);
    void extractTag(const QString& comment, const QString& tag, QString& value);
    void extractTags(const QString& comment, const QString& tag, QStringList& values);
    void extractOptions(const QString& comment, QMap<QString, QString>& options);
    void extractDependencies(const QString& content, DocModule& module);
    void traverseAST(ProgramNodePtr ast, const QString& filename);
    void processDeclaration(DeclarationNodePtr decl, const QString& filename);
    void processStatement(StatementNodePtr stmt, const QString& filename);
    void extractDocFromComments(DeclarationNodePtr decl, DocItem& item);
    QString extractNamespace(const QString& fullName);
    QString symbolTypeToString(SymbolKind kind);
    
    // ========================================================================
    // Рендеринг
    // ========================================================================
    
    QString renderHTML(const DocItem& item);
    QString renderMarkdown(const DocItem& item);
    QString renderLaTeX(const DocItem& item);
    QString renderXML(const DocItem& item);
    
    // ========================================================================
    // Генерация страниц
    // ========================================================================
    
    QString generateCSS();
    QString generateJavaScript();
    QString generateTableOfContents();
    QString generateIndex();
    QString generateSearchIndex();
    QString generateHeader();
    QString generateFooter();
    QString generateNavigation();
    QString generateSidebar();
    void generateModulePages();
    void generateAPIPages();
    void generateExamplePages();
    void generateTutorialPages();
    void generateFAQPages();
    
    // ========================================================================
    // Вспомогательные функции
    // ========================================================================
    
    void log(int level, const QString& message);
    QString formatTimestamp(const QDateTime& dt) const;
    QString formatFileSize(qint64 size) const;
    QString formatDuration(qint64 ms) const;
    QColor getTypeColor(const QString& type) const;
    QIcon getTypeIcon(const QString& type) const;
    QString getTemplateContent(const QString& templateName) const;
    bool saveToFile(const QString& path, const QString& content);
    QString loadFromFile(const QString& path) const;
    bool createDirectory(const QString& path);
    bool copyFile(const QString& source, const QString& dest);
    bool copyDirectory(const QString& source, const QString& dest);
    void cleanOutputDirectory();
    
    // ========================================================================
    // Члены класса
    // ========================================================================
    
    DocProject project;
    QString outputDir;
    DocFormat format;
    bool includePrivate;
    bool includeExamples;
    QString templatePath;
    QString language;
    
    SymbolTable* symbolTable;
    QMap<QString, ProgramNodePtr> asts;
    
    int itemsProcessed;
    int totalItems;
    qint64 generationTime;
    
    QStringList validationErrors;
    QStringList validationWarnings;
    
    QNetworkAccessManager* networkManager;
    QProgressDialog* progressDialog;
};

} // namespace proxima

#endif // CENTAURI_DOCGENERATOR_H