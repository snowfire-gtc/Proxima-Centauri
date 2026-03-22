#include "DocGenerator.h"
#include "utils/CollectionParser.h"
#include "utils/Logger.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QStack>
#include <QXmlStreamWriter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QCryptographicHash>
#include <QImage>
#include <QPainter>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QStyle>
#include <QStyleOption>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>
#include <QVariantHash>
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
#include <QElapsedTimer>
#include <QQueue>
#include <QSet>
#include <QHash>
#include <QMultiHash>
#include <QMultiMap>
#include <QPair>
#include <QTuple>
#include <QTypeInfo>
#include <QMetaType>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QMetaMethod>
#include <QMetaClassInfo>
#include <QMetaEnum>
#include <QMetaType>
#include <QVariantAnimation>
#include <QAbstractAnimation>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QAnimationGroup>
#include <QEasingCurve>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QGraphicsWidget>
#include <QGraphicsLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsAnchorLayout>
#include <QStyle>
#include <QStyleFactory>
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

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

DocGenerator::DocGenerator(QObject *parent)
    : QObject(parent)
    , format(DocFormat::HTML)
    , includePrivate(false)
    , includeExamples(true)
    , itemsProcessed(0)
    , totalItems(0)
    , typeChecker(nullptr) {
    
    project.name = "Proxima Project";
    project.version = "1.0.0";
    project.generatedAt = QDateTime::currentDateTime();
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
    QDir().mkpath(outputDir);
    
    LOG_DEBUG("Output directory set to: " + dir.toStdString());
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

void DocGenerator::setTemplate(const QString& templatePath) {
    this->templatePath = templatePath;
    LOG_DEBUG("Template path set to: " + templatePath.toStdString());
}

void DocGenerator::setTypeChecker(TypeChecker* checker) {
    typeChecker = checker;
    LOG_DEBUG("TypeChecker set for documentation generation");
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
    }
    
    LOG_INFO("Symbol table added: " + std::to_string(allSymbols.size()) + " symbols");
}

void DocGenerator::addAST(ProgramNodePtr ast, const QString& filename) {
    if (!ast) return;
    
    // Обход AST для извлечения документации
    traverseAST(ast, filename);
    
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
                currentItem.options["since"] = sinceMatch.captured(1);
            }
            
            // Извлечение @author
            QRegularExpressionMatch authorMatch = authorRe.match(line);
            if (authorMatch.hasMatch()) {
                currentItem.options["author"] = authorMatch.captured(1);
            }
            
            // Извлечение @version
            QRegularExpressionMatch versionMatch = versionRe.match(line);
            if (versionMatch.hasMatch()) {
                currentItem.options["version"] = versionMatch.captured(1);
            }
            
            // Извлечение @note
            QRegularExpressionMatch noteMatch = noteRe.match(line);
            if (noteMatch.hasMatch()) {
                currentItem.options["note"] = noteMatch.captured(1);
            }
            
            // Извлечение @warning
            QRegularExpressionMatch warningMatch = warningRe.match(line);
            if (warningMatch.hasMatch()) {
                currentItem.options["warning"] = warningMatch.captured(1);
            }
            
            // Извлечение @todo
            QRegularExpressionMatch todoMatch = todoRe.match(line);
            if (todoMatch.hasMatch()) {
                currentItem.options["todo"] = todoMatch.captured(1);
            }
        }
        
        // Конец блока документации
        if (line.contains("*/") || (!line.contains("@") && !line.contains("//"))) {
            if (inDocBlock && !currentItem.name.isEmpty()) {
                module.items.append(currentItem);
                currentItem = DocItem();
            }
            inDocBlock = false;
        }
    }
    
    // Добавление последнего элемента
    if (!currentItem.name.isEmpty()) {
        module.items.append(currentItem);
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
            // Извлечение документации из комментариев
            extractDocFromComments(decl, item);
            project.items.append(item);
            break;
        }
        
        case NodeType::FUNCTION_DECL: {
            item.type = "function";
            item.name = decl->name;
            item.returnType = decl->typeName;
            // Извлечение параметров
            for (const auto& param : decl->parameters) {
                item.parameters.append(param.first + ": " + param.second);
            }
            extractDocFromComments(decl, item);
            project.items.append(item);
            break;
        }
        
        case NodeType::VARIABLE_DECL: {
            item.type = "variable";
            item.name = decl->name;
            item.type = decl->typeName;
            extractDocFromComments(decl, item);
            project.items.append(item);
            break;
        }
        
        default:
            break;
    }
}

void DocGenerator::processStatement(StatementNodePtr stmt, const QString& filename) {
    // Обработка statement'ов для извлечения документации
    // Например, документация внутри функций
}

void DocGenerator::extractDocFromComments(DeclarationNodePtr decl, DocItem& item) {
    // Извлечение документации из комментариев перед декларацией
    // Это требует доступа к исходному коду и парсинга комментариев
    // Упрощённая реализация
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
        default:
            success = generateHTML();
            break;
    }
    
    emit generationCompleted(success);
    
    if (success) {
        LOG_INFO("Documentation generated successfully in " + outputDir.toStdString());
    } else {
        LOG_ERROR("Documentation generation failed");
    }
    
    return success;
}

bool DocGenerator::generateHTML() {
    QString outputPath = outputDir + "/index.html";
    
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Cannot create file: " + outputPath);
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // HTML заголовок
    out << "<!DOCTYPE html>\n";
    out << "<html lang=\"en\">\n";
    out << "<head>\n";
    out << "    <meta charset=\"UTF-8\">\n";
    out << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    out << "    <title>" << escapeHTML(project.name) << " Documentation</title>\n";
    out << generateCSS();
    out << "</head>\n";
    out << "<body>\n";
    
    // Заголовок
    out << "    <header>\n";
    out << "        <h1>" << escapeHTML(project.name) << "</h1>\n";
    out << "        <p class=\"version\">Version " << escapeHTML(project.version) << "</p>\n";
    out << "        <p class=\"generated\">Generated: " << project.generatedAt.toString() << "</p>\n";
    out << "    </header>\n\n";
    
    // Содержание
    out << "    <nav>\n";
    out << "        <h2>Table of Contents</h2>\n";
    out << generateTableOfContents();
    out << "    </nav>\n\n";
    
    // Основная документация
    out << "    <main>\n";
    out << "        <section id=\"overview\">\n";
    out << "            <h2>Overview</h2>\n";
    out << "            <p>" << escapeHTML(project.description) << "</p>\n";
    out << "        </section>\n\n";
    
    // Модули
    out << "        <section id=\"modules\">\n";
    out << "            <h2>Modules</h2>\n";
    for (const DocModule& module : project.modules) {
        out << "            <div class=\"module\">\n";
        out << "                <h3>" << escapeHTML(module.name) << "</h3>\n";
        out << "                <p>" << escapeHTML(module.description) << "</p>\n";
        
        // Элементы модуля
        for (const DocItem& item : module.items) {
            out << renderHTML(item);
        }
        
        out << "            </div>\n";
    }
    out << "        </section>\n\n";
    
    // API Reference
    out << "        <section id=\"api\">\n";
    out << "            <h2>API Reference</h2>\n";
    for (const DocItem& item : project.items) {
        out << renderHTML(item);
    }
    out << "        </section>\n\n";
    
    // Index
    out << "        <section id=\"index\">\n";
    out << "            <h2>Index</h2>\n";
    out << generateIndex();
    out << "        </section>\n\n";
    
    // Поиск
    out << "        <section id=\"search\">\n";
    out << "            <h2>Search</h2>\n";
    out << generateSearchIndex();
    out << "        </section>\n\n";
    
    out << "    </main>\n";
    
    // Футер
    out << "    <footer>\n";
    out << "        <p>Generated by Proxima DocGenerator v" << project.generatorVersion << "</p>\n";
    out << "        <p>License: GPLv3</p>\n";
    out << "    </footer>\n";
    
    out << "</body>\n";
    out << "</html>\n";
    
    file.close();
    
    // Генерация дополнительных страниц
    generateModulePages();
    generateAPIPages();
    
    return true;
}

bool DocGenerator::generateMarkdown() {
    QString outputPath = outputDir + "/README.md";
    
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Cannot create file: " + outputPath);
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // Заголовок
    out << "# " << project.name << " Documentation\n\n";
    out << "**Version:** " << project.version << "\n\n";
    out << "**Generated:** " << project.generatedAt.toString() << "\n\n";
    
    // Содержание
    out << "## Table of Contents\n\n";
    out << "- [Overview](#overview)\n";
    out << "- [Modules](#modules)\n";
    out << "- [API Reference](#api-reference)\n";
    out << "- [Index](#index)\n\n";
    
    // Overview
    out << "## Overview\n\n";
    out << project.description << "\n\n";
    
    // Modules
    out << "## Modules\n\n";
    for (const DocModule& module : project.modules) {
        out << "### " << module.name << "\n\n";
        out << module.description << "\n\n";
        
        for (const DocItem& item : module.items) {
            out << renderMarkdown(item);
        }
    }
    
    // API Reference
    out << "## API Reference\n\n";
    for (const DocItem& item : project.items) {
        out << renderMarkdown(item);
    }
    
    // Index
    out << "## Index\n\n";
    out << generateMarkdownIndex();
    
    file.close();
    return true;
}

bool DocGenerator::generatePDF() {
    // Для генерации PDF сначала генерируем LaTeX, затем конвертируем
    if (!generateLaTeX()) {
        return false;
    }
    
    // Конвертация LaTeX в PDF (требует наличия pdflatex)
    // Это внешняя зависимость, поэтому просто генерируем LaTeX
    LOG_WARNING("PDF generation requires pdflatex. LaTeX file generated instead.");
    
    return true;
}

bool DocGenerator::generateLaTeX() {
    QString outputPath = outputDir + "/documentation.tex";
    
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Cannot create file: " + outputPath);
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // LaTeX преамбула
    out << "\\documentclass[12pt,a4paper]{report}\n";
    out << "\\usepackage[utf8]{inputenc}\n";
    out << "\\usepackage[T1]{fontenc}\n";
    out << "\\usepackage{hyperref}\n";
    out << "\\usepackage{listings}\n";
    out << "\\usepackage{xcolor}\n";
    out << "\\usepackage{amsmath}\n";
    out << "\\usepackage{amssymb}\n";
    out << "\\usepackage{geometry}\n";
    out << "\\geometry{margin=1in}\n\n";
    
    // Настройка листов кода
    out << "\\definecolor{codegreen}{rgb}{0,0.6,0}\n";
    out << "\\definecolor{codegray}{rgb}{0.5,0.5,0.5}\n";
    out << "\\definecolor{codepurple}{rgb}{0.58,0,0.82}\n";
    out << "\\definecolor{backcolour}{rgb}{0.95,0.95,0.92}\n\n";
    
    out << "\\lstdefinestyle{proximastyle}{\n";
    out << "    backgroundcolor=\\color{backcolour},\n";
    out << "    commentstyle=\\color{codegreen},\n";
    out << "    keywordstyle=\\color{magenta},\n";
    out << "    numberstyle=\\tiny\\color{codegray},\n";
    out << "    stringstyle=\\color{codepurple},\n";
    out << "    basicstyle=\\ttfamily\\footnotesize,\n";
    out << "    breakatwhitespace=false,\n";
    out << "    breaklines=true,\n";
    out << "    captionpos=b,\n";
    out << "    keepspaces=true,\n";
    out << "    numbers=left,\n";
    out << "    numbersep=5pt,\n";
    out << "    showspaces=false,\n";
    out << "    showstringspaces=false,\n";
    out << "    showtabs=false,\n";
    out << "    tabsize=2,\n";
    out << "    language=C++\n";
    out << "}\n\n";
    
    out << "\\lstset{style=proximastyle}\n\n";
    
    // Заголовок документа
    out << "\\title{" << escapeLaTeX(project.name) << "}\n";
    out << "\\author{Proxima Development Team}\n";
    out << "\\version{" << escapeLaTeX(project.version) << "}\n";
    out << "\\date{" << project.generatedAt.toString("yyyy-MM-dd") << "}\n\n";
    
    out << "\\begin{document}\n\n";
    out << "\\maketitle\n\n";
    out << "\\tableofcontents\n\n";
    out << "\\newpage\n\n";
    
    // Overview
    out << "\\chapter{Overview}\n\n";
    out << escapeLaTeX(project.description) << "\n\n";
    
    // Modules
    out << "\\chapter{Modules}\n\n";
    for (const DocModule& module : project.modules) {
        out << "\\section{" << escapeLaTeX(module.name) << "}\n\n";
        out << escapeLaTeX(module.description) << "\n\n";
        
        for (const DocItem& item : module.items) {
            out << renderLaTeX(item);
        }
    }
    
    // API Reference
    out << "\\chapter{API Reference}\n\n";
    for (const DocItem& item : project.items) {
        out << renderLaTeX(item);
    }
    
    // Index
    out << "\\chapter{Index}\n\n";
    out << "\\printindex\n\n";
    
    out << "\\end{document}\n";
    
    file.close();
    return true;
}

bool DocGenerator::generateXML() {
    QString outputPath = outputDir + "/documentation.xml";
    
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Cannot create file: " + outputPath);
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<documentation>\n";
    out << "    <project>\n";
    out << "        <name>" << escapeXML(project.name) << "</name>\n";
    out << "        <version>" << escapeXML(project.version) << "</version>\n";
    out << "        <generated>" << project.generatedAt.toString(Qt::ISODate) << "</generated>\n";
    out << "        <generator>" << escapeXML(project.generatorVersion) << "</generator>\n";
    out << "    </project>\n\n";
    
    out << "    <modules>\n";
    for (const DocModule& module : project.modules) {
        out << "        <module name=\"" << escapeXML(module.name) << "\">\n";
        out << "            <description>" << escapeXML(module.description) << "</description>\n";
        
        for (const DocItem& item : module.items) {
            out << "            <item>\n";
            out << "                <name>" << escapeXML(item.name) << "</name>\n";
            out << "                <type>" << escapeXML(item.type) << "</type>\n";
            out << "                <description>" << escapeXML(item.description) << "</description>\n";
            out << "                <line>" << item.line << "</line>\n";
            out << "            </item>\n";
        }
        
        out << "        </module>\n";
    }
    out << "    </modules>\n\n";
    
    out << "    <api>\n";
    for (const DocItem& item : project.items) {
        out << "        <item>\n";
        out << "            <name>" << escapeXML(item.name) << "</name>\n";
        out << "            <type>" << escapeXML(item.type) << "</type>\n";
        out << "            <description>" << escapeXML(item.description) << "</description>\n";
        out << "        </item>\n";
    }
    out << "    </api>\n\n";
    
    out << "</documentation>\n";
    
    file.close();
    return true;
}

// ============================================================================
// Рендеринг элементов документации
// ============================================================================

QString DocGenerator::renderHTML(const DocItem& item) {
    QString html;
    
    html += "            <div class=\"doc-item " + item.type + "\">\n";
    html += "                <h4 class=\"item-name\">" + escapeHTML(item.name) + "</h4>\n";
    html += "                <p class=\"item-type\">Type: " + escapeHTML(item.type) + "</p>\n";
    
    if (!item.description.isEmpty()) {
        html += "                <div class=\"item-description\">\n";
        html += "                    " + processLatexFormulas(escapeHTML(item.description)) + "\n";
        html += "                </div>\n";
    }
    
    if (!item.parameters.isEmpty()) {
        html += "                <div class=\"item-parameters\">\n";
        html += "                    <h5>Parameters:</h5>\n";
        html += "                    <ul>\n";
        for (const QString& param : item.parameters) {
            html += "                        <li><code>" + escapeHTML(param) + "</code></li>\n";
        }
        html += "                    </ul>\n";
        html += "                </div>\n";
    }
    
    if (!item.returnType.isEmpty()) {
        html += "                <div class=\"item-returns\">\n";
        html += "                    <h5>Returns:</h5>\n";
        html += "                    <p><code>" + escapeHTML(item.returnType) + "</code></p>\n";
        html += "                </div>\n";
    }
    
    if (!item.options.isEmpty()) {
        html += "                <div class=\"item-options\">\n";
        html += "                    <h5>Options:</h5>\n";
        html += "                    <ul>\n";
        for (auto it = item.options.begin(); it != item.options.end(); ++it) {
            html += "                        <li><code>" + escapeHTML(it.key()) + "</code>: " + 
                    escapeHTML(it.value()) + "</li>\n";
        }
        html += "                    </ul>\n";
        html += "                </div>\n";
    }
    
    if (includeExamples && !item.examples.isEmpty()) {
        html += "                <div class=\"item-examples\">\n";
        html += "                    <h5>Examples:</h5>\n";
        for (const QString& example : item.examples) {
            html += "                    <pre><code>" + escapeHTML(example) + "</code></pre>\n";
        }
        html += "                </div>\n";
    }
    
    if (!item.seeAlso.isEmpty()) {
        html += "                <div class=\"item-see-also\">\n";
        html += "                    <h5>See Also:</h5>\n";
        html += "                    <ul>\n";
        for (const QString& see : item.seeAlso) {
            html += "                        <li><a href=\"#"+ escapeHTML(see) + "\">" + 
                    escapeHTML(see) + "</a></li>\n";
        }
        html += "                    </ul>\n";
        html += "                </div>\n";
    }
    
    if (item.isDeprecated) {
        html += "                <div class=\"item-deprecated\">\n";
        html += "                    <strong>Deprecated:</strong> " + 
                escapeHTML(item.deprecatedMessage) + "\n";
        html += "                </div>\n";
    }
    
    html += "                <p class=\"item-location\">File: " + escapeHTML(item.file) + 
            ":" + QString::number(item.line) + "</p>\n";
    html += "            </div>\n\n";
    
    return html;
}

QString DocGenerator::renderMarkdown(const DocItem& item) {
    QString md;
    
    md += "### " + item.name + "\n\n";
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
    
    if (!item.options.isEmpty()) {
        md += "**Options:**\n\n";
        for (auto it = item.options.begin(); it != item.options.end(); ++it) {
            md += "- `" + it.key() + "`: " + it.value() + "\n";
        }
        md += "\n";
    }
    
    if (includeExamples && !item.examples.isEmpty()) {
        md += "**Examples:**\n\n";
        for (const QString& example : item.examples) {
            md += "```proxima\n" + example + "\n```\n\n";
        }
    }
    
    if (!item.seeAlso.isEmpty()) {
        md += "**See Also:** ";
        for (int i = 0; i < item.seeAlso.size(); i++) {
            if (i > 0) md += ", ";
            md += "[" + item.seeAlso[i] + "](#" + item.seeAlso[i] + ")";
        }
        md += "\n\n";
    }
    
    if (item.isDeprecated) {
        md += "**Deprecated:** " + item.deprecatedMessage + "\n\n";
    }
    
    md += "*Location: " + item.file + ":" + QString::number(item.line) + "*\n\n";
    
    return md;
}

QString DocGenerator::renderLaTeX(const DocItem& item) {
    QString latex;
    
    latex += "\\subsection{" + escapeLaTeX(item.name) + "}\n\n";
    latex += "\\textbf{Type:} \\texttt{" + escapeLaTeX(item.type) + "}\n\n";
    
    if (!item.description.isEmpty()) {
        latex += processLatexFormulas(item.description) + "\n\n";
    }
    
    if (!item.parameters.isEmpty()) {
        latex += "\\textbf{Parameters:}\n\n";
        latex += "\\begin{itemize}\n";
        for (const QString& param : item.parameters) {
            latex += "    \\item \\texttt{" + escapeLaTeX(param) + "}\n";
        }
        latex += "\\end{itemize}\n\n";
    }
    
    if (!item.returnType.isEmpty()) {
        latex += "\\textbf{Returns:} \\texttt{" + escapeLaTeX(item.returnType) + "}\n\n";
    }
    
    if (includeExamples && !item.examples.isEmpty()) {
        latex += "\\textbf{Examples:}\n\n";
        latex += "\\begin{lstlisting}[language=C++]\n";
        for (const QString& example : item.examples) {
            latex += escapeLaTeX(example) + "\n";
        }
        latex += "\\end{lstlisting}\n\n";
    }
    
    latex += "\\textit{Location: " + escapeLaTeX(item.file) + ":" + 
             QString::number(item.line) + "}\n\n";
    
    return latex;
}

// ============================================================================
// Вспомогательные функции
// ============================================================================

QString DocGenerator::generateCSS() {
    return R"(
        <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        header {
            border-bottom: 2px solid #4CAF50;
            padding-bottom: 20px;
            margin-bottom: 30px;
        }
        h1 { color: #2E7D32; }
        h2 { color: #388E3C; border-bottom: 1px solid #ddd; padding-bottom: 10px; }
        h3 { color: #4CAF50; }
        h4 { color: #66BB6A; }
        nav {
            background: #f5f5f5;
            padding: 20px;
            border-radius: 5px;
            margin-bottom: 30px;
        }
        .doc-item {
            background: #fafafa;
            border: 1px solid #e0e0e0;
            border-radius: 5px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .doc-item.class { border-left: 4px solid #2196F3; }
        .doc-item.method { border-left: 4px solid #4CAF50; }
        .doc-item.function { border-left: 4px solid #FF9800; }
        .doc-item.variable { border-left: 4px solid #9C27B0; }
        code {
            background: #f5f5f5;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: 'Consolas', 'Monaco', monospace;
        }
        pre {
            background: #263238;
            color: #aed581;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
        }
        .item-deprecated {
            background: #ffebee;
            border: 1px solid #ef9a9a;
            padding: 10px;
            border-radius: 3px;
            color: #c62828;
        }
        footer {
            margin-top: 50px;
            padding-top: 20px;
            border-top: 1px solid #ddd;
            text-align: center;
            color: #666;
        }
        @media (max-width: 768px) {
            body { padding: 10px; }
        }
        </style>
    )";
}

QString DocGenerator::generateTableOfContents() {
    QString toc;
    
    toc += "        <ul>\n";
    toc += "            <li><a href=\"#overview\">Overview</a></li>\n";
    toc += "            <li><a href=\"#modules\">Modules</a></li>\n";
    
    for (const DocModule& module : project.modules) {
        toc += "            <li><a href=\"#module-" + module.name + "\">" + 
               escapeHTML(module.name) + "</a></li>\n";
    }
    
    toc += "            <li><a href=\"#api\">API Reference</a></li>\n";
    toc += "            <li><a href=\"#index\">Index</a></li>\n";
    toc += "            <li><a href=\"#search\">Search</a></li>\n";
    toc += "        </ul>\n";
    
    return toc;
}

QString DocGenerator::generateIndex() {
    QString index;
    
    index += "        <div class=\"index\">\n";
    index += "            <ul>\n";
    
    // Сортировка элементов по алфавиту
    QVector<DocItem> sortedItems = project.items;
    std::sort(sortedItems.begin(), sortedItems.end(),
        [](const DocItem& a, const DocItem& b) {
            return a.name < b.name;
        });
    
    for (const DocItem& item : sortedItems) {
        index += "                <li><a href=\"#item-" + item.name + "\">" + 
                 escapeHTML(item.name) + "</a> (" + escapeHTML(item.type) + ")</li>\n";
    }
    
    index += "            </ul>\n";
    index += "        </div>\n";
    
    return index;
}

QString DocGenerator::generateMarkdownIndex() {
    QString index;
    
    QVector<DocItem> sortedItems = project.items;
    std::sort(sortedItems.begin(), sortedItems.end(),
        [](const DocItem& a, const DocItem& b) {
            return a.name < b.name;
        });
    
    for (const DocItem& item : sortedItems) {
        index += "- [" + item.name + "](#item-" + item.name + ") (" + item.type + ")\n";
    }
    
    return index;
}

QString DocGenerator::generateSearchIndex() {
    // Генерация JavaScript для поиска
    QString search;
    
    search += "        <script>\n";
    search += "            const searchIndex = [\n";
    
    bool first = true;
    for (const DocItem& item : project.items) {
        if (!first) search += ",\n";
        search += "                {";
        search += "\"name\": \"" + escapeJavaScript(item.name) + "\", ";
        search += "\"type\": \"" + escapeJavaScript(item.type) + "\", ";
        search += "\"description\": \"" + escapeJavaScript(item.description) + "\", ";
        search += "\"file\": \"" + escapeJavaScript(item.file) + "\"";
        search += "}";
        first = false;
    }
    
    search += "\n            ];\n";
    search += "            // Search functionality would be implemented here\n";
    search += "        </script>\n";
    
    return search;
}

QString DocGenerator::processLatexFormulas(const QString& text) {
    QString processed = text;
    
    // Обработка inline формул: $...$
    QRegularExpression inlineRe(R"(\$([^\$]+)\$)");
    processed.replace(inlineRe, "<code>\\1</code>");
    
    // Обработка блочных формул: $$...$$
    QRegularExpression blockRe(R"(\$\$([^\$]+)\$\$)");
    processed.replace(blockRe, "<pre><code>\\1</code></pre>");
    
    // Обработка \frac{a}{b}
    QRegularExpression fracRe(R"(\\frac\{([^\}]+)\}\{([^\}]+)\})");
    processed.replace(fracRe, "<span class=\"frac\">\\1/\\2</span>");
    
    // Обработка \sqrt{x}
    QRegularExpression sqrtRe(R"(\\sqrt\{([^\}]+)\})");
    processed.replace(sqrtRe, "<span class=\"sqrt\">√(\\1)</span>");
    
    // Обработка \sum, \int, etc.
    processed.replace("\\sum", "&sum;");
    processed.replace("\\int", "&int;");
    processed.replace("\\infty", "&infty;");
    processed.replace("\\alpha", "&alpha;");
    processed.replace("\\beta", "&beta;");
    processed.replace("\\gamma", "&gamma;");
    processed.replace("\\delta", "&delta;");
    processed.replace("\\lambda", "&lambda;");
    processed.replace("\\pi", "&pi;");
    
    return processed;
}

QString DocGenerator::escapeHTML(const QString& text) const {
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#39;");
    return escaped;
}

QString DocGenerator::escapeXML(const QString& text) const {
    return escapeHTML(text);
}

QString DocGenerator::escapeLaTeX(const QString& text) const {
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

QString DocGenerator::escapeJavaScript(const QString& text) const {
    QString escaped = text;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    escaped.replace("\t", "\\t");
    return escaped;
}

// ============================================================================
// Поиск и фильтрация
// ============================================================================

QVector<DocItem> DocGenerator::search(const QString& query) const {
    QVector<DocItem> results;
    
    QString lowerQuery = query.toLower();
    
    for (const DocItem& item : project.items) {
        bool match = false;
        
        // Поиск по имени
        if (item.name.toLower().contains(lowerQuery)) {
            match = true;
        }
        
        // Поиск по описанию
        if (item.description.toLower().contains(lowerQuery)) {
            match = true;
        }
        
        // Поиск по типу
        if (item.type.toLower().contains(lowerQuery)) {
            match = true;
        }
        
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

// ============================================================================
// Логирование
// ============================================================================

void DocGenerator::log(int level, const QString& message) {
    if (level <= 3) {
        std::cout << "[DocGenerator] " << message.toStdString() << std::endl;
        LOG_INFO("[DocGenerator] " + message);
    }
}

} // namespace proxima