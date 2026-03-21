#include "SnippetManager.h"
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSyntaxHighlighter>
#include "utils/Logger.h"

namespace proxima {

SnippetManager& SnippetManager::getInstance() {
    static SnippetManager instance;
    return instance;
}

SnippetManager::SnippetManager()
    : document(nullptr)
    , tabNavigationEnabled(true)
    , autoCloseBrackets(true)
    , placeholderTimeout(30000)  // 30 seconds timeout
    , isUpdating(false)
    , lastPlaceholderCount(0) {
    
    // Default colors
    placeholderColor = QColor(60, 60, 80);
    activePlaceholderColor = QColor(80, 80, 120);
    
    // Setup formats
    placeholderFormat.setBackground(placeholderColor);
    placeholderFormat.setForeground(QColor(200, 200, 200));
    
    activePlaceholderFormat.setBackground(activePlaceholderColor);
    activePlaceholderFormat.setForeground(QColor(255, 255, 255));
    activePlaceholderFormat.setFontWeight(QFont::Bold);
    
    // Setup timeout timer
    sessionTimeoutTimer = new QTimer(this);
    sessionTimeoutTimer->setSingleShot(true);
    connect(sessionTimeoutTimer, &QTimer::timeout, this, &SnippetManager::endSnippetSession);
    
    LOG_INFO("SnippetManager initialized");
}

SnippetManager::~SnippetManager() {
    endSnippetSession();
}

void SnippetManager::initialize() {
    loadDefaultSnippets();
    LOG_INFO("SnippetManager initialized with " + std::to_string(snippets.size()) + " snippets");
}

void SnippetManager::loadDefaultSnippets() {
    // Control flow snippets
    SnippetDefinition ifSnippet;
    ifSnippet.trigger = "if";
    ifSnippet.name = "If Statement";
    ifSnippet.description = "Conditional if statement";
    ifSnippet.content = "if ${1:condition}\n    ${2:// code}\nend";
    ifSnippet.language = "proxima";
    ifSnippet.scopes = {"source.proxima"};
    registerSnippet(ifSnippet);
    
    SnippetDefinition ifElseSnippet;
    ifElseSnippet.trigger = "ifelse";
    ifElseSnippet.name = "If-Else Statement";
    ifElseSnippet.description = "Conditional if-else statement";
    ifElseSnippet.content = "if ${1:condition}\n    ${2:// code}\nelse\n    ${3:// code}\nend";
    ifElseSnippet.language = "proxima";
    ifElseSnippet.scopes = {"source.proxima"};
    registerSnippet(ifElseSnippet);
    
    SnippetDefinition forSnippet;
    forSnippet.trigger = "for";
    forSnippet.name = "For Loop";
    forSnippet.description = "For loop over iterable";
    forSnippet.content = "for ${1:i} in ${2:range}\n    ${3:// code}\nend";
    forSnippet.language = "proxima";
    forSnippet.scopes = {"source.proxima"};
    registerSnippet(forSnippet);
    
    SnippetDefinition whileSnippet;
    whileSnippet.trigger = "while";
    whileSnippet.name = "While Loop";
    whileSnippet.description = "While loop with condition";
    whileSnippet.content = "while ${1:condition}\n    ${2:// code}\nend";
    whileSnippet.language = "proxima";
    whileSnippet.scopes = {"source.proxima"};
    registerSnippet(whileSnippet);
    
    // Function snippets
    SnippetDefinition funcSnippet;
    funcSnippet.trigger = "func";
    funcSnippet.name = "Function";
    funcSnippet.description = "Function definition";
    funcSnippet.content = "${1:name}(${2:args}) : ${3:returnType}\n    ${4:// code}\nend";
    funcSnippet.language = "proxima";
    funcSnippet.scopes = {"source.proxima"};
    registerSnippet(funcSnippet);
    
    SnippetDefinition mainSnippet;
    mainSnippet.trigger = "main";
    mainSnippet.name = "Main Function";
    mainSnippet.description = "Main entry point";
    mainSnippet.content = "main() : int32\n    ${1:// code}\n    return 0;\nend";
    mainSnippet.language = "proxima";
    mainSnippet.scopes = {"source.proxima"};
    registerSnippet(mainSnippet);
    
    // Class snippets
    SnippetDefinition classSnippet;
    classSnippet.trigger = "class";
    classSnippet.name = "Class Definition";
    classSnippet.description = "Class with constructor";
    classSnippet.content = "class ${1:ClassName}\npublic:\n    constructor()\n        ${2:// init}\n    end\n    \n    ${3:method}() : ${4:void}\n        ${5:// code}\n    end\nend";
    classSnippet.language = "proxima";
    classSnippet.scopes = {"source.proxima"};
    registerSnippet(classSnippet);
    
    // Time API snippets
    SnippetDefinition timeNowSnippet;
    timeNowSnippet.trigger = "time.now";
    timeNowSnippet.name = "Get Current Time";
    timeNowSnippet.description = "Get current system time";
    timeNowSnippet.content = "t:time = time.now();";
    timeNowSnippet.language = "proxima";
    timeNowSnippet.scopes = {"source.proxima"};
    registerSnippet(timeNowSnippet);
    
    SnippetDefinition timeSleepSnippet;
    timeSleepSnippet.trigger = "time.sleep";
    timeSleepSnippet.name = "Sleep";
    timeSleepSnippet.description = "Delay execution";
    timeSleepSnippet.content = "time.sleep(${1:milliseconds});";
    timeSleepSnippet.language = "proxima";
    timeSleepSnippet.scopes = {"source.proxima"};
    registerSnippet(timeSleepSnippet);
    
    // Print snippets
    SnippetDefinition printSnippet;
    printSnippet.trigger = "print";
    printSnippet.name = "Print to Console";
    printSnippet.description = "Print formatted output";
    printSnippet.content = "print(\"${1:format}\", ${2:args});";
    printSnippet.language = "proxima";
    printSnippet.scopes = {"source.proxima"};
    registerSnippet(printSnippet);
    
    // Matrix snippets
    SnippetDefinition matrixZerosSnippet;
    matrixZerosSnippet.trigger = "zeros";
    matrixZerosSnippet.name = "Zero Matrix";
    matrixZerosSnippet.description = "Create matrix of zeros";
    matrixZerosSnippet.content = "m:matrix<double> = zeros(${1:rows}, ${2:cols});";
    matrixZerosSnippet.language = "proxima";
    matrixZerosSnippet.scopes = {"source.proxima"};
    registerSnippet(matrixZerosSnippet);
    
    SnippetDefinition matrixRandSnippet;
    matrixRandSnippet.trigger = "rand";
    matrixRandSnippet.name = "Random Matrix";
    matrixRandSnippet.description = "Create matrix with random values";
    matrixRandSnippet.content = "m:matrix<double> = rand(${1:rows}, ${2:cols});";
    matrixRandSnippet.language = "proxima";
    matrixRandSnippet.scopes = {"source.proxima"};
    registerSnippet(matrixRandSnippet);
    
    // Region snippets
    SnippetDefinition regionSnippet;
    regionSnippet.trigger = "region";
    regionSnippet.name = "Code Region";
    regionSnippet.description = "Foldable code region";
    regionSnippet.content = "#region ${1:Region Name}\n${2:// code}\n#endregion ${1:Region Name}";
    regionSnippet.language = "proxima";
    regionSnippet.scopes = {"source.proxima"};
    registerSnippet(regionSnippet);
    
    // Comment snippets
    SnippetDefinition commentBlockSnippet;
    commentBlockSnippet.trigger = "comment";
    commentBlockSnippet.name = "Block Comment";
    commentBlockSnippet.description = "Named block comment";
    commentBlockSnippet.content = "/*${1:Name}\n${2:// comment text}\n*/${1:Name}";
    commentBlockSnippet.language = "proxima";
    commentBlockSnippet.scopes = {"source.proxima"};
    registerSnippet(commentBlockSnippet);
    
    // Debug snippets
    SnippetDefinition dbgPrintSnippet;
    dbgPrintSnippet.trigger = "dbgprint";
    dbgPrintSnippet.name = "Debug Print";
    dbgPrintSnippet.description = "Print debug message";
    dbgPrintSnippet.content = "dbgprint(\"${1:message}\", level=${2:3});";
    dbgPrintSnippet.language = "proxima";
    dbgPrintSnippet.scopes = {"source.proxima"};
    registerSnippet(dbgPrintSnippet);
    
    SnippetDefinition dbgStopSnippet;
    dbgStopSnippet.trigger = "dbgstop";
    dbgStopSnippet.name = "Debug Breakpoint";
    dbgStopSnippet.description = "Stop execution";
    dbgStopSnippet.content = "dbgstop();";
    dbgStopSnippet.language = "proxima";
    dbgStopSnippet.scopes = {"source.proxima"};
    registerSnippet(dbgStopSnippet);
    
    // Test snippets
    SnippetDefinition testSuiteSnippet;
    testSuiteSnippet.trigger = "suite";
    testSuiteSnippet.name = "Test Suite";
    testSuiteSnippet.description = "Test suite definition";
    testSuiteSnippet.content = "suite ${1:test_suite_name}\n    test ${2:test_name}() : void\n        ${3:// assertions}\n    end\nend";
    testSuiteSnippet.language = "proxima";
    testSuiteSnippet.scopes = {"source.proxima"};
    registerSnippet(testSuiteSnippet);
    
    SnippetDefinition assertSnippet;
    assertSnippet.trigger = "assert";
    assertSnippet.name = "Assertion";
    assertSnippet.description = "Test assertion";
    assertSnippet.content = "assert(${1:condition}, \"${2:message}\");";
    assertSnippet.language = "proxima";
    assertSnippet.scopes = {"source.proxima", "source.proxima.test"};
    registerSnippet(assertSnippet);
    
    // Parallel snippets
    SnippetDefinition parallelSnippet;
    parallelSnippet.trigger = "parallel";
    parallelSnippet.name = "Parallel Loop";
    parallelSnippet.description = "Parallel for loop";
    parallelSnippet.content = "parallel(threads=${1:8}, array=&${2:A}) for ${3:i} in ${4:1:1000}\n    ${5:A[i] = i;}\nend";
    parallelSnippet.language = "proxima";
    parallelSnippet.scopes = {"source.proxima"};
    registerSnippet(parallelSnippet);
    
    // Collection snippets
    SnippetDefinition collectionSnippet;
    collectionSnippet.trigger = "collection";
    collectionSnippet.name = "Collection";
    collectionSnippet.description = "Create collection with headers";
    collectionSnippet.content = "${1:name}: collection = [\n    ${2:\"value1\"}, \"value2\",,,\n    \"value3\", \"value4\"\n];\n${1:name}.header = [${3:\"Col1\"}, \"Col2\"];";
    collectionSnippet.language = "proxima";
    collectionSnippet.scopes = {"source.proxima"};
    registerSnippet(collectionSnippet);
    
    LOG_DEBUG("Loaded " + std::to_string(snippets.size()) + " default snippets");
}

void SnippetManager::loadSnippets(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_WARNING("Could not load snippets from: " + filePath.toStdString());
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        LOG_WARNING("JSON parse error in snippets file: " + error.errorString().toStdString());
        return;
    }
    
    QJsonArray array = doc.array();
    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        
        SnippetDefinition snippet;
        snippet.trigger = obj["trigger"].toString();
        snippet.name = obj["name"].toString();
        snippet.description = obj["description"].toString();
        snippet.content = obj["content"].toString();
        snippet.language = obj["language"].toString("proxima");
        
        QJsonArray scopesArray = obj["scopes"].toArray();
        for (const QJsonValue& scope : scopesArray) {
            snippet.scopes.append(scope.toString());
        }
        
        snippet.relevance = obj["relevance"].toInt(100);
        
        registerSnippet(snippet);
    }
    
    LOG_INFO("Loaded " + std::to_string(array.size()) + " snippets from file");
}

void SnippetManager::saveSnippets(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_WARNING("Could not save snippets to: " + filePath.toStdString());
        return;
    }
    
    QJsonArray array;
    for (const SnippetDefinition& snippet : snippets) {
        QJsonObject obj;
        obj["trigger"] = snippet.trigger;
        obj["name"] = snippet.name;
        obj["description"] = snippet.description;
        obj["content"] = snippet.content;
        obj["language"] = snippet.language;
        obj["relevance"] = snippet.relevance;
        
        QJsonArray scopesArray;
        for (const QString& scope : snippet.scopes) {
            scopesArray.append(scope);
        }
        obj["scopes"] = scopesArray;
        
        array.append(obj);
    }
    
    QJsonDocument doc(array);
    QTextStream out(&file);
    out << doc.toJson(QJsonDocument::Indented);
    file.close();
    
    LOG_INFO("Saved " + std::to_string(snippets.size()) + " snippets to file");
}

void SnippetManager::registerSnippet(const SnippetDefinition& snippet) {
    if (snippetsByTrigger.contains(snippet.trigger)) {
        // Update existing
        for (int i = 0; i < snippets.size(); i++) {
            if (snippets[i].trigger == snippet.trigger) {
                snippets[i] = snippet;
                break;
            }
        }
    } else {
        snippets.append(snippet);
    }
    snippetsByTrigger[snippet.trigger] = snippet;
    
    LOG_DEBUG("Registered snippet: " + snippet.trigger.toStdString());
}

void SnippetManager::unregisterSnippet(const QString& trigger) {
    snippetsByTrigger.remove(trigger);
    
    for (int i = snippets.size() - 1; i >= 0; i--) {
        if (snippets[i].trigger == trigger) {
            snippets.removeAt(i);
        }
    }
    
    LOG_DEBUG("Unregistered snippet: " + trigger.toStdString());
}

SnippetDefinition SnippetManager::getSnippet(const QString& trigger) const {
    return snippetsByTrigger.value(trigger);
}

QVector<SnippetDefinition> SnippetManager::getAllSnippets() const {
    return snippets;
}

QVector<SnippetDefinition> SnippetManager::getSnippetsByScope(const QString& scope) const {
    QVector<SnippetDefinition> result;
    for (const SnippetDefinition& snippet : snippets) {
        if (snippet.scopes.contains(scope) || snippet.scopes.contains("source.proxima")) {
            result.append(snippet);
        }
    }
    return result;
}

void SnippetManager::setDocument(QTextDocument* doc) {
    if (document) {
        disconnect(document, &QTextDocument::contentsChanged,
                  this, &SnippetManager::onDocumentContentsChanged);
    }
    
    document = doc;
    
    if (document) {
        connect(document, &QTextDocument::contentsChanged,
                this, &SnippetManager::onDocumentContentsChanged);
    }
}

void SnippetManager::startSnippetSession(QTextDocument* doc, const QString& content, int insertPosition) {
    if (!doc) return;
    
    // End any existing session
    endSnippetSession();
    
    session = SnippetSession();
    session.document = doc;
    session.snippetStart = insertPosition;
    session.snippetId = "custom";
    
    // Parse and insert snippet
    parseAndInsertSnippet(content, insertPosition);
    
    if (!session.placeholders.isEmpty()) {
        session.isActive = true;
        session.currentPlaceholderIndex = 0;
        
        // Select first placeholder
        goToPlaceholder(0);
        
        // Start timeout timer
        sessionTimeoutTimer->start(placeholderTimeout);
        
        emit sessionStarted(session);
        emit placeholderNavigated(0, session.placeholders.size());
        
        LOG_INFO("Snippet session started with " + std::to_string(session.placeholders.size()) + " placeholders");
    } else {
        // No placeholders, just insert and end session
        endSnippetSession();
    }
}

void SnippetManager::endSnippetSession() {
    if (!session.isActive) return;
    
    clearPlaceholderHighlights();
    sessionTimeoutTimer->stop();
    
    SnippetSession oldSession = session;
    session = SnippetSession();
    
    emit sessionEnded();
    emit snippetCompleted();
    
    LOG_INFO("Snippet session ended");
}

void SnippetManager::parseAndInsertSnippet(const QString& content, int position) {
    if (!document) return;
    
    isUpdating = true;
    
    QTextCursor cursor(document);
    cursor.setPosition(position);
    
    // Extract placeholders and get clean content
    session.placeholders = extractPlaceholders(content, position);
    QString cleanContent = parsePlaceholderSyntax(content);
    
    // Insert clean content
    cursor.insertText(cleanContent);
    
    // Update session boundaries
    session.snippetEnd = position + cleanContent.length();
    
    // Build linked placeholders map
    linkedPlaceholders.clear();
    for (int i = 0; i < session.placeholders.size(); i++) {
        int id = session.placeholders[i].id;
        if (!linkedPlaceholders.contains(id)) {
            linkedPlaceholders[id] = QVector<int>();
        }
        linkedPlaceholders[id].append(i);
    }
    
    // Highlight first placeholder
    if (!session.placeholders.isEmpty()) {
        highlightPlaceholder(0);
    }
    
    isUpdating = false;
}

void SnippetManager::nextPlaceholder() {
    if (!session.isActive || session.placeholders.isEmpty()) return;
    
    // Reset current placeholder
    if (session.currentPlaceholderIndex >= 0 && 
        session.currentPlaceholderIndex < session.placeholders.size()) {
        session.placeholders[session.currentPlaceholderIndex].isActive = false;
        session.placeholders[session.currentPlaceholderIndex].isVisited = true;
    }
    
    // Find next placeholder
    int nextIndex = session.currentPlaceholderIndex + 1;
    
    // Wrap around if at end
    if (nextIndex >= session.placeholders.size()) {
        nextIndex = 0;
    }
    
    // Skip already visited placeholders (except if we've wrapped)
    int startIndex = nextIndex;
    while (session.placeholders[nextIndex].isVisited && 
           session.placeholders[nextIndex].id != session.placeholders[startIndex].id) {
        nextIndex++;
        if (nextIndex >= session.placeholders.size()) {
            nextIndex = 0;
        }
        if (nextIndex == startIndex) {
            break;  // All visited
        }
    }
    
    goToPlaceholder(nextIndex);
}

void SnippetManager::previousPlaceholder() {
    if (!session.isActive || session.placeholders.isEmpty()) return;
    
    // Reset current placeholder
    if (session.currentPlaceholderIndex >= 0 && 
        session.currentPlaceholderIndex < session.placeholders.size()) {
        session.placeholders[session.currentPlaceholderIndex].isActive = false;
    }
    
    // Find previous placeholder
    int prevIndex = session.currentPlaceholderIndex - 1;
    
    // Wrap around if at beginning
    if (prevIndex < 0) {
        prevIndex = session.placeholders.size() - 1;
    }
    
    goToPlaceholder(prevIndex);
}

void SnippetManager::goToPlaceholder(int index) {
    if (!session.isActive || index < 0 || index >= session.placeholders.size()) return;
    
    // Reset previous placeholder
    if (session.currentPlaceholderIndex >= 0 && 
        session.currentPlaceholderIndex < session.placeholders.size()) {
        session.placeholders[session.currentPlaceholderIndex].isActive = false;
    }
    
    // Set new active placeholder
    session.currentPlaceholderIndex = index;
    SnippetPlaceholder& placeholder = session.placeholders[index];
    placeholder.isActive = true;
    
    // Select placeholder text
    QTextCursor cursor(document);
    cursor.setPosition(placeholder.startPosition);
    cursor.setPosition(placeholder.endPosition, QTextCursor::KeepAnchor);
    
    // Apply active format
    clearPlaceholderHighlights();
    cursor.setCharFormat(activePlaceholderFormat);
    
    // Move cursor to end of placeholder for editing
    cursor.setPosition(placeholder.endPosition);
    
    // Update document cursor
    // (would need editor integration to set cursor)
    
    highlightPlaceholder(index);
    
    emit placeholderNavigated(index, session.placeholders.size());
    
    // Reset timeout timer
    sessionTimeoutTimer->start(placeholderTimeout);
    
    LOG_DEBUG("Navigated to placeholder " + std::to_string(index + 1) + 
              " of " + std::to_string(session.placeholders.size()));
}

void SnippetManager::selectCurrentPlaceholder() {
    if (!session.isActive || session.currentPlaceholderIndex < 0) return;
    
    SnippetPlaceholder& placeholder = session.placeholders[session.currentPlaceholderIndex];
    
    QTextCursor cursor(document);
    cursor.setPosition(placeholder.startPosition);
    cursor.setPosition(placeholder.endPosition, QTextCursor::KeepAnchor);
    
    // Apply active format
    cursor.setCharFormat(activePlaceholderFormat);
}

void SnippetManager::updatePlaceholderText(int placeholderId, const QString& text) {
    // Synchronize all linked placeholders with same id
    synchronizeLinkedPlaceholders(placeholderId);
    
    emit placeholderChanged(placeholderId, text);
}

void SnippetManager::synchronizeLinkedPlaceholders(int placeholderId) {
    if (!linkedPlaceholders.contains(placeholderId)) return;
    
    QVector<int> indices = linkedPlaceholders[placeholderId];
    if (indices.size() <= 1) return;  // No linking needed
    
    // Get text from first placeholder
    QString text = getPlaceholderText(placeholderId);
    
    // Update all other placeholders with same id
    for (int i = 1; i < indices.size(); i++) {
        int idx = indices[i];
        if (idx >= 0 && idx < session.placeholders.size()) {
            SnippetPlaceholder& placeholder = session.placeholders[idx];
            
            QTextCursor cursor(document);
            cursor.setPosition(placeholder.startPosition);
            cursor.setPosition(placeholder.endPosition, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.insertText(text);
            
            // Update positions
            int lengthDiff = text.length() - (placeholder.endPosition - placeholder.startPosition);
            placeholder.endPosition = placeholder.startPosition + text.length();
            
            // Adjust positions of subsequent placeholders
            for (int j = idx + 1; j < session.placeholders.size(); j++) {
                session.placeholders[j].startPosition += lengthDiff;
                session.placeholders[j].endPosition += lengthDiff;
            }
        }
    }
    
    // Update session boundaries
    updateSessionBoundaries();
}

QString SnippetManager::getPlaceholderText(int placeholderId) const {
    for (const SnippetPlaceholder& placeholder : session.placeholders) {
        if (placeholder.id == placeholderId) {
            QTextCursor cursor(document);
            cursor.setPosition(placeholder.startPosition);
            cursor.setPosition(placeholder.endPosition, QTextCursor::KeepAnchor);
            return cursor.selectedText();
        }
    }
    return "";
}

void SnippetManager::updatePlaceholderHighlights() {
    if (!session.isActive) return;
    
    clearPlaceholderHighlights();
    
    for (int i = 0; i < session.placeholders.size(); i++) {
        if (i == session.currentPlaceholderIndex) {
            highlightPlaceholder(i);
        }
    }
}

QRect SnippetManager::getPlaceholderRect(int placeholderIndex) const {
    if (placeholderIndex < 0 || placeholderIndex >= session.placeholders.size()) {
        return QRect();
    }
    
    const SnippetPlaceholder& placeholder = session.placeholders[placeholderIndex];
    
    QTextCursor cursor(document);
    cursor.setPosition(placeholder.startPosition);
    QRect startRect = document->documentLayout()->blockBoundingRect(cursor.block()).toRect();
    
    cursor.setPosition(placeholder.endPosition);
    QRect endRect = document->documentLayout()->blockBoundingRect(cursor.block()).toRect();
    
    return startRect.united(endRect);
}

bool SnippetManager::isCursorInPlaceholder(const QTextCursor& cursor) const {
    int pos = cursor.position();
    return isPlaceholderAtPosition(pos);
}

void SnippetManager::setTabNavigation(bool enable) {
    tabNavigationEnabled = enable;
}

void SnippetManager::setPlaceholderColor(const QColor& color) {
    placeholderColor = color;
    placeholderFormat.setBackground(color);
}

void SnippetManager::setActivePlaceholderColor(const QColor& color) {
    activePlaceholderColor = color;
    activePlaceholderFormat.setBackground(color);
}

QString SnippetManager::parsePlaceholderSyntax(const QString& content) {
    QString result;
    int pos = 0;
    
    while (pos < content.length()) {
        if (content[pos] == '$') {
            if (pos + 1 < content.length() && content[pos + 1] == '{') {
                // Found placeholder start
                SnippetPlaceholder placeholder;
                QString parsed = parsePlaceholder(content, pos, placeholder);
                result += parsed;
            } else {
                result += content[pos];
                pos++;
            }
        } else if (content[pos] == '\\') {
            // Escape character
            if (pos + 1 < content.length()) {
                result += content[pos + 1];
                pos += 2;
            } else {
                result += content[pos];
                pos++;
            }
        } else {
            result += content[pos];
            pos++;
        }
    }
    
    return result;
}

QString SnippetManager::parsePlaceholder(const QString& text, int& pos, SnippetPlaceholder& placeholder) {
    // Format: ${id:default_text} or ${id} or ${id:choice|choice|choice}
    pos += 2;  // Skip ${
    
    // Parse id
    QString idStr;
    while (pos < text.length() && text[pos].isDigit()) {
        idStr += text[pos];
        pos++;
    }
    
    placeholder.id = idStr.toInt();
    
    // Check for colon (default text or choice)
    if (pos < text.length() && text[pos] == ':') {
        pos++;  // Skip colon
        
        // Check for choice (pipe-separated)
        if (pos < text.length() && text[pos] == '|') {
            // Choice placeholder
            QString choice = parseChoice(text, pos);
            placeholder.defaultText = choice;
        } else {
            // Regular default text
            QString defaultText;
            int braceCount = 1;
            
            while (pos < text.length() && braceCount > 0) {
                if (text[pos] == '{') {
                    braceCount++;
                    defaultText += text[pos];
                } else if (text[pos] == '}') {
                    braceCount--;
                    if (braceCount > 0) {
                        defaultText += text[pos];
                    }
                } else if (text[pos] == '\\') {
                    // Escape character
                    if (pos + 1 < text.length()) {
                        defaultText += text[pos + 1];
                        pos++;
                    }
                } else {
                    defaultText += text[pos];
                }
                pos++;
            }
            
            placeholder.defaultText = defaultText;
        }
    } else {
        // No default text
        placeholder.defaultText = "";
        
        // Skip to closing brace
        while (pos < text.length() && text[pos] != '}') {
            pos++;
        }
    }
    
    // Skip closing brace
    if (pos < text.length() && text[pos] == '}') {
        pos++;
    }
    
    return placeholder.defaultText;
}

QString SnippetManager::parseChoice(const QString& text, int& pos) {
    // Parse choice: |choice1|choice2|choice3|
    QStringList choices;
    QString currentChoice;
    
    while (pos < text.length() && text[pos] != '}') {
        if (text[pos] == '|') {
            choices.append(currentChoice);
            currentChoice.clear();
        } else {
            currentChoice += text[pos];
        }
        pos++;
    }
    
    if (!currentChoice.isEmpty()) {
        choices.append(currentChoice);
    }
    
    // Return first choice as default
    return choices.isEmpty() ? "" : choices[0];
}

QVector<SnippetPlaceholder> SnippetManager::extractPlaceholders(const QString& content, int basePosition) {
    QVector<SnippetPlaceholder> placeholders;
    int pos = 0;
    int currentPos = basePosition;
    
    while (pos < content.length()) {
        if (content[pos] == '$' && pos + 1 < content.length() && content[pos + 1] == '{') {
            SnippetPlaceholder placeholder;
            int startPos = pos;
            
            // Parse placeholder to get id and default text
            QString placeholderText = parsePlaceholder(content, pos, placeholder);
            
            // Calculate positions in final document
            placeholder.startPosition = currentPos;
            placeholder.endPosition = currentPos + placeholderText.length();
            placeholder.userText = placeholder.defaultText;
            
            placeholders.append(placeholder);
            
            currentPos += placeholderText.length();
        } else if (content[pos] == '\\') {
            // Escape character - skip next
            pos++;
            currentPos++;
            if (pos < content.length()) {
                pos++;
                currentPos++;
            }
        } else {
            currentPos++;
            pos++;
        }
    }
    
    return placeholders;
}

QString SnippetManager::expandSnippet(const QString& content, const QMap<int, QString>& values) {
    QString result = content;
    
    for (auto it = values.begin(); it != values.end(); ++it) {
        QString pattern = "\\$\\{" + QString::number(it.key()) + "[^}]*\\}";
        QRegularExpression re(pattern);
        result.replace(re, it.value());
    }
    
    return result;
}

void SnippetManager::highlightPlaceholder(int placeholderIndex) {
    if (placeholderIndex < 0 || placeholderIndex >= session.placeholders.size()) return;
    
    const SnippetPlaceholder& placeholder = session.placeholders[placeholderIndex];
    
    QTextCursor cursor(document);
    cursor.setPosition(placeholder.startPosition);
    cursor.setPosition(placeholder.endPosition, QTextCursor::KeepAnchor);
    
    if (placeholderIndex == session.currentPlaceholderIndex) {
        cursor.setCharFormat(activePlaceholderFormat);
    } else {
        cursor.setCharFormat(placeholderFormat);
    }
}

void SnippetManager::clearPlaceholderHighlights() {
    // Reset all placeholder formatting to default
    // This would need proper implementation with text formats
}

void SnippetManager::updateSessionBoundaries() {
    if (session.placeholders.isEmpty()) return;
    
    session.snippetStart = session.placeholders.first().startPosition;
    session.snippetEnd = session.placeholders.last().endPosition;
}

bool SnippetManager::isPlaceholderAtPosition(int position) const {
    for (const SnippetPlaceholder& placeholder : session.placeholders) {
        if (position >= placeholder.startPosition && position <= placeholder.endPosition) {
            return true;
        }
    }
    return false;
}

int SnippetManager::findPlaceholderAtPosition(int position) const {
    for (int i = 0; i < session.placeholders.size(); i++) {
        if (position >= session.placeholders[i].startPosition && 
            position <= session.placeholders[i].endPosition) {
            return i;
        }
    }
    return -1;
}

void SnippetManager::onDocumentContentsChanged(int position, int charsRemoved, int charsAdded) {
    if (isUpdating || !session.isActive) return;
    
    // Adjust placeholder positions based on changes
    for (SnippetPlaceholder& placeholder : session.placeholders) {
        if (position < placeholder.startPosition) {
            int delta = charsAdded - charsRemoved;
            placeholder.startPosition += delta;
            placeholder.endPosition += delta;
        }
    }
    
    updateSessionBoundaries();
}

void SnippetManager::onCursorPositionChanged() {
    if (!session.isActive) return;
    
    // Check if cursor moved outside snippet
    // Could end session if user clicks outside
}

void SnippetManager::commitPlaceholderChanges() {
    // Replace all placeholders with their final values
    for (const SnippetPlaceholder& placeholder : session.placeholders) {
        QString text = getPlaceholderText(placeholder.id);
        
        QTextCursor cursor(document);
        cursor.setPosition(placeholder.startPosition);
        cursor.setPosition(placeholder.endPosition, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertText(text);
    }
}

} // namespace proxima