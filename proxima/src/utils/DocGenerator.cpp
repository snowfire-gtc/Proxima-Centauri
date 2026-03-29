#include "DocGenerator.h"
#include "utils/CollectionParser.h"
#include "utils/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <algorithm>

namespace fs = std::filesystem;

namespace proxima {

// ============================================================================
// Вспомогательные функции
// ============================================================================

static std::string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static std::string escapeHTML(const std::string& text) {
    std::string result;
    result.reserve(text.size() * 1.1);
    for (char c : text) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c; break;
        }
    }
    return result;
}

static std::string escapeMarkdown(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char c : text) {
        if (c == '\\' || c == '`' || c == '*' || c == '_' || c == '{' || c == '}' || 
            c == '[' || c == ']' || c == '(' || c == ')' || c == '#' || c == '+' || 
            c == '-' || c == '.' || c == '!') {
            result += '\\';
        }
        result += c;
    }
    return result;
}

static bool createDirectory(const std::string& path) {
    try {
        return fs::create_directories(path);
    } catch (...) {
        return false;
    }
}

static bool fileExists(const std::string& path) {
    return fs::exists(path);
}

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    return file.good();
}

static std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

DocGenerator::DocGenerator()
    : format(DocFormat::HTML)
    , includePrivate(false)
    , includeExamples(true)
    , itemsProcessed(0)
    , totalItems(0)
    , typeChecker(nullptr) {
    
    project.name = "Proxima Project";
    project.version = "1.0.0";
    project.generatedAt = getCurrentDateTime();
    project.generatorVersion = "1.0.0";
    
    outputDir = "./docs";
    templatePath = "";
    
    LOG_INFO("DocGenerator created");
}

DocGenerator::~DocGenerator() {
    LOG_INFO("DocGenerator destroyed");
}

// ============================================================================
// Конфигурация
// ============================================================================

void DocGenerator::setProjectName(const std::string& name) {
    project.name = name;
    LOG_DEBUG("Project name set to: " + name);
}

void DocGenerator::setProjectVersion(const std::string& version) {
    project.version = version;
    LOG_DEBUG("Project version set to: " + version);
}

void DocGenerator::setOutputDir(const std::string& dir) {
    outputDir = dir;
    
    // Создаём директорию если не существует
    createDirectory(outputDir);
    
    LOG_DEBUG("Output directory set to: " + dir);
}

void DocGenerator::setFormat(DocFormat newFormat) {
    format = newFormat;
    LOG_DEBUG("Documentation format set to: " + std::to_string(static_cast<int>(format)));
}

void DocGenerator::setIncludePrivate(bool include) {
    includePrivate = include;
    LOG_DEBUG("Include private members: " + std::string(include ? "true" : "false"));
}

void DocGenerator::setIncludeExamples(bool include) {
    includeExamples = include;
    LOG_DEBUG("Include examples: " + std::string(include ? "true" : "false"));
}

void DocGenerator::setTemplate(const std::string& tplPath) {
    templatePath = tplPath;
    LOG_DEBUG("Template path set to: " + templatePath);
}

void DocGenerator::setTypeChecker(TypeChecker* checker) {
    typeChecker = checker;
    LOG_DEBUG("TypeChecker set for documentation generation");
}

// ============================================================================
// Добавление исходных файлов
// ============================================================================

void DocGenerator::addSourceFile(const std::string& path, const std::string& content) {
    DocModule module;
    module.path = path;
    
    // Извлекаем имя файла без расширения
    fs::path p(path);
    module.name = p.stem().string();
    module.description = "";
    
    // Парсинг комментариев и извлечение документации
    parseComments(content, module);
    
    // Извлечение зависимостей
    extractDependencies(content, module);
    
    project.modules.push_back(module);
    totalItems += module.items.size();
    
    LOG_INFO("Source file added: " + path + 
             " (" + std::to_string(module.items.size()) + " items)");
}

void DocGenerator::addSourceDirectory(const std::string& path) {
    if (!fs::exists(path)) {
        LOG_WARNING("Directory not found: " + path);
        return;
    }
    
    // Рекурсивный поиск файлов .prx
    try {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".prx") {
                std::string content = readFile(entry.path().string());
                addSourceFile(entry.path().string(), content);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error reading directory: " + std::string(e.what()));
    }
    
    LOG_INFO("Source directory processed: " + path);
}

void DocGenerator::addSymbolTable(const SymbolTable& symbols) {
    auto allSymbols = symbols.getAllSymbols();
    
    for (const Symbol& symbol : allSymbols) {
        DocItem item;
        item.name = symbol.name;
        item.type = symbolTypeToString(symbol.kind);
        item.file = symbol.filename;
        item.line = symbol.line;
        item.namespace_ = extractNamespace(symbol.fullName);
        item.isPublic = (symbol.access == SymbolAccess::Public);
        
        // Извлечение параметров для функций
        if (symbol.kind == SymbolKind::FUNCTION || 
            symbol.kind == SymbolKind::METHOD) {
            for (const auto& param : symbol.parameters) {
                item.parameters.push_back(param.first + ": " + param.second);
            }
            item.returnType = symbol.type;
        }
        
        project.items.push_back(item);
    }
    
    LOG_INFO("Symbol table added: " + std::to_string(allSymbols.size()) + " symbols");
}

void DocGenerator::addAST(ProgramNodePtr ast, const std::string& filename) {
    if (!ast) return;
    
    // Обход AST для извлечения документации
    traverseAST(ast, filename);
    
    LOG_INFO("AST added for: " + filename);
}

// ============================================================================
// Парсинг комментариев
// ============================================================================

void DocGenerator::parseComments(const std::string& content, DocModule& module) {
    // Регулярные выражения для различных типов комментариев
    std::regex classCommentRe(R"((/\*\*|\s*//)\s*@class\s+(\w+))");
    std::regex functionCommentRe(R"((/\*\*|\s*//)\s*@method\s+(\w+))");
    std::regex paramCommentRe(R"((/\*\*|\s*//)\s*@param\s+(\w+):\s*(\w+)\s*-?\s*(.*))");
    std::regex returnCommentRe(R"((/\*\*|\s*//)\s*@return\s+(\w+)\s*-?\s*(.*))");
    std::regex descriptionRe(R"((/\*\*|\s*//)\s*@description\s+(.*))");
    std::regex exampleRe(R"((/\*\*|\s*//)\s*@example\s+(.*))");
    std::regex seeAlsoRe(R"((/\*\*|\s*//)\s*@see\s+(.*))");
    std::regex optionRe(R"((/\*\*|\s*//)\s*@option\s+(\w+):\s*(\w+)\s*-?\s*(.*))");
    std::regex deprecatedRe(R"((/\*\*|\s*//)\s*@deprecated\s*(.*))");
    std::regex sinceRe(R"((/\*\*|\s*//)\s*@since\s+(.*))");
    std::regex authorRe(R"((/\*\*|\s*//)\s*@author\s+(.*))");
    std::regex versionRe(R"((/\*\*|\s*//)\s*@version\s+(.*))");
    std::regex noteRe(R"((/\*\*|\s*//)\s*@note\s+(.*))");
    std::regex warningRe(R"((/\*\*|\s*//)\s*@warning\s+(.*))");
    std::regex todoRe(R"((/\*\*|\s*//)\s*@todo\s+(.*))");
    
    std::vector<std::string> lines = splitLines(content);
    DocItem currentItem;
    bool inDocBlock = false;
    std::string currentDescription;
    
    for (size_t i = 0; i < lines.size(); i++) {
        const std::string& line = lines[i];
        
        // Проверка на начало блока документации
        if (line.find("/**") != std::string::npos || 
            (line.find("// @") != std::string::npos)) {
            inDocBlock = true;
            currentDescription.clear();
        }
        
        if (inDocBlock) {
            // Извлечение описания
            std::smatch descMatch;
            if (std::regex_search(line, descMatch, descriptionRe)) {
                currentDescription += descMatch[1].str() + "\n";
            }
            
            // Извлечение @class
            std::smatch classMatch;
            if (std::regex_search(line, classMatch, classCommentRe)) {
                if (!currentItem.name.empty()) {
                    module.items.push_back(currentItem);
                }
                
                currentItem = DocItem();
                currentItem.name = classMatch[2].str();
                currentItem.type = "class";
                currentItem.line = i + 1;
                currentItem.description = currentDescription;
                inDocBlock = false;
                continue;
            }
            
            // Извлечение @method
            std::smatch funcMatch;
            if (std::regex_search(line, funcMatch, functionCommentRe)) {
                if (!currentItem.name.empty()) {
                    module.items.push_back(currentItem);
                }
                
                currentItem = DocItem();
                currentItem.name = funcMatch[2].str();
                currentItem.type = "method";
                currentItem.line = i + 1;
                currentItem.description = currentDescription;
                inDocBlock = false;
                continue;
            }
            
            // Извлечение @param
            std::smatch paramMatch;
            if (std::regex_search(line, paramMatch, paramCommentRe)) {
                std::string paramName = paramMatch[1].str();
                std::string paramType = paramMatch[2].str();
                std::string paramDesc = paramMatch[4].str();
                currentItem.parameters.push_back(paramName + ": " + paramType + " - " + paramDesc);
            }
            
            // Извлечение @return
            std::smatch returnMatch;
            if (std::regex_search(line, returnMatch, returnCommentRe)) {
                currentItem.returnType = returnMatch[1].str();
                if (!returnMatch[2].str().empty()) {
                    if (!currentItem.description.empty()) {
                        currentItem.description += "\nReturns: " + returnMatch[2].str();
                    } else {
                        currentItem.description = "Returns: " + returnMatch[2].str();
                    }
                }
            }
            
            // Извлечение @option
            std::smatch optionMatch;
            if (std::regex_search(line, optionMatch, optionRe)) {
                std::string optName = optionMatch[1].str();
                std::string optType = optionMatch[2].str();
                std::string optDesc = optionMatch[3].str();
                currentItem.options[optName] = optType + " - " + optDesc;
            }
            
            // Извлечение @example
            std::smatch exampleMatch;
            if (std::regex_search(line, exampleMatch, exampleRe)) {
                currentItem.examples.push_back(exampleMatch[1].str());
            }
            
            // Извлечение @see
            std::smatch seeMatch;
            if (std::regex_search(line, seeMatch, seeAlsoRe)) {
                currentItem.seeAlso.push_back(seeMatch[1].str());
            }
            
            // Извлечение @deprecated
            std::smatch deprecatedMatch;
            if (std::regex_search(line, deprecatedMatch, deprecatedRe)) {
                currentItem.isDeprecated = true;
                currentItem.deprecatedMessage = deprecatedMatch[1].str();
            }
            
            // Извлечение @since
            std::smatch sinceMatch;
            if (std::regex_search(line, sinceMatch, sinceRe)) {
                currentItem.options["since"] = sinceMatch[1].str();
            }
            
            // Извлечение @author
            std::smatch authorMatch;
            if (std::regex_search(line, authorMatch, authorRe)) {
                currentItem.options["author"] = authorMatch[1].str();
            }
            
            // Извлечение @version
            std::smatch versionMatch;
            if (std::regex_search(line, versionMatch, versionRe)) {
                currentItem.options["version"] = versionMatch[1].str();
            }
            
            // Извлечение @note
            std::smatch noteMatch;
            if (std::regex_search(line, noteMatch, noteRe)) {
                currentItem.options["note"] = noteMatch[1].str();
            }
            
            // Извлечение @warning
            std::smatch warningMatch;
            if (std::regex_search(line, warningMatch, warningRe)) {
                currentItem.options["warning"] = warningMatch[1].str();
            }
            
            // Извлечение @todo
            std::smatch todoMatch;
            if (std::regex_search(line, todoMatch, todoRe)) {
                currentItem.options["todo"] = todoMatch[1].str();
            }
        }
        
        // Проверка на конец блока документации
        if (line.find("*/") != std::string::npos) {
            inDocBlock = false;
        }
    }
    
    // Добавляем последний элемент
    if (!currentItem.name.empty()) {
        module.items.push_back(currentItem);
    }
}

void DocGenerator::extractDependencies(const std::string& content, DocModule& module) {
    std::regex importRe(R"(import\s+\"([^\"]+)\"|use\s+(\w+))");
    std::smatch match;
    std::string::const_iterator searchStart(content.cbegin());
    
    while (std::regex_search(searchStart, content.cend(), match, importRe)) {
        if (match[1].matched) {
            module.dependencies.push_back(match[1].str());
        } else if (match[2].matched) {
            module.dependencies.push_back(match[2].str());
        }
        searchStart = match.suffix().first;
    }
}

std::string DocGenerator::extractNamespace(const std::string& fullName) {
    size_t pos = fullName.rfind("::");
    if (pos != std::string::npos) {
        return fullName.substr(0, pos);
    }
    return "";
}

// ============================================================================
// Обход AST
// ============================================================================

void DocGenerator::traverseAST(ProgramNodePtr node, const std::string& filename) {
    if (!node) return;
    
    for (const auto& stmt : node->statements) {
        processStatement(stmt, filename);
    }
}

void DocGenerator::visitDeclaration(DeclarationNodePtr decl, const std::string& filename) {
    if (!decl) return;
    
    DocItem item;
    item.name = decl->name;
    item.file = filename;
    item.line = decl->line;
    
    extractDocFromComments(decl, item);
    extractDocFromBody(decl, item);
    
    if (validateDocItem(item)) {
        project.items.push_back(item);
        itemsProcessed++;
    }
}

void DocGenerator::visitFunction(FunctionNodePtr func, const std::string& filename) {
    if (!func) return;
    
    DocItem item;
    item.name = func->name;
    item.type = "function";
    item.file = filename;
    item.line = func->line;
    
    // Параметры
    for (const auto& param : func->parameters) {
        item.parameters.push_back(param->name + ": " + param->type);
    }
    
    // Возвращаемый тип
    item.returnType = func->returnType;
    
    extractDocFromComments(func, item);
    extractDocFromBody(func, item);
    
    if (validateDocItem(item)) {
        project.items.push_back(item);
        itemsProcessed++;
    }
}

void DocGenerator::visitClass(ClassNodePtr cls, const std::string& filename) {
    if (!cls) return;
    
    DocItem item;
    item.name = cls->name;
    item.type = "class";
    item.file = filename;
    item.line = cls->line;
    
    extractDocFromComments(cls, item);
    extractDocFromBody(cls, item);
    
    if (validateDocItem(item)) {
        project.items.push_back(item);
        itemsProcessed++;
    }
}

void DocGenerator::visitVariable(VariableNodePtr var, const std::string& filename) {
    if (!var) return;
    
    DocItem item;
    item.name = var->name;
    item.type = "variable";
    item.file = filename;
    item.line = var->line;
    item.returnType = var->type;
    
    extractDocFromComments(var, item);
    extractDocFromBody(var, item);
    
    if (validateDocItem(item)) {
        project.items.push_back(item);
        itemsProcessed++;
    }
}

// ============================================================================
// Генерация документации
// ============================================================================

bool DocGenerator::generate() {
    LOG_INFO("Starting documentation generation...");
    
    // Создаём выходную директорию
    if (!createDirectory(outputDir)) {
        LOG_ERROR("Failed to create output directory: " + outputDir);
        return false;
    }
    
    bool success = true;
    
    switch (format) {
        case DocFormat::HTML:
            success = generateHTML();
            break;
        case DocFormat::MARKDOWN:
            success = generateMarkdown();
            break;
        case DocFormat::XML:
            success = generateXML();
            break;
        case DocFormat::JSON:
            success = generateJSON();
            break;
        default:
            LOG_ERROR("Unsupported documentation format");
            success = false;
    }
    
    if (success) {
        LOG_INFO("Documentation generation completed: " + outputDir);
    }
    
    return success;
}

bool DocGenerator::generateHTML() {
    std::ostringstream html;
    
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n";
    html << "<head>\n";
    html << "    <meta charset=\"UTF-8\">\n";
    html << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "    <title>" << escapeHTML(project.name) << " - Documentation</title>\n";
    html << "    <style>\n";
    html << "        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }\n";
    html << "        h1 { color: #333; }\n";
    html << "        h2 { color: #555; border-bottom: 1px solid #ddd; padding-bottom: 5px; }\n";
    html << "        .module { margin: 20px 0; }\n";
    html << "        .item { margin: 10px 0; padding: 10px; background: #f9f9f9; }\n";
    html << "        .deprecated { color: #999; text-decoration: line-through; }\n";
    html << "        code { background: #f0f0f0; padding: 2px 5px; border-radius: 3px; }\n";
    html << "        pre { background: #f0f0f0; padding: 10px; overflow-x: auto; }\n";
    html << "    </style>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "    <h1>" << escapeHTML(project.name) << "</h1>\n";
    html << "    <p>Version: " << escapeHTML(project.version) << "</p>\n";
    html << "    <p>Generated: " << escapeHTML(project.generatedAt) << "</p>\n";
    
    // Модули
    for (const auto& module : project.modules) {
        html << "    <div class=\"module\">\n";
        html << "        <h2>Module: " << escapeHTML(module.name) << "</h2>\n";
        html << "        <p>" << escapeHTML(module.description) << "</p>\n";
        
        for (const auto& item : module.items) {
            html << "        <div class=\"item\">\n";
            html << "            <h3>" << escapeHTML(item.name) << "</h3>\n";
            html << "            <p>Type: " << escapeHTML(item.type) << "</p>\n";
            if (item.isDeprecated) {
                html << "            <p class=\"deprecated\">Deprecated: " << escapeHTML(item.deprecatedMessage) << "</p>\n";
            }
            html << "            <p>" << escapeHTML(item.description) << "</p>\n";
            if (!item.parameters.empty()) {
                html << "            <p><strong>Parameters:</strong></p>\n";
                html << "            <ul>\n";
                for (const auto& param : item.parameters) {
                    html << "                <li>" << escapeHTML(param) << "</li>\n";
                }
                html << "            </ul>\n";
            }
            if (!item.returnType.empty()) {
                html << "            <p><strong>Returns:</strong> " << escapeHTML(item.returnType) << "</p>\n";
            }
            html << "        </div>\n";
        }
        html << "    </div>\n";
    }
    
    html << "</body>\n";
    html << "</html>\n";
    
    std::string outputPath = outputDir + "/index.html";
    return writeHTMLFile(outputPath, html.str());
}

bool DocGenerator::generateMarkdown() {
    std::ostringstream md;
    
    md << "# " << project.name << "\n\n";
    md << "**Version:** " << project.version << "\n\n";
    md << "**Generated:** " << project.generatedAt << "\n\n";
    
    // Модули
    for (const auto& module : project.modules) {
        md << "## Module: " << module.name << "\n\n";
        md << module.description << "\n\n";
        
        for (const auto& item : module.items) {
            md << "### " << item.name << "\n\n";
            md << "**Type:** " << item.type << "\n\n";
            if (item.isDeprecated) {
                md << "> **Deprecated:** " << item.deprecatedMessage << "\n\n";
            }
            md << item.description << "\n\n";
            if (!item.parameters.empty()) {
                md << "**Parameters:**\n\n";
                for (const auto& param : item.parameters) {
                    md << "- " << param << "\n";
                }
                md << "\n";
            }
            if (!item.returnType.empty()) {
                md << "**Returns:** " << item.returnType << "\n\n";
            }
        }
    }
    
    std::string outputPath = outputDir + "/README.md";
    return writeMarkdownFile(outputPath, md.str());
}

bool DocGenerator::generateXML() {
    std::ostringstream xml;
    
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<documentation>\n";
    xml << "    <project>\n";
    xml << "        <name>" << escapeHTML(project.name) << "</name>\n";
    xml << "        <version>" << escapeHTML(project.version) << "</version>\n";
    xml << "        <generatedAt>" << escapeHTML(project.generatedAt) << "</generatedAt>\n";
    xml << "    </project>\n";
    
    for (const auto& module : project.modules) {
        xml << "    <module name=\"" << escapeHTML(module.name) << "\">\n";
        xml << "        <description>" << escapeHTML(module.description) << "</description>\n";
        
        for (const auto& item : module.items) {
            xml << "        <item>\n";
            xml << "            <name>" << escapeHTML(item.name) << "</name>\n";
            xml << "            <type>" << escapeHTML(item.type) << "</type>\n";
            xml << "            <description>" << escapeHTML(item.description) << "</description>\n";
            if (item.isDeprecated) {
                xml << "            <deprecated>" << escapeHTML(item.deprecatedMessage) << "</deprecated>\n";
            }
            xml << "        </item>\n";
        }
        xml << "    </module>\n";
    }
    
    xml << "</documentation>\n";
    
    std::string outputPath = outputDir + "/documentation.xml";
    return writeFile(outputPath, xml.str());
}

bool DocGenerator::generateJSON() {
    std::ostringstream json;
    
    json << "{\n";
    json << "  \"project\": {\n";
    json << "    \"name\": \"" << project.name << "\",\n";
    json << "    \"version\": \"" << project.version << "\",\n";
    json << "    \"generatedAt\": \"" << project.generatedAt << "\"\n";
    json << "  },\n";
    json << "  \"modules\": [\n";
    
    bool firstModule = true;
    for (const auto& module : project.modules) {
        if (!firstModule) json << ",\n";
        firstModule = false;
        
        json << "    {\n";
        json << "      \"name\": \"" << module.name << "\",\n";
        json << "      \"description\": \"" << module.description << "\",\n";
        json << "      \"items\": [\n";
        
        bool firstItem = true;
        for (const auto& item : module.items) {
            if (!firstItem) json << ",\n";
            firstItem = false;
            
            json << "        {\n";
            json << "          \"name\": \"" << item.name << "\",\n";
            json << "          \"type\": \"" << item.type << "\",\n";
            json << "          \"description\": \"" << item.description << "\"\n";
            json << "        }";
        }
        
        json << "\n      ]\n";
        json << "    }";
    }
    
    json << "\n  ]\n";
    json << "}\n";
    
    std::string outputPath = outputDir + "/documentation.json";
    return writeJSONFile(outputPath, json.str());
}

// ============================================================================
// Утилиты записи
// ============================================================================

bool DocGenerator::writeHTMLFile(const std::string& filename, const std::string& content) {
    return writeFile(filename, content);
}

bool DocGenerator::writeMarkdownFile(const std::string& filename, const std::string& content) {
    return writeFile(filename, content);
}

bool DocGenerator::writeJSONFile(const std::string& filename, const std::string& content) {
    return writeFile(filename, content);
}

std::string DocGenerator::escapeHTML(const std::string& text) {
    return ::escapeHTML(text);
}

std::string DocGenerator::escapeMarkdown(const std::string& text) {
    return ::escapeMarkdown(text);
}

// ============================================================================
// Извлечение документации
// ============================================================================

void DocGenerator::extractDocFromComments(DeclarationNodePtr decl, DocItem& item) {
    if (!decl) return;
    // Заглушка для будущей реализации
}

std::string DocGenerator::getSourceCodeForFile(const std::string& filename) const {
    return readFile(filename);
}

std::string DocGenerator::extractBlockComment(const std::vector<std::string>& lines, size_t endLine) {
    std::string comment;
    // Заглушка для будущей реализации
    return comment;
}

void DocGenerator::parseDocComment(const std::string& comment, DocItem& item) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractDocFromBody(DeclarationNodePtr decl, DocItem& item) {
    if (!decl) return;
    // Заглушка для будущей реализации
}

// ============================================================================
// Обработка statement'ов
// ============================================================================

void DocGenerator::processStatement(StatementNodePtr stmt, const std::string& filename) {
    if (!stmt) return;
    
    // Обработка различных типов statement'ов
    if (auto func = std::dynamic_pointer_cast<FunctionNode>(stmt)) {
        visitFunction(func, filename);
    } else if (auto cls = std::dynamic_pointer_cast<ClassNode>(stmt)) {
        visitClass(cls, filename);
    } else if (auto var = std::dynamic_pointer_cast<VariableNode>(stmt)) {
        visitVariable(var, filename);
    }
}

void DocGenerator::extractDocFromCondition(ExpressionNodePtr condition, const std::string& filename, size_t line) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractDocFromLoop(StatementNodePtr loopStmt, const std::string& filename) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractDocFromExpression(ExpressionNodePtr expr, const std::string& filename, size_t line) {
    // Заглушка для будущей реализации
}

// ============================================================================
// Обработка специальных конструкций
// ============================================================================

void DocGenerator::extractRegionDocumentation(const std::string& content, DocModule& module, const std::string& filename) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractNamedBlockComments(const std::string& content, DocModule& module, const std::string& filename) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractGEMInterfaceDocumentation(DocItem& item) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractCodeExamples(const std::string& comment, DocItem& item) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractCrossReferences(DocItem& item) {
    // Заглушка для будущей реализации
}

void DocGenerator::extractLatexFormulas(DocItem& item) {
    // Заглушка для будущей реализации
}

// ============================================================================
// Валидация и утилиты
// ============================================================================

bool DocGenerator::validateDocItem(DocItem& item) {
    return !item.name.empty();
}

void DocGenerator::updateStatistics() {
    // Заглушка для будущей реализации
}

void DocGenerator::extractTypeInfo(DocItem& item) {
    // Заглушка для будущей реализации
}

std::string DocGenerator::typeCategoryToString(TypeCategory category) const {
    switch (category) {
        case TypeCategory::INT: return "int";
        case TypeCategory::FLOAT: return "float";
        case TypeCategory::STRING: return "string";
        case TypeCategory::BOOL: return "bool";
        case TypeCategory::ARRAY: return "array";
        case TypeCategory::OBJECT: return "object";
        case TypeCategory::FUNCTION: return "function";
        case TypeCategory::VOID: return "void";
        default: return "unknown";
    }
}

std::string DocGenerator::symbolTypeToString(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::VARIABLE: return "variable";
        case SymbolKind::CONSTANT: return "constant";
        case SymbolKind::FUNCTION: return "function";
        case SymbolKind::METHOD: return "method";
        case SymbolKind::CLASS: return "class";
        case SymbolKind::MODULE: return "module";
        case SymbolKind::NAMESPACE: return "namespace";
        case SymbolKind::PARAMETER: return "parameter";
        case SymbolKind::TYPE: return "type";
        default: return "unknown";
    }
}

} // namespace proxima
