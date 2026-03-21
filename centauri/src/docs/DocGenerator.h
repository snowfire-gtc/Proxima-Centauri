#ifndef PROXIMA_DOC_GENERATOR_H
#define PROXIMA_DOC_GENERATOR_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
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
    QString type; // function, class, variable, etc.
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
    QDateTime generatedAt;
    QString generatorVersion;
};

class DocGenerator : public QObject {
    Q_OBJECT
    
public:
    explicit DocGenerator(QObject *parent = nullptr);
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
    QString getOutputPath() const { return outputDir; }
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
    void parseSourceFile(const QString& path, const QString& content);
    void parseComments(const QString& content, DocModule& module);
    void parseFunction(const QString& signature, const QString& comment, DocItem& item);
    void parseClass(const QString& signature, const QString& comment, DocItem& item);
    void parseVariable(const QString& signature, const QString& comment, DocItem& item);
    void extractTag(const QString& comment, const QString& tag, QString& value);
    void extractTags(const QString& comment, const QString& tag, QStringList& values);
    void extractOptions(const QString& comment, QMap<QString, QString>& options);
    
    bool writeHTML(const QString& path);
    bool writeMarkdown(const QString& path);
    bool writeLaTeX(const QString& path);
    bool writeXML(const QString& path);
    
    QString renderHTML(const DocItem& item);
    QString renderMarkdown(const DocItem& item);
    QString renderLaTeX(const DocItem& item);
    QString processLatexFormulas(const QString& text);
    QString generateTableOfContents();
    QString generateIndex();
    QString generateSearchIndex();
    QString escapeHTML(const QString& text) const;
    QString escapeMarkdown(const QString& text) const;
    QString escapeLaTeX(const QString& text) const;
    
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