#ifndef PROXIMA_DOC_GENERATOR_H
#define PROXIMA_DOC_GENERATOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include "parser/AST.h"
#include "semantic/SymbolTable.h"

namespace proxima {

enum class DocFormat {
    HTML,
    PDF,
    Markdown,
    LaTeX,
    XML
};

struct DocItem {
    QString name;
    QString type;  // function, class, variable, etc.
    QString description;
    QString file;
    int line;
    QStringList parameters;
    QString returnType;
    QStringList examples;
    QStringList seeAlso;
    QMap<QString, QString> options;
    QString namespace_;
    bool isPublic;
    bool isDeprecated;
    QString deprecatedMessage;
    
    DocItem() : line(0), isPublic(true), isDeprecated(false) {}
};

struct DocModule {
    QString name;
    QString path;
    QString description;
    QVector<DocItem> items;
    QStringList dependencies;
};

struct DocProject {
    QString name;
    QString version;
    QString description;
    QVector<DocModule> modules;
    QVector<DocItem> items;
    QDateTime generatedAt;
    QString generatorVersion;
};

class DocGenerator : public QObject {
    Q_OBJECT
    
public:
    explicit DocGenerator();
    ~DocGenerator();
    
    // Configuration
    void setProjectName(const QString& name);
    void setProjectVersion(const QString& version);
    void setOutputDir(const QString& dir);
    void setFormat(DocFormat format);
    void setIncludePrivate(bool include);
    void setIncludeExamples(bool include);
    void setTemplate(const QString& templatePath);
    
    // Source processing
    void addSourceFile(const QString& path, const QString& content);
    void addSourceDirectory(const QString& path);
    void addSymbolTable(const SymbolTable& symbols);
    void addAST(ProgramNodePtr ast, const QString& filename);
    
    // Generation
    bool generate();
    bool generateHTML();
    bool generateMarkdown();
    bool generatePDF();
    bool generateLaTeX();
    bool generateXML();
    
    // Output
    QString getOutputDir() const { return outputDir; }
    DocFormat getFormat() const { return format; }
    DocProject getProject() const { return project; }
    
    // Statistics
    int getModuleCount() const { return project.modules.size(); }
    int getItemCount() const;
    int getFunctionCount() const;
    int getClassCount() const;
    int getVariableCount() const;
    
    // Search
    QVector<DocItem> search(const QString& query) const;
    DocItem getItemByName(const QString& name) const;
    QVector<DocItem> getItemsByType(const QString& type) const;
    QVector<DocItem> getItemsByNamespace(const QString& ns) const;
    
signals:
    void generationStarted();
    void generationProgress(int current, int total);
    void generationCompleted(bool success);
    void errorOccurred(const QString& error);
    
private:
    void parseComments(const QString& content, DocModule& module);
    void extractDependencies(const QString& content, DocModule& module);
    void traverseAST(ProgramNodePtr ast, const QString& filename);
    void processDeclaration(DeclarationNodePtr decl, const QString& filename);
    void processStatement(StatementNodePtr stmt, const QString& filename);
    void extractDocFromComments(DeclarationNodePtr decl, DocItem& item);
    QString extractNamespace(const QString& fullName);
    QString symbolTypeToString(SymbolKind kind);
    
    QString renderHTML(const DocItem& item);
    QString renderMarkdown(const DocItem& item);
    QString renderLaTeX(const DocItem& item);
    
    QString generateCSS();
    QString generateTableOfContents();
    QString generateIndex();
    QString generateMarkdownIndex();
    QString generateSearchIndex();
    QString processLatexFormulas(const QString& text);
    
    QString escapeHTML(const QString& text) const;
    QString escapeXML(const QString& text) const;
    QString escapeLaTeX(const QString& text) const;
    QString escapeJavaScript(const QString& text) const;
    
    void generateModulePages();
    void generateAPIPages();
    void log(int level, const QString& message);
    
    DocProject project;
    QString outputDir;
    DocFormat format;
    bool includePrivate;
    bool includeExamples;
    QString templatePath;
    
    SymbolTable symbolTable;
    QMap<QString, ProgramNodePtr> asts;
    
    int itemsProcessed;
    int totalItems;
};

} // namespace proxima

#endif // PROXIMA_DOC_GENERATOR_H