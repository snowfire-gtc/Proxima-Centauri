#ifndef PROXIMA_DOC_GENERATOR_H
#define PROXIMA_DOC_GENERATOR_H

#include "../parser/AST.h"
#include "../semantic/SymbolTable.h"
#include <string>
#include <vector>
#include <map>

namespace proxima {

struct DocEntry {
    std::string name;
    std::string type;
    std::string description;
    std::string filename;
    int line;
    std::vector<std::string> parameters;
    std::string returnType;
    std::map<std::string, std::string> options;
    std::vector<std::string> examples;
    std::vector<std::string> seeAlso;
};

enum class DocFormat {
    HTML,
    PDF,
    MARKDOWN,
    LATEX
};

class DocGenerator {
public:
    DocGenerator();
    
    void setProjectName(const std::string& name) { projectName = name; }
    void setVersion(const std::string& version) { projectVersion = version; }
    void setOutputDir(const std::string& dir) { outputDir = dir; }
    void setFormat(DocFormat format) { docFormat = format; }
    
    void addFile(const std::string& filename, const std::string& content);
    void addSymbolTable(const SymbolTable& symbols);
    
    bool generate();
    bool generateHTML();
    bool generateMarkdown();
    bool generatePDF();
    bool generateLatex();
    
    std::string getLastError() const { return lastError; }
    
private:
    std::string projectName;
    std::string projectVersion;
    std::string outputDir;
    DocFormat docFormat;
    std::vector<DocEntry> entries;
    std::string lastError;
    
    void parseComments(const std::string& filename, const std::string& content);
    void extractDocEntry(const std::string& comment, DocEntry& entry);
    std::string renderHTML(const DocEntry& entry);
    std::string renderMarkdown(const DocEntry& entry);
    std::string renderLatex(const DocEntry& entry);
    std::string processLatexFormulas(const std::string& text);
    std::string generateTableOfContents();
    std::string generateIndex();
};

} // namespace proxima

#endif // PROXIMA_DOC_GENERATOR_H