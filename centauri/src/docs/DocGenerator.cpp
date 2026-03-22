#include "DocGenerator.h"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QReadWriteLock>
#include <QAtomicInt>
#include <QAtomicPointer>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QExplicitlySharedDataPointer>
#include <QImplicitlySharedDataPointer>
#include <QDataStream>
#include <QDebug>
#include <QBuffer>
#include <QByteArray>
#include <QChar>
#include <QStringList>
#include <QTextCodec>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QIODevice>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>
#include <QDomText>
#include <QDomCDATASection>
#include <QDomProcessingInstruction>
#include <QDomComment>
#include <QDomDocumentType>
#include <QDomNotation>
#include <QDomEntity>
#include <QDomEntityReference>
#include <QDomCharacterData>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomNamedNodeMap>
#include <QDomImplementation>
#include <QDomDocumentFragment>
#include <QDomTypeInfo>
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QXmlItem>
#include <QXmlName>
#include <QXmlNamePool>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlFormatter>
#include <QXmlSerializer>
#include <QAbstractMessageHandler>
#include <QSourceLocation>
#include <QNetworkDiskCache>
#include <QNetworkCacheMetaData>
#include <QAbstractNetworkCache>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QCookieJar>
#include <QAbstractCookieJar>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificate>
#include <QSslCipher>
#include <QSslError>
#include <QSslPreSharedKeyAuthenticator>
#include <QAuthenticator>
#include <QProxyFactory>
#include <QAbstractProxyFactory>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

DocGenerator::DocGenerator(QObject *parent)
    : QObject(parent)
    , format(DocFormat::HTML)
    , includePrivate(false)
    , includeExamples(true)
    , symbolTable(nullptr)
    , itemsProcessed(0)
    , totalItems(0)
    , generationTime(0)
    , networkManager(nullptr)
    , progressDialog(nullptr) {
    
    project.name = "Proxima Project";
    project.version = "1.0.0";
    project.generatedAt = QDateTime::currentDateTime();
    project.generatorVersion = "1.0.0";
    project.language = "ru";
    project.title = "Документация проекта";
    project.outputFormat = "html";
    
    outputDir = "./docs";
    templatePath = "";
    language = "ru";
    
    // Инициализация сетевого менеджера
    networkManager = new QNetworkAccessManager(this);
    
    LOG_INFO("DocGenerator created for IDE");
}

DocGenerator::~DocGenerator() {
    LOG_INFO("DocGenerator destroyed");
}

// ============================================================================
// Конфигурация
// ============================================================================

void DocGenerator::setProjectName(const QString& name) {
    project.name = name;
    LOG_DEBUG("Project name set to: " + name.toStdString());
}

void DocGenerator::setProjectVersion(const QString& version) {
    project.version = version;
    LOG_DEBUG("Project version set to: " + version.toStdString());
}

void DocGenerator::setOutputDir(const QString& dir) {
    outputDir = dir;
    
    // Создаём директорию если не существует
    createDirectory(outputDir);
    
    LOG_DEBUG("Output directory set to: " + dir.toStdString());
}

void DocGenerator::setFormat(DocFormat newFormat) {
    format = newFormat;
    project.outputFormat = QString::number(static_cast<int>(newFormat));
    LOG_DEBUG("Documentation format set to: " + std::to_string(static_cast<int>(format)));
}

void DocGenerator::setIncludePrivate(bool include) {
    includePrivate = include;
    project.includePrivate = include;
    LOG_DEBUG("Include private members: " + std::string(include ? "true" : "false"));
}

void DocGenerator::setIncludeExamples(bool include) {
    includeExamples = include;
    project.includeExamples = include;
    LOG_DEBUG("Include examples: " + std::string(include ? "true" : "false"));
}

void DocGenerator::setTemplate(const QString& templatePath) {
    this->templatePath = templatePath;
    project.templatePath = templatePath;
    LOG_DEBUG("Template path set to: " + templatePath.toStdString());
}

void DocGenerator::setLanguage(const QString& lang) {
    language = lang;
    project.language = lang;
    LOG_DEBUG("Documentation language set to: " + lang.toStdString());
}

void DocGenerator::setTitle(const QString& title) {
    project.title = title;
    LOG_DEBUG("Documentation title set to: " + title.toStdString());
}

void DocGenerator::setAuthor(const QString& author) {
    project.author = author;
    LOG_DEBUG("Documentation author set to: " + author.toStdString());
}

void DocGenerator::setCopyright(const QString& copyright) {
    project.copyright = copyright;
    LOG_DEBUG("Documentation copyright set to: " + copyright.toStdString());
}

void DocGenerator::setLicense(const QString& license) {
    project.license = license;
    LOG_DEBUG("Documentation license set to: " + license.toStdString());
}

// ============================================================================
// Добавление исходных файлов
// ============================================================================

void DocGenerator::addSourceFile(const QString& path, const QString& content) {
    DocModule module;
    module.path = path;
    module.name = QFileInfo(path).baseName();
    module.description = "";
    
    // Парсинг комментариев и извлечение документации
    parseComments(content, module);
    
    // Извлечение зависимостей
    extractDependencies(content, module);
    
    project.modules.append(module);
    totalItems += module.items.size();
    
    emit fileProcessed(path);
    
    LOG_INFO("Source file added: " + path.toStdString() + 
             " (" + std::to_string(module.items.size()) + " items)");
}

void DocGenerator::addSourceDirectory(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        LOG_WARNING("Directory not found: " + path.toStdString());
        return;
    }
    
    // Рекурсивный поиск файлов .prx
    QStringList filters;
    filters << "*.prx";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo& fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setCodec("UTF-8");
            QString content = in.readAll();
            file.close();
            
            addSourceFile(fileInfo.absoluteFilePath(), content);
        }
    }
    
    // Обработка поддиректорий
    QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& dirInfo : dirs) {
        addSourceDirectory(dirInfo.absoluteFilePath());
    }
    
    LOG_INFO("Source directory processed: " + path.toStdString());
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
                item.parameters.append(param.first + ": " + param.second);
            }
            item.returnType = symbol.type;
        }
        
        project.items.append(item);
        emit itemAdded(item);
    }
    
    LOG_INFO("Symbol table added: " + std::to_string(allSymbols.size()) + " symbols");
}

void DocGenerator::addAST(ProgramNodePtr ast, const QString& filename) {
    if (!ast) return;
    
    // Обход AST для извлечения документации
    traverseAST(ast, filename);
    
    asts[filename] = ast;
    
    LOG_INFO("AST added for: " + filename.toStdString());
}

// ============================================================================
// Парсинг комментариев
// ============================================================================

void DocGenerator::parseComments(const QString& content, DocModule& module) {
    // Регулярные выражения для различных типов комментариев
    QRegularExpression classCommentRe(R"((/\*\*|\s*//)\s*@class\s+(\w+))");
    QRegularExpression functionCommentRe(R"((/\*\*|\s*//)\s*@method\s+(\w+))");
    QRegularExpression paramCommentRe(R"((/\*\*|\s*//)\s*@param\s+(\w+):\s*(\w+)\s*-?\s*(.*))");
    QRegularExpression returnCommentRe(R"((/\*\*|\s*//)\s*@return\s+(\w+)\s*-?\s*(.*))");
    QRegularExpression descriptionRe(R"((/\*\*|\s*//)\s*@description\s+(.*))");
    QRegularExpression exampleRe(R"((/\*\*|\s*//)\s*@example\s+(.*))");
    QRegularExpression seeAlsoRe(R"((/\*\*|\s*//)\s*@see\s+(.*))");
    QRegularExpression optionRe(R"((/\*\*|\s*//)\s*@option\s+(\w+):\s*(\w+)\s*-?\s*(.*))");
    QRegularExpression deprecatedRe(R"((/\*\*|\s*//)\s*@deprecated\s*(.*))");
    QRegularExpression sinceRe(R"((/\*\*|\s*//)\s*@since\s+(.*))");
    QRegularExpression authorRe(R"((/\*\*|\s*//)\s*@author\s+(.*))");
    QRegularExpression versionRe(R"((/\*\*|\s*//)\s*@version\s+(.*))");
    QRegularExpression noteRe(R"((/\*\*|\s*//)\s*@note\s+(.*))");
    QRegularExpression warningRe(R"((/\*\*|\s*//)\s*@warning\s+(.*))");
    QRegularExpression todoRe(R"((/\*\*|\s*//)\s*@todo\s+(.*))");
    
    QStringList lines = content.split("\n");
    DocItem currentItem;
    bool inDocBlock = false;
    QString currentDescription;
    
    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i];
        
        // Проверка на начало блока документации
        if (line.contains("/**") || line.contains("// @")) {
            inDocBlock = true;
            currentDescription.clear();
        }
        
        if (inDocBlock) {
            // Извлечение описания
            QRegularExpressionMatch descMatch = descriptionRe.match(line);
            if (descMatch.hasMatch()) {
                currentDescription += descMatch.captured(1) + "\n";
            }
            
            // Извлечение @class
            QRegularExpressionMatch classMatch = classCommentRe.match(line);
            if (classMatch.hasMatch()) {
                if (!currentItem.name.isEmpty()) {
                    module.items.append(currentItem);
                    emit itemAdded(currentItem);
                }
                
                currentItem = DocItem();
                currentItem.name = classMatch.captured(2);
                currentItem.type = "class";
                currentItem.line = i + 1;
                currentItem.description = currentDescription;
                inDocBlock = false;
                continue;
            }
            
            // Извлечение @method
            QRegularExpressionMatch funcMatch = functionCommentRe.match(line);
            if (funcMatch.hasMatch()) {
                if (!currentItem.name.isEmpty()) {
                    module.items.append(currentItem);
                    emit itemAdded(currentItem);
                }
                
                currentItem = DocItem();
                currentItem.name = funcMatch.captured(2);
                currentItem.type = "method";
                currentItem.line = i + 1;
                currentItem.description = currentDescription;
                inDocBlock = false;
                continue;
            }
            
            // Извлечение @param
            QRegularExpressionMatch paramMatch = paramCommentRe.match(line);
            if (paramMatch.hasMatch()) {
                QString paramName = paramMatch.captured(1);
                QString paramType = paramMatch.captured(2);
                QString paramDesc = paramMatch.captured(4);
                currentItem.parameters.append(paramName + ": " + paramType + " - " + paramDesc);
            }
            
            // Извлечение @return
            QRegularExpressionMatch returnMatch = returnCommentRe.match(line);
            if (returnMatch.hasMatch()) {
                currentItem.returnType = returnMatch.captured(1);
                if (!returnMatch.captured(2).isEmpty()) {
                    if (!currentItem.description.isEmpty()) {
                        currentItem.description += "\nReturns: " + returnMatch.captured(2);
                    } else {
                        currentItem.description = "Returns: " + returnMatch.captured(2);
                    }
                }
            }
            
            // Извлечение @option
            QRegularExpressionMatch optionMatch = optionRe.match(line);
            if (optionMatch.hasMatch()) {
                QString optName = optionMatch.captured(1);
                QString optType = optionMatch.captured(2);
                QString optDesc = optionMatch.captured(3);
                currentItem.options[optName] = optType + " - " + optDesc;
            }
            
            // Извлечение @example
            QRegularExpressionMatch exampleMatch = exampleRe.match(line);
            if (exampleMatch.hasMatch()) {
                currentItem.examples.append(exampleMatch.captured(1));
            }
            
            // Извлечение @see
            QRegularExpressionMatch seeMatch = seeAlsoRe.match(line);
            if (seeMatch.hasMatch()) {
                currentItem.seeAlso.append(seeMatch.captured(1));
            }
            
            // Извлечение @deprecated
            QRegularExpressionMatch deprecatedMatch = deprecatedRe.match(line);
            if (deprecatedMatch.hasMatch()) {
                currentItem.isDeprecated = true;
                currentItem.deprecatedMessage = deprecatedMatch.captured(1);
            }
            
            // Извлечение @since
            QRegularExpressionMatch sinceMatch = sinceRe.match(line);
            if (sinceMatch.hasMatch()) {
                currentItem.since = sinceMatch.captured(1);
            }
            
            // Извлечение @author
            QRegularExpressionMatch authorMatch = authorRe.match(line);
            if (authorMatch.hasMatch()) {
                currentItem.author = authorMatch.captured(1);
            }
            
            // Извлечение @version
            QRegularExpressionMatch versionMatch = versionRe.match(line);
            if (versionMatch.hasMatch()) {
                currentItem.version = versionMatch.captured(1);
            }
            
            // Извлечение @note
            QRegularExpressionMatch noteMatch = noteRe.match(line);
            if (noteMatch.hasMatch()) {
                currentItem.notes.append(noteMatch.captured(1));
            }
            
            // Извлечение @warning
            QRegularExpressionMatch warningMatch = warningRe.match(line);
            if (warningMatch.hasMatch()) {
                currentItem.warnings.append(warningMatch.captured(1));
            }
            
            // Извлечение @todo
            QRegularExpressionMatch todoMatch = todoRe.match(line);
            if (todoMatch.hasMatch()) {
                currentItem.todos.append(todoMatch.captured(1));
            }
        }
        
        // Конец блока документации
        if (line.contains("*/") || (!line.contains("@") && !line.contains("//"))) {
            if (inDocBlock && !currentItem.name.isEmpty()) {
                module.items.append(currentItem);
                emit itemAdded(currentItem);
                currentItem = DocItem();
            }
            inDocBlock = false;
        }
    }
    
    // Добавление последнего элемента
    if (!currentItem.name.isEmpty()) {
        module.items.append(currentItem);
        emit itemAdded(currentItem);
    }
}

void DocGenerator::parseDocComment(const QString& comment, DocItem& item) {
    // Извлечение тегов из комментария
    extractTag(comment, "@description", item.description);
    extractTag(comment, "@param", item.parameters);
    extractTag(comment, "@return", item.returnType);
    extractTag(comment, "@example", item.examples);
    extractTag(comment, "@see", item.seeAlso);
    extractTag(comment, "@deprecated", item.deprecatedMessage);
    extractTag(comment, "@since", item.since);
    extractTag(comment, "@author", item.author);
    extractTag(comment, "@version", item.version);
    extractTag(comment, "@note", item.notes);
    extractTag(comment, "@warning", item.warnings);
    extractTag(comment, "@todo", item.todos);
    
    if (!item.deprecatedMessage.isEmpty()) {
        item.isDeprecated = true;
    }
}

void DocGenerator::extractTag(const QString& comment, const QString& tag, QString& value) {
    QRegularExpression re(tag + "\\s+(.*?)(?=@|\\*/|$)", 
                         QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = re.match(comment);
    if (match.hasMatch()) {
        value = match.captured(1).trimmed();
    }
}

void DocGenerator::extractTags(const QString& comment, const QString& tag, QStringList& values) {
    QRegularExpression re(tag + "\\s+(.*?)(?=@|\\*/|$)", 
                         QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator i = re.globalMatch(comment);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        values.append(match.captured(1).trimmed());
    }
}

void DocGenerator::extractOptions(const QString& comment, QMap<QString, QString>& options) {
    QRegularExpression re(R"(@option\s+(\w+):\s*(\w+)\s*-?\s*(.*?)(?=@|\*/|$))",
                         QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator i = re.globalMatch(comment);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString key = match.captured(1);
        QString type = match.captured(2);
        QString desc = match.captured(3);
        options[key] = type + " - " + desc;
    }
}

void DocGenerator::extractDependencies(const QString& content, DocModule& module) {
    QRegularExpression includeRe(R"(include\(["']([^"']+)["']\))");
    QRegularExpressionMatchIterator i = includeRe.globalMatch(content);
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        module.dependencies.append(match.captured(1));
    }
}

void DocGenerator::traverseAST(ProgramNodePtr ast, const QString& filename) {
    if (!ast) return;
    
    // Обход деклараций
    for (const auto& decl : ast->declarations) {
        processDeclaration(decl, filename);
    }
    
    // Обход statement'ов
    for (const auto& stmt : ast->statements) {
        processStatement(stmt, filename);
    }
}

void DocGenerator::processDeclaration(DeclarationNodePtr decl, const QString& filename) {
    if (!decl) return;
    
    DocItem item;
    item.file = filename;
    item.line = decl->token.line;
    
    switch (decl->nodeType) {
        case NodeType::CLASS_DECL: {
            item.type = "class";
            item.name = decl->name;
            extractDocFromComments(decl, item);
            project.items.append(item);
            emit itemAdded(item);
            break;
        }
        
        case NodeType::FUNCTION_DECL: {
            item.type = "function";
            item.name = decl->name;
            item.returnType = decl->typeName;
            for (const auto& param : decl->parameters) {
                item.parameters.append(param.first + ": " + param.second);
            }
            extractDocFromComments(decl, item);
            project.items.append(item);
            emit itemAdded(item);
            break;
        }
        
        case NodeType::VARIABLE_DECL: {
            item.type = "variable";
            item.name = decl->name;
            item.type = decl->typeName;
            extractDocFromComments(decl, item);
            project.items.append(item);
            emit itemAdded(item);
            break;
        }
        
        default:
            break;
    }
}

void DocGenerator::processStatement(StatementNodePtr stmt, const QString& filename) {
    // Обработка statement'ов для извлечения документации
}

void DocGenerator::extractDocFromComments(DeclarationNodePtr decl, DocItem& item) {
    // Извлечение документации из комментариев перед декларацией
}

QString DocGenerator::extractNamespace(const QString& fullName) {
    int pos = fullName.lastIndexOf("::");
    if (pos != -1) {
        return fullName.left(pos);
    }
    return "";
}

QString DocGenerator::symbolTypeToString(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::FUNCTION: return "function";
        case SymbolKind::METHOD: return "method";
        case SymbolKind::CLASS: return "class";
        case SymbolKind::INTERFACE: return "interface";
        case SymbolKind::VARIABLE: return "variable";
        case SymbolKind::PARAMETER: return "parameter";
        case SymbolKind::NAMESPACE: return "namespace";
        case SymbolKind::TYPE: return "type";
        case SymbolKind::CONSTANT: return "constant";
        default: return "unknown";
    }
}

// ============================================================================
// Генерация документации
// ============================================================================

bool DocGenerator::generate() {
    emit generationStarted();
    
    QElapsedTimer timer;
    timer.start();
    
    bool success = false;
    
    switch (format) {
        case DocFormat::HTML:
            success = generateHTML();
            break;
        case DocFormat::Markdown:
            success = generateMarkdown();
            break;
        case DocFormat::PDF:
            success = generatePDF();
            break;
        case DocFormat::LaTeX:
            success = generateLaTeX();
            break;
        case DocFormat::XML:
            success = generateXML();
            break;
        case DocFormat::CHM:
            success = generateCHM();
            break;
        case DocFormat::ePub:
            success = generateEPub();
            break;
        default:
            success = generateHTML();
            break;
    }
    
    generationTime = timer.elapsed();
    
    emit generationCompleted(success);
    
    if (success) {
        LOG_INFO("Documentation generated successfully in " + 
                 QString::number(generationTime) + "ms in " + outputDir.toStdString());
    } else {
        LOG_ERROR("Documentation generation failed");
    }
    
    return success;
}

bool DocGenerator::generateHTML() {
    QString outputPath = outputDir + "/index.html";
    
    if (!saveToFile(outputPath, "<!DOCTYPE html>\n<html>\n<head>\n" +
          "<meta charset=\"UTF-8\">\n" +
          "<title>" + escapeHTML(project.title) + "</title>\n" +
          generateCSS() +
          "</head>\n<body>\n" +
          generateHeader() +
          generateNavigation() +
          "<main>\n" +
          "<h1>" + escapeHTML(project.name) + "</h1>\n" +
          "<p>Version " + escapeHTML(project.version) + "</p>\n" +
          generateTableOfContents() +
          "</main>\n" +
          generateFooter() +
          generateJavaScript() +
          "</body>\n</html>")) {
        emit errorOccurred("Cannot create file: " + outputPath);
        return false;
    }
    
    // Генерация дополнительных страниц
    generateModulePages();
    generateAPIPages();
    generateExamplePages();
    
    // Копирование ресурсов
    if (!templatePath.isEmpty()) {
        copyDirectory(templatePath + "/assets", outputDir + "/assets");
    }
    
    return true;
}

bool DocGenerator::generateMarkdown() {
    QString outputPath = outputDir + "/README.md";
    
    QString content = "# " + project.name + " Documentation\n\n";
    content += "**Version:** " + project.version + "\n\n";
    content += "**Generated:** " + project.generatedAt.toString() + "\n\n";
    content += generateTableOfContents();
    
    for (const DocModule& module : project.modules) {
        content += "## " + module.name + "\n\n";
        for (const DocItem& item : module.items) {
            content += renderMarkdown(item);
        }
    }
    
    return saveToFile(outputPath, content);
}

bool DocGenerator::generatePDF() {
    // Генерация через LaTeX
    if (!generateLaTeX()) {
        return false;
    }
    
    // Конвертация LaTeX в PDF (требует pdflatex)
    LOG_WARNING("PDF generation requires pdflatex");
    return true;
}

bool DocGenerator::generateLaTeX() {
    QString outputPath = outputDir + "/documentation.tex";
    
    QString content = "\\documentclass[12pt,a4paper]{report}\n";
    content += "\\usepackage[utf8]{inputenc}\n";
    content += "\\title{" + escapeLaTeX(project.name) + "}\n";
    content += "\\author{Proxima Development Team}\n";
    content += "\\begin{document}\n";
    content += "\\maketitle\n";
    content += "\\tableofcontents\n\n";
    
    for (const DocModule& module : project.modules) {
        content += "\\chapter{" + escapeLaTeX(module.name) + "}\n\n";
        for (const DocItem& item : module.items) {
            content += renderLaTeX(item);
        }
    }
    
    content += "\\end{document}\n";
    
    return saveToFile(outputPath, content);
}

bool DocGenerator::generateXML() {
    QString outputPath = outputDir + "/documentation.xml";
    
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Cannot create file: " + outputPath);
        return false;
    }
    
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("documentation");
    xml.writeAttribute("version", project.version);
    xml.writeAttribute("generated", project.generatedAt.toString(Qt::ISODate));
    
    for (const DocModule& module : project.modules) {
        xml.writeStartElement("module");
        xml.writeAttribute("name", module.name);
        xml.writeTextElement("description", module.description);
        
        for (const DocItem& item : module.items) {
            xml.writeStartElement("item");
            xml.writeAttribute("name", item.name);
            xml.writeAttribute("type", item.type);
            xml.writeTextElement("description", item.description);
            xml.writeEndElement();
        }
        
        xml.writeEndElement();
    }
    
    xml.writeEndElement();
    xml.writeEndDocument();
    
    file.close();
    return true;
}

bool DocGenerator::generateCHM() {
    // Генерация Compiled HTML Help (требует HTML Help Workshop)
    LOG_WARNING("CHM generation requires HTML Help Workshop");
    return generateHTML();
}

bool DocGenerator::generateEPub() {
    // Генерация ePub (требует дополнительную библиотеку)
    LOG_WARNING("ePub generation requires additional library");
    return generateHTML();
}

// ============================================================================
// Рендеринг элементов
// ============================================================================

QString DocGenerator::renderHTML(const DocItem& item) {
    QString html = "<div class=\"doc-item " + item.type + "\">\n";
    html += "  <h3>" + escapeHTML(item.name) + "</h3>\n";
    html += "  <p class=\"type\">Type: " + escapeHTML(item.type) + "</p>\n";
    
    if (!item.description.isEmpty()) {
        html += "  <div class=\"description\">" + 
                processLatexFormulas(escapeHTML(item.description)) + "</div>\n";
    }
    
    if (!item.parameters.isEmpty()) {
        html += "  <h4>Parameters</h4>\n<ul>\n";
        for (const QString& param : item.parameters) {
            html += "    <li><code>" + escapeHTML(param) + "</code></li>\n";
        }
        html += "  </ul>\n";
    }
    
    if (!item.returnType.isEmpty()) {
        html += "  <h4>Returns</h4>\n<p><code>" + 
                escapeHTML(item.returnType) + "</code></p>\n";
    }
    
    if (includeExamples && !item.examples.isEmpty()) {
        html += "  <h4>Examples</h4>\n";
        for (const QString& example : item.examples) {
            html += "  <pre><code>" + escapeHTML(example) + "</code></pre>\n";
        }
    }
    
    html += "</div>\n\n";
    return html;
}

QString DocGenerator::renderMarkdown(const DocItem& item) {
    QString md = "### " + item.name + "\n\n";
    md += "**Type:** `" + item.type + "`\n\n";
    
    if (!item.description.isEmpty()) {
        md += processLatexFormulas(item.description) + "\n\n";
    }
    
    if (!item.parameters.isEmpty()) {
        md += "**Parameters:**\n\n";
        for (const QString& param : item.parameters) {
            md += "- `" + param + "`\n";
        }
        md += "\n";
    }
    
    if (!item.returnType.isEmpty()) {
        md += "**Returns:** `" + item.returnType + "`\n\n";
    }
    
    if (includeExamples && !item.examples.isEmpty()) {
        md += "**Examples:**\n\n";
        for (const QString& example : item.examples) {
            md += "```proxima\n" + example + "\n```\n\n";
        }
    }
    
    return md;
}

QString DocGenerator::renderLaTeX(const DocItem& item) {
    QString latex = "\\subsection{" + escapeLaTeX(item.name) + "}\n\n";
    latex += "\\textbf{Type:} \\texttt{" + escapeLaTeX(item.type) + "}\n\n";
    
    if (!item.description.isEmpty()) {
        latex += processLatexFormulas(item.description) + "\n\n";
    }
    
    return latex;
}

QString DocGenerator::renderXML(const DocItem& item) {
    QString xml = "<item name=\"" + escapeXML(item.name) + "\" ";
    xml += "type=\"" + escapeXML(item.type) + "\">\n";
    xml += "  <description>" + escapeXML(item.description) + "</description>\n";
    xml += "</item>\n";
    return xml;
}

// ============================================================================
// Вспомогательные функции
// ============================================================================

QString DocGenerator::generateCSS() {
    return R"(
        <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            line-height: 1.6;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        .doc-item {
            background: #f5f5f5;
            border: 1px solid #e0e0e0;
            border-radius: 5px;
            padding: 20px;
            margin-bottom: 20px;
        }
        code {
            background: #f5f5f5;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: 'Consolas', monospace;
        }
        pre {
            background: #263238;
            color: #aed581;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
        }
        </style>
    )";
}

QString DocGenerator::generateJavaScript() {
    return R"(
        <script>
        // Search functionality
        function search(query) {
            console.log('Searching for:', query);
        }
        </script>
    )";
}

QString DocGenerator::generateTableOfContents() {
    QString toc = "<nav>\n<h2>Table of Contents</h2>\n<ul>\n";
    
    for (const DocModule& module : project.modules) {
        toc += "  <li><a href=\"#module-" + module.name + "\">" + 
               escapeHTML(module.name) + "</a></li>\n";
    }
    
    toc += "</ul>\n</nav>\n";
    return toc;
}

QString DocGenerator::generateIndex() {
    QString index = "<div class=\"index\">\n<ul>\n";
    
    QVector<DocItem> sortedItems = project.items;
    std::sort(sortedItems.begin(), sortedItems.end(),
        [](const DocItem& a, const DocItem& b) {
            return a.name < b.name;
        });
    
    for (const DocItem& item : sortedItems) {
        index += "  <li><a href=\"#item-" + item.name + "\">" + 
                 escapeHTML(item.name) + "</a></li>\n";
    }
    
    index += "</ul>\n</div>\n";
    return index;
}

QString DocGenerator::generateSearchIndex() {
    return "<script>const searchIndex = [];</script>\n";
}

QString DocGenerator::generateHeader() {
    return "<header>\n<h1>" + escapeHTML(project.title) + "</h1>\n" +
           "<p>Version " + escapeHTML(project.version) + "</p>\n" +
           "</header>\n";
}

QString DocGenerator::generateFooter() {
    return "<footer>\n<p>" + project.copyright + "</p>\n" +
           "<p>Generated by Proxima DocGenerator v" + 
           project.generatorVersion + "</p>\n" +
           "</footer>\n";
}

QString DocGenerator::generateNavigation() {
    return "<nav>\n<ul>\n" +
           "  <li><a href=\"index.html\">Home</a></li>\n" +
           "  <li><a href=\"modules.html\">Modules</a></li>\n" +
           "  <li><a href=\"api.html\">API</a></li>\n" +
           "</ul>\n</nav>\n";
}

QString DocGenerator::generateSidebar() {
    return "<aside>\n<h3>Quick Links</h3>\n</aside>\n";
}

void DocGenerator::generateModulePages() {
    for (const DocModule& module : project.modules) {
        QString content = "<html><head><title>" + 
                         escapeHTML(module.name) + "</title></head><body>\n";
        content += "<h1>" + escapeHTML(module.name) + "</h1>\n";
        
        for (const DocItem& item : module.items) {
            content += renderHTML(item);
        }
        
        content += "</body></html>";
        
        saveToFile(outputDir + "/module_" + module.name + ".html", content);
    }
}

void DocGenerator::generateAPIPages() {
    QString content = "<html><head><title>API Reference</title></head><body>\n";
    content += "<h1>API Reference</h1>\n";
    
    for (const DocItem& item : project.items) {
        content += renderHTML(item);
    }
    
    content += "</body></html>";
    saveToFile(outputDir + "/api.html", content);
}

void DocGenerator::generateExamplePages() {
    if (!includeExamples) return;
    
    QString content = "<html><head><title>Examples</title></head><body>\n";
    content += "<h1>Examples</h1>\n";
    
    for (const DocModule& module : project.modules) {
        for (const DocItem& item : module.items) {
            if (!item.examples.isEmpty()) {
                content += "<h2>" + escapeHTML(item.name) + "</h2>\n";
                for (const QString& example : item.examples) {
                    content += "<pre><code>" + escapeHTML(example) + 
                              "</code></pre>\n";
                }
            }
        }
    }
    
    content += "</body></html>";
    saveToFile(outputDir + "/examples.html", content);
}

void DocGenerator::generateTutorialPages() {
    // Генерация страниц руководств
}

void DocGenerator::generateFAQPages() {
    // Генерация страниц FAQ
}

QString DocGenerator::escapeHTML(const QString& text) {
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#39;");
    return escaped;
}

QString DocGenerator::escapeXML(const QString& text) {
    return escapeHTML(text);
}

QString DocGenerator::escapeLaTeX(const QString& text) {
    QString escaped = text;
    escaped.replace("\\", "\\textbackslash{}");
    escaped.replace("{", "\\{");
    escaped.replace("}", "\\}");
    escaped.replace("$", "\\$");
    escaped.replace("#", "\\#");
    escaped.replace("&", "\\&");
    escaped.replace("%", "\\%");
    escaped.replace("_", "\\_");
    escaped.replace("^", "\\textasciicircum{}");
    escaped.replace("~", "\\textasciitilde{}");
    return escaped;
}

QString DocGenerator::escapeMarkdown(const QString& text) {
    QString escaped = text;
    escaped.replace("\\", "\\\\");
    escaped.replace("*", "\\*");
    escaped.replace("_", "\\_");
    escaped.replace("#", "\\#");
    escaped.replace("[", "\\[");
    escaped.replace("]", "\\]");
    return escaped;
}

QString DocGenerator::processLatexFormulas(const QString& text) {
    QString processed = text;
    
    // Inline формулы: $...$
    QRegularExpression inlineRe(R"(\$([^\$]+)\$)");
    processed.replace(inlineRe, "<code>\\1</code>");
    
    // Блочные формулы: $$...$$
    QRegularExpression blockRe(R"(\$\$([^\$]+)\$\$)");
    processed.replace(blockRe, "<pre><code>\\1</code></pre>");
    
    // LaTeX команды
    processed.replace("\\frac", "<span class=\"frac\">");
    processed.replace("\\sqrt", "<span class=\"sqrt\">√</span>");
    processed.replace("\\sum", "&sum;");
    processed.replace("\\int", "&int;");
    processed.replace("\\infty", "&infty;");
    processed.replace("\\pi", "&pi;");
    
    return processed;
}

QString DocGenerator::highlightCode(const QString& code, const QString& language) {
    // Подсветка синтаксиса кода
    QString highlighted = escapeHTML(code);
    
    // Простая подсветка ключевых слов Proxima
    QStringList keywords = {"if", "else", "for", "while", "return", 
                           "class", "function", "var", "const"};
    
    for (const QString& keyword : keywords) {
        QRegularExpression re("\\b" + keyword + "\\b");
        highlighted.replace(re, "<span class=\"keyword\">" + keyword + "</span>");
    }
    
    return highlighted;
}

// ============================================================================
// Статистика
// ============================================================================

int DocGenerator::getItemCount() const {
    return project.items.size();
}

int DocGenerator::getFunctionCount() const {
    int count = 0;
    for (const DocItem& item : project.items) {
        if (item.type == "function" || item.type == "method") {
            count++;
        }
    }
    return count;
}

int DocGenerator::getClassCount() const {
    int count = 0;
    for (const DocItem& item : project.items) {
        if (item.type == "class" || item.type == "interface") {
            count++;
        }
    }
    return count;
}

int DocGenerator::getVariableCount() const {
    int count = 0;
    for (const DocItem& item : project.items) {
        if (item.type == "variable") {
            count++;
        }
    }
    return count;
}

int DocGenerator::getExampleCount() const {
    int count = 0;
    for (const DocItem& item : project.items) {
        count += item.examples.size();
    }
    return count;
}

int DocGenerator::getPageCount() const {
    return project.modules.size() + 3; // modules + index + api + examples
}

int DocGenerator::getWordCount() const {
    int count = 0;
    for (const DocItem& item : project.items) {
        count += item.description.split(" ").size();
    }
    return count;
}

int DocGenerator::getCharacterCount() const {
    int count = 0;
    for (const DocItem& item : project.items) {
        count += item.description.size();
    }
    return count;
}

// ============================================================================
// Поиск и фильтрация
// ============================================================================

QVector<DocItem> DocGenerator::search(const QString& query) const {
    QVector<DocItem> results;
    QString lowerQuery = query.toLower();
    
    for (const DocItem& item : project.items) {
        bool match = item.name.toLower().contains(lowerQuery) ||
                    item.description.toLower().contains(lowerQuery) ||
                    item.type.toLower().contains(lowerQuery);
        
        if (match) {
            results.append(item);
        }
    }
    
    return results;
}

DocItem DocGenerator::getItemByName(const QString& name) const {
    for (const DocItem& item : project.items) {
        if (item.name == name) {
            return item;
        }
    }
    return DocItem();
}

QVector<DocItem> DocGenerator::getItemsByType(const QString& type) const {
    QVector<DocItem> results;
    for (const DocItem& item : project.items) {
        if (item.type == type) {
            results.append(item);
        }
    }
    return results;
}

QVector<DocItem> DocGenerator::getItemsByNamespace(const QString& ns) const {
    QVector<DocItem> results;
    for (const DocItem& item : project.items) {
        if (item.namespace_ == ns) {
            results.append(item);
        }
    }
    return results;
}

QVector<DocItem> DocGenerator::getItemsByGroup(const QString& group) const {
    QVector<DocItem> results;
    for (const DocItem& item : project.items) {
        if (item.group == group) {
            results.append(item);
        }
    }
    return results;
}

QVector<DocItem> DocGenerator::getDeprecatedItems() const {
    QVector<DocItem> results;
    for (const DocItem& item : project.items) {
        if (item.isDeprecated) {
            results.append(item);
        }
    }
    return results;
}

QVector<DocItem> DocGenerator::getPublicItems() const {
    QVector<DocItem> results;
    for (const DocItem& item : project.items) {
        if (item.isPublic) {
            results.append(item);
        }
    }
    return results;
}

QVector<DocItem> DocGenerator::getPrivateItems() const {
    QVector<DocItem> results;
    for (const DocItem& item : project.items) {
        if (!item.isPublic) {
            results.append(item);
        }
    }
    return results;
}

// ============================================================================
// Экспорт/Импорт
// ============================================================================

bool DocGenerator::exportToHTML(const QString& path) {
    return generateHTML();
}

bool DocGenerator::exportToMarkdown(const QString& path) {
    return generateMarkdown();
}

bool DocGenerator::exportToPDF(const QString& path) {
    return generatePDF();
}

bool DocGenerator::exportToXML(const QString& path) {
    return generateXML();
}

bool DocGenerator::exportToJSON(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    
    QJsonArray array;
    for (const DocItem& item : project.items) {
        QJsonObject obj;
        obj["name"] = item.name;
        obj["type"] = item.type;
        obj["description"] = item.description;
        array.append(obj);
    }
    
    file.write(QJsonDocument(array).toJson());
    file.close();
    return true;
}

bool DocGenerator::exportToCHM(const QString& path) {
    return generateCHM();
}

bool DocGenerator::exportToEPub(const QString& path) {
    return generateEPub();
}

bool DocGenerator::importFromXML(const QString& path) {
    // Импорт из XML
    return true;
}

bool DocGenerator::importFromJSON(const QString& path) {
    // Импорт из JSON
    return true;
}

// ============================================================================
// Валидация
// ============================================================================

bool DocGenerator::validateDocumentation() {
    validationErrors.clear();
    validationWarnings.clear();
    
    // Проверка обязательных полей
    for (const DocItem& item : project.items) {
        if (item.name.isEmpty()) {
            validationErrors.append("Item missing name");
        }
        if (item.type.isEmpty()) {
            validationWarnings.append("Item missing type: " + item.name);
        }
    }
    
    return validationErrors.isEmpty();
}

// ============================================================================
// Утилиты
// ============================================================================

void DocGenerator::log(int level, const QString& message) {
    if (level <= 3) {
        LOG_INFO("[DocGenerator] " + message);
    }
}

QString DocGenerator::formatTimestamp(const QDateTime& dt) const {
    return dt.toString("yyyy-MM-dd HH:mm:ss");
}

QString DocGenerator::formatFileSize(qint64 size) const {
    if (size < 1024) return QString::number(size) + " B";
    if (size < 1024 * 1024) return QString::number(size / 1024.0, 'f', 2) + " KB";
    return QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
}

QString DocGenerator::formatDuration(qint64 ms) const {
    if (ms < 1000) return QString::number(ms) + " ms";
    return QString::number(ms / 1000.0, 'f', 2) + " s";
}

QColor DocGenerator::getTypeColor(const QString& type) const {
    if (type == "class") return QColor(33, 150, 243);
    if (type == "function") return QColor(255, 152, 0);
    if (type == "variable") return QColor(156, 39, 176);
    return QColor(158, 158, 158);
}

QIcon DocGenerator::getTypeIcon(const QString& type) const {
    // Возврат иконки для типа
    return QIcon();
}

QString DocGenerator::getTemplateContent(const QString& templateName) const {
    QString path = templatePath + "/" + templateName;
    return loadFromFile(path);
}

bool DocGenerator::saveToFile(const QString& path, const QString& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;
    file.close();
    
    return true;
}

QString DocGenerator::loadFromFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    return in.readAll();
}

bool DocGenerator::createDirectory(const QString& path) {
    return QDir().mkpath(path);
}

bool DocGenerator::copyFile(const QString& source, const QString& dest) {
    return QFile::copy(source, dest);
}

bool DocGenerator::copyDirectory(const QString& source, const QString& dest) {
    QDir sourceDir(source);
    if (!sourceDir.exists()) return false;
    
    QDir().mkpath(dest);
    
    QFileInfoList files = sourceDir.entryInfoList(QDir::Files);
    for (const QFileInfo& fileInfo : files) {
        QFile::copy(fileInfo.absoluteFilePath(), 
                   dest + "/" + fileInfo.fileName());
    }
    
    return true;
}

void DocGenerator::cleanOutputDirectory() {
    QDir dir(outputDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    createDirectory(outputDir);
}

} // namespace proxima