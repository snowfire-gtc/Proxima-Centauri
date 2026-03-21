#ifndef CENTAURI_CODEFOLDINGMANAGER_H
#define CENTAURI_CODEFOLDINGMANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>
#include <QTextDocument>
#include <QTextEdit>

namespace proxima {

enum class FoldType {
    Method,
    Class,
    Interface,
    Namespace,
    Region,
    BlockComment,
    Loop,
    Conditional,
    Custom
};

struct FoldRegion {
    int id;
    FoldType type;
    int startLine;
    int endLine;
    int foldLine;          // Line where fold control is shown
    QString name;          // Name of the region/method/class
    QString preview;       // Preview text when folded
    bool isFolded;
    int nestingLevel;
    QVector<int> childRegions;
    int parentRegion;
    
    FoldRegion() : id(0), type(FoldType::Custom), startLine(0), endLine(0),
                   foldLine(0), isFolded(false), nestingLevel(0), parentRegion(-1) {}
};

struct FoldConfig {
    bool enableFolding;
    bool autoFoldRegions;
    bool autoFoldMethods;
    bool autoFoldClasses;
    bool autoFoldComments;
    bool preserveFoldState;
    int maxPreviewLength;
    QString foldMarkerColor;
    QString foldMarkerHoverColor;
    
    FoldConfig() : enableFolding(true), autoFoldRegions(true),
                   autoFoldMethods(false), autoFoldClasses(false),
                   autoFoldComments(false), preserveFoldState(true),
                   maxPreviewLength(50), foldMarkerColor("#666666"),
                   foldMarkerHoverColor("#AAAAAA") {}
};

class CodeFoldingManager : public QObject {
    Q_OBJECT
    
public:
    static CodeFoldingManager& getInstance();
    
    // Initialization
    void initialize(QTextDocument* document);
    void setConfig(const FoldConfig& config);
    FoldConfig getConfig() const { return config; }
    
    // Fold region management
    void analyzeDocument();
    void addFoldRegion(const FoldRegion& region);
    void removeFoldRegion(int id);
    void clearFoldRegions();
    
    // Fold operations
    void toggleFold(int line);
    void foldRegion(int id);
    void unfoldRegion(int id);
    void foldAll();
    void unfoldAll();
    void foldLevel(int level);
    void foldMethods();
    void foldClasses();
    void foldRegions();
    void foldComments();
    
    // Query
    bool isLineFolded(int line) const;
    bool isRegionFolded(int id) const;
    FoldRegion getFoldRegion(int id) const;
    QVector<FoldRegion> getFoldRegions() const { return foldRegions; }
    QVector<FoldRegion> getFoldRegionsAtLine(int line) const;
    int getFoldedLineCount() const;
    
    // Visual
    void updateFoldMarkers();
    QRect getFoldMarkerRect(int line) const;
    bool isMouseOverFoldMarker(const QPoint& pos) const;
    int getLineFromMousePos(const QPoint& pos) const;
    
    // Persistence
    void saveFoldState(const QString& filePath);
    void loadFoldState(const QString& filePath);
    void clearFoldState(const QString& filePath);
    
    // Integration
    void connectToEditor(QTextEdit* editor);
    void disconnectFromEditor();
    
signals:
    void foldStateChanged(int regionId, bool isFolded);
    void documentAnalyzed(int regionCount);
    void foldMarkerClicked(int line);
    void visibilityChanged(int startLine, int endLine, bool visible);
    
private:
    CodeFoldingManager();
    ~CodeFoldingManager();
    CodeFoldingManager(const CodeFoldingManager&) = delete;
    CodeFoldingManager& operator=(const CodeFoldingManager&) = delete;
    
    // Analysis
    void analyzeMethods();
    void analyzeClasses();
    void analyzeInterfaces();
    void analyzeNamespaces();
    void analyzeRegions();
    void analyzeBlockComments();
    void analyzeLoops();
    void analyzeConditionals();
    void buildFoldHierarchy();
    void calculateNestingLevels();
    
    // Pattern matching
    bool isMethodStart(const QString& line, int& startLine, int& endLine, QString& name);
    bool isClassStart(const QString& line, int& startLine, int& endLine, QString& name);
    bool isRegionStart(const QString& line, int& startLine, int& endLine, QString& name);
    bool isBlockCommentStart(const QString& line, int& startLine, int& endLine, QString& name);
    bool isLoopStart(const QString& line, int& startLine, int& endLine);
    bool isConditionalStart(const QString& line, int& startLine, int& endLine);
    int findMatchingEnd(const QString& line, int startLine, const QString& endPattern);
    
    // Visual helpers
    QString generatePreview(const QString& text, int maxLength) const;
    QString getFoldIcon(FoldType type) const;
    QColor getFoldColor(FoldType type) const;
    
    // State management
    QMap<QString, QVector<int>> foldStateCache;
    void cacheFoldState(const QString& filePath);
    void restoreFoldState(const QString& filePath);
    
    QTextDocument* document;
    QTextEdit* editor;
    QVector<FoldRegion> foldRegions;
    QMap<int, int> lineToFoldRegion;  // Maps line to fold region id
    QMap<int, bool> foldedLines;       // Lines that are hidden due to folding
    FoldConfig config;
    
    int nextRegionId;
    bool isAnalyzing;
    bool isUpdating;
    
    // Mouse tracking
    int hoverFoldLine;
    bool mouseOverFoldMarker;
};

} // namespace proxima

#endif // CENTAURI_CODEFOLDINGMANAGER_H