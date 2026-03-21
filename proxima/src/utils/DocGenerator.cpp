// ============================================================================
// Извлечение документации из комментариев перед декларацией
// ============================================================================

void DocGenerator::extractDocFromComments(DeclarationNodePtr decl, DocItem& item) {
    if (!decl) return;
    
    // Получаем доступ к исходному коду для парсинга комментариев
    // В реальной реализации нужен доступ к полному тексту файла
    QString sourceCode = getSourceCodeForFile(item.file);
    
    if (sourceCode.isEmpty()) {
        LOG_WARNING("Cannot extract comments - source code not available for: " + item.file.toStdString());
        return;
    }
    
    // Находим позицию декларации в исходном коде
    int declLine = decl->token.line;
    QStringList lines = sourceCode.split("\n");
    
    if (declLine < 1 || declLine > lines.size()) {
        return;
    }
    
    // Собираем комментарии перед декларацией
    QString docComment;
    int commentStartLine = declLine - 1;
    
    // Идём вверх по строкам в поисках комментариев
    while (commentStartLine >= 0) {
        QString line = lines[commentStartLine].trimmed();
        
        // Проверка на конец блочного комментария
        if (line.startsWith("*/")) {
            // Нашли конец блочного комментария, ищем начало
            QString blockComment = extractBlockComment(lines, commentStartLine);
            if (!blockComment.isEmpty()) {
                docComment = blockComment + "\n" + docComment;
                commentStartLine -= blockComment.count("\n") + 1;
                break;
            }
        }
        // Проверка на строковый комментарий
        else if (line.startsWith("//")) {
            docComment = line.mid(2).trimmed() + "\n" + docComment;
            commentStartLine--;
        }
        // Пустая строка - продолжаем поиск
        else if (line.isEmpty()) {
            commentStartLine--;
        }
        // Нашли код - прекращаем поиск комментариев
        else {
            break;
        }
    }
    
    // Парсим собранные комментарии
    parseDocComment(docComment, item);
    
    // Извлекаем документацию из body декларации (для функций/классов)
    extractDocFromBody(decl, item);
}

QString DocGenerator::getSourceCodeForFile(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();
    
    return content;
}

QString DocGenerator::extractBlockComment(const QStringList& lines, int endLine) {
    QString blockComment;
    int lineIndex = endLine;
    bool foundEnd = false;
    
    while (lineIndex >= 0) {
        QString line = lines[lineIndex];
        blockComment = line + "\n" + blockComment;
        
        if (line.contains("/*")) {
            foundEnd = true;
            break;
        }
        
        lineIndex--;
    }
    
    return foundEnd ? blockComment : "";
}

void DocGenerator::parseDocComment(const QString& comment, DocItem& item) {
    if (comment.isEmpty()) return;
    
    // Регулярные выражения для различных тегов документации
    QRegularExpression descriptionRe(R"((?:^|\n)\s*(?:/\*\*|//)\s*(?!@)(.*?)(?=\n\s*(?:/\*\*|//\s*@|$))");
    QRegularExpression paramRe(R"((?:/\*\*|//)\s*@param\s+(\w+)\s*:\s*(\w+)\s*-?\s*(.*?)(?=\n|$))");
    QRegularExpression returnRe(R"((?:/\*\*|//)\s*@return\s+(\w*)\s*-?\s*(.*?)(?=\n|$))");
    QRegularExpression methodRe(R"((?:/\*\*|//)\s*@method\s+(\w+))");
    QRegularExpression classRe(R"((?:/\*\*|//)\s*@class\s+(\w+))");
    QRegularExpression exampleRe(R"((?:/\*\*|//)\s*@example\s+(.*?)(?=\n\s*(?:/\*\*|//\s*@|$))");
    QRegularExpression seeAlsoRe(R"((?:/\*\*|//)\s*@see\s+(.*?)(?=\n|$))");
    QRegularExpression optionRe(R"((?:/\*\*|//)\s*@option\s+(\w+)\s*:\s*(\w+)\s*-?\s*(.*?)(?=\n|$))");
    QRegularExpression deprecatedRe(R"((?:/\*\*|//)\s*@deprecated\s*(.*?)(?=\n|$))");
    QRegularExpression sinceRe(R"((?:/\*\*|//)\s*@since\s+(.*?)(?=\n|$))");
    QRegularExpression authorRe(R"((?:/\*\*|//)\s*@author\s+(.*?)(?=\n|$))");
    QRegularExpression versionRe(R"((?:/\*\*|//)\s*@version\s+(.*?)(?=\n|$))");
    QRegularExpression noteRe(R"((?:/\*\*|//)\s*@note\s+(.*?)(?=\n|$))");
    QRegularExpression warningRe(R"((?:/\*\*|//)\s*@warning\s+(.*?)(?=\n|$))");
    QRegularExpression todoRe(R"((?:/\*\*|//)\s*@todo\s+(.*?)(?=\n|$))");
    
    // Извлечение описания
    QRegularExpressionMatchIterator descIt = descriptionRe.globalMatch(comment);
    while (descIt.hasNext()) {
        QRegularExpressionMatch match = descIt.next();
        QString desc = match.captured(1).trimmed();
        if (!desc.isEmpty()) {
            if (item.description.isEmpty()) {
                item.description = desc;
            } else {
                item.description += "\n" + desc;
            }
        }
    }
    
    // Извлечение @param
    QRegularExpressionMatchIterator paramIt = paramRe.globalMatch(comment);
    while (paramIt.hasNext()) {
        QRegularExpressionMatch match = paramIt.next();
        QString paramName = match.captured(1);
        QString paramType = match.captured(2);
        QString paramDesc = match.captured(3).trimmed();
        
        QString paramStr = paramName + ": " + paramType;
        if (!paramDesc.isEmpty()) {
            paramStr += " - " + paramDesc;
        }
        item.parameters.append(paramStr);
    }
    
    // Извлечение @return
    QRegularExpressionMatchIterator returnIt = returnRe.globalMatch(comment);
    while (returnIt.hasNext()) {
        QRegularExpressionMatch match = returnIt.next();
        if (!match.captured(1).isEmpty()) {
            item.returnType = match.captured(1).trimmed();
        }
        if (!match.captured(2).isEmpty()) {
            if (!item.description.isEmpty()) {
                item.description += "\n\n@returns: " + match.captured(2).trimmed();
            } else {
                item.description = "@returns: " + match.captured(2).trimmed();
            }
        }
    }
    
    // Извлечение @method
    QRegularExpressionMatchIterator methodIt = methodRe.globalMatch(comment);
    while (methodIt.hasNext()) {
        QRegularExpressionMatch match = methodIt.next();
        if (!match.captured(1).isEmpty()) {
            item.name = match.captured(1).trimmed();
        }
    }
    
    // Извлечение @class
    QRegularExpressionMatchIterator classIt = classRe.globalMatch(comment);
    while (classIt.hasNext()) {
        QRegularExpressionMatch match = classIt.next();
        if (!match.captured(1).isEmpty()) {
            item.name = match.captured(1).trimmed();
            item.type = "class";
        }
    }
    
    // Извлечение @example
    QRegularExpressionMatchIterator exampleIt = exampleRe.globalMatch(comment);
    while (exampleIt.hasNext()) {
        QRegularExpressionMatch match = exampleIt.next();
        QString example = match.captured(1).trimmed();
        if (!example.isEmpty()) {
            item.examples.append(example);
        }
    }
    
    // Извлечение @see
    QRegularExpressionMatchIterator seeIt = seeAlsoRe.globalMatch(comment);
    while (seeIt.hasNext()) {
        QRegularExpressionMatch match = seeIt.next();
        QString see = match.captured(1).trimmed();
        if (!see.isEmpty()) {
            item.seeAlso.append(see);
        }
    }
    
    // Извлечение @option
    QRegularExpressionMatchIterator optionIt = optionRe.globalMatch(comment);
    while (optionIt.hasNext()) {
        QRegularExpressionMatch match = optionIt.next();
        QString optName = match.captured(1);
        QString optType = match.captured(2);
        QString optDesc = match.captured(3).trimmed();
        
        QString optStr = optType;
        if (!optDesc.isEmpty()) {
            optStr += " - " + optDesc;
        }
        item.options[optName] = optStr;
    }
    
    // Извлечение @deprecated
    QRegularExpressionMatchIterator deprecatedIt = deprecatedRe.globalMatch(comment);
    while (deprecatedIt.hasNext()) {
        QRegularExpressionMatch match = deprecatedIt.next();
        item.isDeprecated = true;
        item.deprecatedMessage = match.captured(1).trimmed();
    }
    
    // Извлечение @since
    QRegularExpressionMatchIterator sinceIt = sinceRe.globalMatch(comment);
    while (sinceIt.hasNext()) {
        QRegularExpressionMatch match = sinceIt.next();
        item.options["since"] = match.captured(1).trimmed();
    }
    
    // Извлечение @author
    QRegularExpressionMatchIterator authorIt = authorRe.globalMatch(comment);
    while (authorIt.hasNext()) {
        QRegularExpressionMatch match = authorIt.next();
        item.options["author"] = match.captured(1).trimmed();
    }
    
    // Извлечение @version
    QRegularExpressionMatchIterator versionIt = versionRe.globalMatch(comment);
    while (versionIt.hasNext()) {
        QRegularExpressionMatch match = versionIt.next();
        item.options["version"] = match.captured(1).trimmed();
    }
    
    // Извлечение @note
    QRegularExpressionMatchIterator noteIt = noteRe.globalMatch(comment);
    while (noteIt.hasNext()) {
        QRegularExpressionMatch match = noteIt.next();
        item.options["note"] = match.captured(1).trimmed();
    }
    
    // Извлечение @warning
    QRegularExpressionMatchIterator warningIt = warningRe.globalMatch(comment);
    while (warningIt.hasNext()) {
        QRegularExpressionMatch match = warningIt.next();
        item.options["warning"] = match.captured(1).trimmed();
    }
    
    // Извлечение @todo
    QRegularExpressionMatchIterator todoIt = todoRe.globalMatch(comment);
    while (todoIt.hasNext()) {
        QRegularExpressionMatch match = todoIt.next();
        item.options["todo"] = match.captured(1).trimmed();
    }
}

void DocGenerator::extractDocFromBody(DeclarationNodePtr decl, DocItem& item) {
    if (!decl) return;
    
    // Для функций и классов извлекаем документацию из тела
    switch (decl->nodeType) {
        case NodeType::FUNCTION_DECL: {
            auto funcDecl = std::static_pointer_cast<FunctionDeclNode>(decl);
            
            // Извлекаем параметры из объявления функции
            for (const auto& param : funcDecl->parameters) {
                // Проверяем, есть ли уже этот параметр в документации
                bool found = false;
                for (const QString& existingParam : item.parameters) {
                    if (existingParam.startsWith(param.first + ":")) {
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    item.parameters.append(param.first + ": " + param.second);
                }
            }
            
            // Тип возврата
            if (item.returnType.isEmpty() && !funcDecl->returnType.isEmpty()) {
                item.returnType = funcDecl->returnType;
            }
            
            break;
        }
        
        case NodeType::CLASS_DECL: {
            auto classDecl = std::static_pointer_cast<ClassDeclNode>(decl);
            
            // Извлекаем поля класса
            for (const auto& field : classDecl->fields) {
                // Можно добавить документацию для полей
                item.options["field:" + field.name] = field.type;
            }
            
            break;
        }
        
        default:
            break;
    }
}

// ============================================================================
// Обработка statement'ов для извлечения документации
// ============================================================================

void DocGenerator::processStatement(StatementNodePtr stmt, const QString& filename) {
    if (!stmt) return;
    
    switch (stmt->nodeType) {
        case NodeType::BLOCK: {
            // Обработка блока statement'ов
            auto block = std::static_pointer_cast<BlockNode>(stmt);
            for (const auto& childStmt : block->statements) {
                processStatement(childStmt, filename);
            }
            break;
        }
        
        case NodeType::IF_STATEMENT: {
            // Документация для условных блоков
            auto ifStmt = std::static_pointer_cast<IfNode>(stmt);
            extractDocFromCondition(ifStmt->condition, filename, ifStmt->token.line);
            break;
        }
        
        case NodeType::FOR_LOOP: {
            // Документация для циклов
            auto forStmt = std::static_pointer_cast<ForNode>(stmt);
            extractDocFromLoop(forStmt, filename);
            break;
        }
        
        case NodeType::WHILE_LOOP: {
            // Документация для while циклов
            auto whileStmt = std::static_pointer_cast<WhileNode>(stmt);
            extractDocFromLoop(whileStmt, filename);
            break;
        }
        
        case NodeType::FUNCTION_DECL: {
            // Вложенные функции
            auto funcDecl = std::static_pointer_cast<FunctionDeclNode>(stmt);
            processDeclaration(funcDecl, filename);
            break;
        }
        
        case NodeType::CLASS_DECL: {
            // Вложенные классы
            auto classDecl = std::static_pointer_cast<ClassDeclNode>(stmt);
            processDeclaration(classDecl, filename);
            break;
        }
        
        case NodeType::EXPRESSION_STATEMENT: {
            // Выражения могут содержать документацию
            auto exprStmt = std::static_pointer_cast<ExpressionStatementNode>(stmt);
            extractDocFromExpression(exprStmt->expression, filename, stmt->token.line);
            break;
        }
        
        default:
            break;
    }
}

void DocGenerator::extractDocFromCondition(ExpressionNodePtr condition, 
                                           const QString& filename, 
                                           int line) {
    if (!condition) return;
    
    // Создаём элемент документации для условия
    DocItem item;
    item.file = filename;
    item.line = line;
    item.type = "condition";
    item.name = "Condition at line " + QString::number(line);
    item.description = "Conditional expression";
    
    // Извлекаем тип условия
    item.returnType = typeChecker.inferType(condition);
    
    project.items.append(item);
    itemsProcessed++;
    
    emit generationProgress(itemsProcessed, totalItems);
}

void DocGenerator::extractDocFromLoop(StatementNodePtr loopStmt, const QString& filename) {
    if (!loopStmt) return;
    
    DocItem item;
    item.file = filename;
    item.type = "loop";
    
    if (loopStmt->nodeType == NodeType::FOR_LOOP) {
        auto forStmt = std::static_pointer_cast<ForNode>(loopStmt);
        item.name = "For loop at line " + QString::number(forStmt->token.line);
        item.line = forStmt->token.line;
        item.description = "Iteration over iterable object";
        
        // Добавляем информацию о переменной цикла
        if (!forStmt->variable.isEmpty()) {
            item.parameters.append("iterator: " + forStmt->variable);
        }
    } else if (loopStmt->nodeType == NodeType::WHILE_LOOP) {
        auto whileStmt = std::static_pointer_cast<WhileNode>(loopStmt);
        item.name = "While loop at line " + QString::number(whileStmt->token.line);
        item.line = whileStmt->token.line;
        item.description = "Loop with precondition";
    }
    
    project.items.append(item);
    itemsProcessed++;
    
    emit generationProgress(itemsProcessed, totalItems);
}

void DocGenerator::extractDocFromExpression(ExpressionNodePtr expr, 
                                            const QString& filename, 
                                            int line) {
    if (!expr) return;
    
    // Извлекаем документацию из выражений (например, присваивания с комментариями)
    if (expr->nodeType == NodeType::BINARY_OP) {
        auto binOp = std::static_pointer_cast<BinaryOpNode>(expr);
        
        if (binOp->op == "=") {
            // Присваивание - создаём документацию для переменной
            DocItem item;
            item.file = filename;
            item.line = line;
            item.type = "variable";
            
            if (binOp->left->nodeType == NodeType::IDENTIFIER) {
                auto ident = std::static_pointer_cast<IdentifierNode>(binOp->left);
                item.name = ident->name;
                
                // Вывод типа
                item.returnType = typeChecker.inferType(binOp->right);
                
                project.items.append(item);
                itemsProcessed++;
                
                emit generationProgress(itemsProcessed, totalItems);
            }
        }
    }
}

// ============================================================================
// Обработка регионов кода (#region)
// ============================================================================

void DocGenerator::extractRegionDocumentation(const QString& content, 
                                              DocModule& module,
                                              const QString& filename) {
    QRegularExpression regionStartRe(R"(^\s*#region\s+(.+)$)");
    QRegularExpression regionEndRe(R"(^\s*#endregion\s*(.*)$)");
    
    QStringList lines = content.split("\n");
    QStack<QString> regionStack;
    QStack<int> startLines;
    
    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i].trimmed();
        
        QRegularExpressionMatch startMatch = regionStartRe.match(line);
        if (startMatch.hasMatch()) {
            regionStack.push(startMatch.captured(1));
            startLines.push(i + 1);
        }
        
        QRegularExpressionMatch endMatch = regionEndRe.match(line);
        if (endMatch.hasMatch() && !regionStack.isEmpty()) {
            QString regionName = regionStack.pop();
            int startLine = startLines.pop();
            
            // Создаём элемент документации для региона
            DocItem item;
            item.name = "#region " + regionName.trimmed();
            item.type = "region";
            item.file = filename;
            item.line = startLine;
            item.description = "Code region: " + regionName.trimmed();
            
            module.items.append(item);
        }
    }
}

// ============================================================================
// Обработка блочных комментариев с именами
// ============================================================================

void DocGenerator::extractNamedBlockComments(const QString& content, 
                                             DocModule& module,
                                             const QString& filename) {
    // Паттерн для именованных блочных комментариев: /*name ... */name
    QRegularExpression namedCommentRe(R"(/\*(\w*)\s*(.*?)\*/(\w*))");
    
    QRegularExpressionMatchIterator it = namedCommentRe.globalMatch(content);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        
        QString openName = match.captured(1);
        QString content = match.captured(2);
        QString closeName = match.captured(3);
        
        // Проверяем соответствие имён
        if (openName == closeName || openName.isEmpty() || closeName.isEmpty()) {
            DocItem item;
            item.name = "/*" + (openName.isEmpty() ? "comment" : openName) + "*/";
            item.type = "comment_block";
            item.file = filename;
            item.line = content.count("\n");  // Приблизительный номер строки
            item.description = content.trimmed();
            
            module.items.append(item);
        }
    }
}

// ============================================================================
// Обработка GEM интерфейса
// ============================================================================

void DocGenerator::extractGEMInterfaceDocumentation(DocItem& item) {
    // Стандартные методы GEM интерфейса
    if (item.type == "interface" && item.name == "gem") {
        item.description = "Generic Execution Model - стандартный интерфейс универсального объекта";
        
        // Добавляем стандартные методы GEM
        item.parameters.clear();
        item.parameters.append("init() : [] - инициализация состояния");
        item.parameters.append("reset() : [] - сброс состояния");
        item.parameters.append("update(time_current: time) : [time_next_call: time, is_alive: bool] - обновление состояния");
        item.parameters.append("show() : [] - отображение состояния");
        item.parameters.append("get_metrics() : [metrics: collection] - получение метрик");
        item.parameters.append("set_params(new_params: collection) : [] - установка параметров");
        item.parameters.append("get_params() : collection - получение параметров");
        item.parameters.append("get_name() : string - получение имени объекта");
        item.parameters.append("publish(doc: document) : [] - публикация отчёта");
        item.parameters.append("store() : collection - сериализация состояния");
        item.parameters.append("restore(state: collection) : void - восстановление состояния");
        
        // Добавляем примеры использования
        item.examples.append(R"(
my_object : MyGemClass = MyGemClass();
my_object.init();
metrics = my_object.get_metrics();
my_object.show();
)");
        
        // Ссылки на связанные документы
        item.seeAlso.append("gem_interface");
        item.seeAlso.append("object_lifecycle");
    }
}

// ============================================================================
// Обработка примеров кода из комментариев
// ============================================================================

void DocGenerator::extractCodeExamples(const QString& comment, DocItem& item) {
    // Поиск блоков кода в комментариях
    QRegularExpression codeBlockRe(R"(```(\w*)\n(.*?)```)", QRegularExpression::DotMatchesEverythingOption);
    
    QRegularExpressionMatchIterator it = codeBlockRe.globalMatch(comment);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString language = match.captured(1);
        QString code = match.captured(2).trimmed();
        
        if (!code.isEmpty()) {
            item.examples.append(code);
        }
    }
    
    // Поиск примеров в формате @example
    QRegularExpression exampleRe(R"(@example\s+(.*?)(?=@|$))", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator exampleIt = exampleRe.globalMatch(comment);
    
    while (exampleIt.hasNext()) {
        QRegularExpressionMatch match = exampleIt.next();
        QString example = match.captured(1).trimmed();
        
        if (!example.isEmpty()) {
            item.examples.append(example);
        }
    }
}

// ============================================================================
// Обработка перекрёстных ссылок
// ============================================================================

void DocGenerator::extractCrossReferences(DocItem& item) {
    // Поиск ссылок на другие элементы документации в описании
    QRegularExpression linkRe(R"(\[(.*?)\]\((.*?)\))");
    QRegularExpressionMatchIterator it = linkRe.globalMatch(item.description);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString linkText = match.captured(1);
        QString linkTarget = match.captured(2);
        
        // Добавляем в seeAlso если это не дубликат
        if (!item.seeAlso.contains(linkTarget)) {
            item.seeAlso.append(linkTarget);
        }
    }
    
    // Поиск ссылок в формате @see
    QRegularExpression seeRe(R"(@see\s+(\w+))");
    QRegularExpressionMatchIterator seeIt = seeRe.globalMatch(item.description);
    
    while (seeIt.hasNext()) {
        QRegularExpressionMatch match = seeIt.next();
        QString seeTarget = match.captured(1);
        
        if (!item.seeAlso.contains(seeTarget)) {
            item.seeAlso.append(seeTarget);
        }
    }
}

// ============================================================================
// Обработка LaTeX формул в документации
// ============================================================================

void DocGenerator::extractLatexFormulas(DocItem& item) {
    // Поиск inline формул: $...$
    QRegularExpression inlineFormulaRe(R"(\$([^\$]+)\$)");
    
    // Поиск блочных формул: $$...$$
    QRegularExpression blockFormulaRe(R"(\$\$([^\$]+)\$\$)", QRegularExpression::DotMatchesEverythingOption);
    
    // Поиск LaTeX команд
    QRegularExpression latexCommandRe(R"(\\(frac|sqrt|sum|int|infty|alpha|beta|gamma|delta|lambda|pi)\{?([^\}]*)\}?)");
    
    // Проверяем описание
    if (item.description.contains("$") || item.description.contains("\\")) {
        item.options["has_latex"] = "true";
    }
    
    // Проверяем примеры
    for (const QString& example : item.examples) {
        if (example.contains("$") || example.contains("\\")) {
            item.options["has_latex"] = "true";
            break;
        }
    }
}

// ============================================================================
// Валидация извлечённой документации
// ============================================================================

bool DocGenerator::validateDocItem(DocItem& item) {
    bool valid = true;
    
    // Проверка обязательных полей
    if (item.name.isEmpty()) {
        LOG_WARNING("Documentation item missing name");
        valid = false;
    }
    
    if (item.type.isEmpty()) {
        LOG_WARNING("Documentation item missing type: " + item.name.toStdString());
        valid = false;
    }
    
    if (item.file.isEmpty()) {
        LOG_WARNING("Documentation item missing file: " + item.name.toStdString());
        valid = false;
    }
    
    if (item.line <= 0) {
        LOG_WARNING("Documentation item has invalid line number: " + item.name.toStdString());
        valid = false;
    }
    
    // Проверка согласованности параметров
    if (!item.parameters.isEmpty()) {
        for (const QString& param : item.parameters) {
            if (!param.contains(":")) {
                LOG_WARNING("Parameter missing type: " + param.toStdString());
            }
        }
    }
    
    // Проверка deprecated
    if (item.isDeprecated && item.deprecatedMessage.isEmpty()) {
        LOG_WARNING("Deprecated item missing deprecation message: " + item.name.toStdString());
        item.deprecatedMessage = "No message provided";
    }
    
    return valid;
}

// ============================================================================
// Обновление статистики
// ============================================================================

void DocGenerator::updateStatistics() {
    totalItems = 0;
    itemsProcessed = 0;
    
    for (const DocModule& module : project.modules) {
        totalItems += module.items.size();
    }
    
    totalItems += project.items.size();
    
    LOG_INFO("Documentation statistics updated: " + std::to_string(totalItems) + " items");
}

// ============================================================================
// Интеграция с TypeChecker для получения информации о типах
// ============================================================================

void DocGenerator::setTypeChecker(TypeChecker* checker) {
    typeChecker = checker;
    LOG_DEBUG("TypeChecker set for documentation generation");
}

void DocGenerator::extractTypeInfo(DocItem& item) {
    if (!typeChecker || item.returnType.isEmpty()) {
        return;
    }
    
    TypeInfo typeInfo = typeChecker->getType(item.returnType);
    
    if (typeInfo.category != TypeCategory::UNKNOWN) {
        item.options["type_category"] = typeCategoryToString(typeInfo.category);
        item.options["type_size"] = QString::number(typeInfo.size);
        
        if (!typeInfo.elementType.isEmpty()) {
            item.options["element_type"] = typeInfo.elementType;
        }
        
        if (!typeInfo.dimensions.isEmpty()) {
            QString dims;
            for (size_t i = 0; i < typeInfo.dimensions.size(); i++) {
                if (i > 0) dims += ", ";
                dims += QString::number(typeInfo.dimensions[i]);
            }
            item.options["dimensions"] = dims;
        }
    }
}

QString DocGenerator::typeCategoryToString(TypeCategory category) const {
    switch (category) {
        case TypeCategory::SCALAR: return "scalar";
        case TypeCategory::VECTOR: return "vector";
        case TypeCategory::MATRIX: return "matrix";
        case TypeCategory::LAYER: return "layer";
        case TypeCategory::COLLECTION: return "collection";
        case TypeCategory::CLASS: return "class";
        case TypeCategory::INTERFACE: return "interface";
        case TypeCategory::FUNCTION: return "function";
        case TypeCategory::POINTER: return "pointer";
        case TypeCategory::AUTO: return "auto";
        case TypeCategory::VOID: return "void";
        default: return "unknown";
    }
}