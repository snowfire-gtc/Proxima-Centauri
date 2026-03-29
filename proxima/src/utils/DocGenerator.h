#ifndef PROXIMA_DOC_GENERATOR_H
#define PROXIMA_DOC_GENERATOR_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include "parser/AST.h"
#include "semantic/SymbolTable.h"

namespace proxima {

class TypeChecker;

// Формат документации
enum class DocFormat {
    HTML,
    MARKDOWN,
    PDF,
    XML,
    JSON
};

// Элемент документации
struct DocItem {
    std::string name;
    std::string type;
    std::string file;
    size_t line = 0;
    std::string namespace_;
    std::string description;
    std::vector<std::string> parameters;
    std::string returnType;
    std::map<std::string, std::string> options;
    std::vector<std::string> examples;
    std::vector<std::string> seeAlso;
    bool isDeprecated = false;
    std::string deprecatedMessage;
    bool isPublic = true;
    
    DocItem() = default;
};

// Модуль документации
struct DocModule {
    std::string path;
    std::string name;
    std::string description;
    std::vector<DocItem> items;
    std::vector<std::string> dependencies;
    
    DocModule() = default;
};

// Проект документации
struct DocProject {
    std::string name;
    std::string version;
    std::string generatedAt;
    std::string generatorVersion;
    std::vector<DocModule> modules;
    std::vector<DocItem> items;
    
    DocProject() : generatedAt("unknown"), generatorVersion("1.0.0") {}
};

// Генератор документации
class DocGenerator {
public:
    DocGenerator();
    ~DocGenerator();
    
    // Конфигурация
    void setProjectName(const std::string& name);
    void setProjectVersion(const std::string& version);
    void setOutputDir(const std::string& dir);
    void setFormat(DocFormat newFormat);
    void setIncludePrivate(bool include);
    void setIncludeExamples(bool include);
    void setTemplate(const std::string& templatePath);
    void setTypeChecker(TypeChecker* checker);
    
    // Добавление исходных файлов
    void addSourceFile(const std::string& path, const std::string& content);
    void addSourceDirectory(const std::string& path);
    void addSymbolTable(const SymbolTable& symbols);
    void addAST(ProgramNodePtr ast, const std::string& filename);
    
    // Генерация документации
    bool generate();
    bool generateHTML();
    bool generateMarkdown();
    bool generateXML();
    bool generateJSON();
    
    // Утилиты
    static std::string symbolTypeToString(SymbolKind kind);
    
private:
    // Парсинг комментариев
    void parseComments(const std::string& content, DocModule& module);
    void extractDependencies(const std::string& content, DocModule& module);
    std::string extractNamespace(const std::string& fullName);
    
    // Обход AST
    void traverseAST(ProgramNodePtr node, const std::string& filename);
    void visitDeclaration(DeclarationNodePtr decl, const std::string& filename);
    void visitFunction(FunctionNodePtr func, const std::string& filename);
    void visitClass(ClassNodePtr cls, const std::string& filename);
    void visitVariable(VariableNodePtr var, const std::string& filename);
    
    // Генерация вывода
    bool writeHTMLFile(const std::string& filename, const std::string& content);
    bool writeMarkdownFile(const std::string& filename, const std::string& content);
    bool writeJSONFile(const std::string& filename, const std::string& content);
    std::string escapeHTML(const std::string& text);
    std::string escapeMarkdown(const std::string& text);
    
    // Извлечение документации
    void extractDocFromComments(DeclarationNodePtr decl, DocItem& item);
    std::string getSourceCodeForFile(const std::string& filename) const;
    std::string extractBlockComment(const std::vector<std::string>& lines, size_t endLine);
    void parseDocComment(const std::string& comment, DocItem& item);
    void extractDocFromBody(DeclarationNodePtr decl, DocItem& item);
    
    // Обработка statement'ов
    void processStatement(StatementNodePtr stmt, const std::string& filename);
    void extractDocFromCondition(ExpressionNodePtr condition, const std::string& filename, size_t line);
    void extractDocFromLoop(StatementNodePtr loopStmt, const std::string& filename);
    void extractDocFromExpression(ExpressionNodePtr expr, const std::string& filename, size_t line);
    
    // Обработка специальных конструкций
    void extractRegionDocumentation(const std::string& content, DocModule& module, const std::string& filename);
    void extractNamedBlockComments(const std::string& content, DocModule& module, const std::string& filename);
    void extractGEMInterfaceDocumentation(DocItem& item);
    void extractCodeExamples(const std::string& comment, DocItem& item);
    void extractCrossReferences(DocItem& item);
    void extractLatexFormulas(DocItem& item);
    
    // Валидация и утилиты
    bool validateDocItem(DocItem& item);
    void updateStatistics();
    void extractTypeInfo(DocItem& item);
    std::string typeCategoryToString(TypeCategory category) const;
    
    // Данные
    DocFormat format;
    bool includePrivate;
    bool includeExamples;
    size_t itemsProcessed;
    size_t totalItems;
    std::string outputDir;
    std::string templatePath;
    DocProject project;
    TypeChecker* typeChecker;
};

} // namespace proxima

#endif // PROXIMA_DOC_GENERATOR_H