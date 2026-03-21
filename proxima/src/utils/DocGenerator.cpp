#include "DocGenerator.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>

namespace proxima {

DocGenerator::DocGenerator() 
    : docFormat(DocFormat::HTML), outputDir("./docs") {}

void DocGenerator::addFile(const std::string& filename, const std::string& content) {
    parseComments(filename, content);
}

void DocGenerator::addSymbolTable(const SymbolTable& symbols) {
    auto allSymbols = symbols.getAllSymbols();
    
    for (const auto& sym : allSymbols) {
        DocEntry entry;
        entry.name = sym.name;
        entry.type = sym.type;
        entry.filename = sym.filename;
        entry.line = sym.line;
        
        for (const auto& param : sym.parameters) {
            entry.parameters.push_back(param.first + ": " + param.second);
        }
        
        entries.push_back(entry);
    }
}

void DocGenerator::parseComments(const std::string& filename, const std::string& content) {
    // Parse documentation comments
    // Look for patterns like:
    // // @method functionName
    // // @param name: type - description
    // // @return type - description
    
    std::regex commentPattern(R"((//|/\*)\s*@(\w+)\s*(.*))");
    std::regex methodPattern(R"((//|/\*)\s*(.*?)\s*(\n|$))");
    
    std::istringstream iss(content);
    std::string line;
    DocEntry currentEntry;
    bool inDocBlock = false;
    
    while (std::getline(iss, line)) {
        std::smatch match;
        
        if (std::regex_search(line, match, commentPattern)) {
            inDocBlock = true;
            std::string tag = match[2].str();
            std::string content = match[3].str();
            
            if (tag == "method" || tag == "function") {
                if (!currentEntry.name.empty()) {
                    entries.push_back(currentEntry);
                }
                currentEntry = DocEntry();
                currentEntry.name = content;
                currentEntry.filename = filename;
            }
            else if (tag == "param") {
                currentEntry.parameters.push_back(content);
            }
            else if (tag == "return") {
                currentEntry.returnType = content;
            }
            else if (tag == "description") {
                currentEntry.description = content;
            }
            else if (tag == "option") {
                size_t colonPos = content.find(":");
                if (colonPos != std::string::npos) {
                    std::string key = content.substr(0, colonPos);
                    std::string value = content.substr(colonPos + 1);
                    currentEntry.options[key] = value;
                }
            }
            else if (tag == "example") {
                currentEntry.examples.push_back(content);
            }
            else if (tag == "see") {
                currentEntry.seeAlso.push_back(content);
            }
        } else if (inDocBlock && line.find("//") == std::string::npos) {
            inDocBlock = false;
            if (!currentEntry.name.empty()) {
                entries.push_back(currentEntry);
                currentEntry = DocEntry();
            }
        }
    }
    
    if (!currentEntry.name.empty()) {
        entries.push_back(currentEntry);
    }
}

bool DocGenerator::generate() {
    switch (docFormat) {
        case DocFormat::HTML:
            return generateHTML();
        case DocFormat::MARKDOWN:
            return generateMarkdown();
        case DocFormat::PDF:
            return generatePDF();
        case DocFormat::LATEX:
            return generateLatex();
        default:
            return generateHTML();
    }
}

bool DocGenerator::generateHTML() {
    std::string filename = outputDir + "/index.html";
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        lastError = "Could not create file: " + filename;
        return false;
    }
    
    file << "<!DOCTYPE html>\n";
    file << "<html>\n<head>\n";
    file << "<title>" << projectName << " Documentation</title>\n";
    file << "<style>\n";
    file << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    file << "h1 { color: #333; }\n";
    file << "h2 { color: #666; border-bottom: 1px solid #ccc; }\n";
    file << ".method { background: #f5f5f5; padding: 15px; margin: 10px 0; }\n";
    file << ".param { color: #0066cc; }\n";
    file << ".return { color: #009900; }\n";
    file << ".example { background: #eee; padding: 10px; font-family: monospace; }\n";
    file << "</style>\n";
    file << "</head>\n<body>\n";
    
    file << "<h1>" << projectName << "</h1>\n";
    file << "<p>Version: " << projectVersion << "</p>\n";
    
    file << "<h2>Table of Contents</h2>\n";
    file << "<ul>\n";
    for (const auto& entry : entries) {
        file << "<li><a href=\"#_" << entry.name << "\">" << entry.name << "</a></li>\n";
    }
    file << "</ul>\n";
    
    file << "<h2>API Reference</h2>\n";
    
    for (const auto& entry : entries) {
        file << renderHTML(entry);
    }
    
    file << "</body>\n</html>\n";
    file.close();
    
    std::cout << "Documentation generated: " << filename << std::endl;
    return true;
}

std::string DocGenerator::renderHTML(const DocEntry& entry) {
    std::ostringstream oss;
    
    oss << "<div class=\"method\" id=\"_" << entry.name << "\">\n";
    oss << "<h3>" << entry.name << "</h3>\n";
    
    if (!entry.description.empty()) {
        oss << "<p>" << entry.description << "</p>\n";
    }
    
    if (!entry.parameters.empty()) {
        oss << "<p><strong>Parameters:</strong></p>\n<ul>\n";
        for (const auto& param : entry.parameters) {
            oss << "<li class=\"param\">" << param << "</li>\n";
        }
        oss << "</ul>\n";
    }
    
    if (!entry.returnType.empty()) {
        oss << "<p class=\"return\"><strong>Returns:</strong> " << entry.returnType << "</p>\n";
    }
    
    if (!entry.examples.empty()) {
        oss << "<p><strong>Examples:</strong></p>\n";
        for (const auto& example : entry.examples) {
            oss << "<div class=\"example\">" << example << "</div>\n";
        }
    }
    
    if (!entry.seeAlso.empty()) {
        oss << "<p><strong>See Also:</strong> ";
        for (size_t i = 0; i < entry.seeAlso.size(); i++) {
            if (i > 0) oss << ", ";
            oss << entry.seeAlso[i];
        }
        oss << "</p>\n";
    }
    
    oss << "<p><small>" << entry.filename << ":" << entry.line << "</small></p>\n";
    oss << "</div>\n";
    
    return oss.str();
}

bool DocGenerator::generateMarkdown() {
    std::string filename = outputDir + "/README.md";
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        lastError = "Could not create file: " + filename;
        return false;
    }
    
    file << "# " << projectName << " Documentation\n\n";
    file << "**Version:** " << projectVersion << "\n\n";
    
    file << "## Table of Contents\n\n";
    for (const auto& entry : entries) {
        file << "- [" << entry.name << "](#" << entry.name << ")\n";
    }
    file << "\n";
    
    file << "## API Reference\n\n";
    
    for (const auto& entry : entries) {
        file << renderMarkdown(entry);
    }
    
    file.close();
    
    std::cout << "Documentation generated: " << filename << std::endl;
    return true;
}

std::string DocGenerator::renderMarkdown(const DocEntry& entry) {
    std::ostringstream oss;
    
    oss << "### " << entry.name << "\n\n";
    
    if (!entry.description.empty()) {
        oss << entry.description << "\n\n";
    }
    
    if (!entry.parameters.empty()) {
        oss << "**Parameters:**\n\n";
        for (const auto& param : entry.parameters) {
            oss << "- `" << param << "`\n";
        }
        oss << "\n";
    }
    
    if (!entry.returnType.empty()) {
        oss << "**Returns:** `" << entry.returnType << "`\n\n";
    }
    
    if (!entry.examples.empty()) {
        oss << "**Examples:**\n\n```proxima\n";
        for (const auto& example : entry.examples) {
            oss << example << "\n";
        }
        oss << "```\n\n";
    }
    
    oss << "*" << entry.filename << ":" << entry.line << "*\n\n";
    
    return oss.str();
}

bool DocGenerator::generatePDF() {
    // Would use a library like wkhtmltopdf or similar
    // For now, generate HTML and convert
    generateHTML();
    lastError = "PDF generation requires external tools";
    return false;
}

bool DocGenerator::generateLatex() {
    std::string filename = outputDir + "/documentation.tex";
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        lastError = "Could not create file: " + filename;
        return false;
    }
    
    file << "\\documentclass{article}\n";
    file << "\\usepackage[utf8]{inputenc}\n";
    file << "\\usepackage{hyperref}\n";
    file << "\\usepackage{listings}\n";
    file << "\\usepackage{xcolor}\n";
    file << "\\title{" << projectName << "}\n";
    file << "\\author{Proxima Development Team}\n";
    file << "\\date{Version " << projectVersion << "}\n";
    file << "\\begin{document}\n";
    file << "\\maketitle\n";
    file << "\\tableofcontents\n";
    file << "\\newpage\n";
    
    for (const auto& entry : entries) {
        file << renderLatex(entry);
    }
    
    file << "\\end{document}\n";
    file.close();
    
    std::cout << "Documentation generated: " << filename << std::endl;
    return true;
}

std::string DocGenerator::renderLatex(const DocEntry& entry) {
    std::ostringstream oss;
    
    oss << "\\section{" << entry.name << "}\n\n";
    
    if (!entry.description.empty()) {
        oss << entry.description << "\n\n";
    }
    
    if (!entry.parameters.empty()) {
        oss << "\\textbf{Parameters:}\n\\begin{itemize}\n";
        for (const auto& param : entry.parameters) {
            oss << "\\item \\texttt{" << param << "}\n";
        }
        oss << "\\end{itemize}\n\n";
    }
    
    if (!entry.returnType.empty()) {
        oss << "\\textbf{Returns:} \\texttt{" << entry.returnType << "}\n\n";
    }
    
    return oss.str();
}

std::string DocGenerator::processLatexFormulas(const std::string& text) {
    // Convert LaTeX-like formulas to proper LaTeX
    std::string result = text;
    
    // Example: $x^2$ -> $x^2$
    // This is a placeholder for more complex formula processing
    
    return result;
}

std::string DocGenerator::generateTableOfContents() {
    std::ostringstream oss;
    
    oss << "<h2>Table of Contents</h2>\n<ul>\n";
    for (const auto& entry : entries) {
        oss << "<li><a href=\"#_" << entry.name << "\">" << entry.name << "</a></li>\n";
    }
    oss << "</ul>\n";
    
    return oss.str();
}

std::string DocGenerator::generateIndex() {
    std::ostringstream oss;
    
    oss << "<h2>Index</h2>\n<ul>\n";
    
    // Sort entries alphabetically
    std::vector<DocEntry> sorted = entries;
    std::sort(sorted.begin(), sorted.end(), 
        [](const DocEntry& a, const DocEntry& b) {
            return a.name < b.name;
        });
    
    for (const auto& entry : sorted) {
        oss << "<li><a href=\"#_" << entry.name << "\">" << entry.name << "</a></li>\n";
    }
    oss << "</ul>\n";
    
    return oss.str();
}

} // namespace proxima