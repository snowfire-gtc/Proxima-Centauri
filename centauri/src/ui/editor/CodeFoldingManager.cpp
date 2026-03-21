#include "CodeFoldingManager.h"
#include <QRegularExpression>
#include <QStack>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "utils/Logger.h"

namespace proxima {

CodeFoldingManager& CodeFoldingManager::getInstance() {
    static CodeFoldingManager instance;
    return instance;
}

CodeFoldingManager::CodeFoldingManager()
    : document(nullptr)
    , editor(nullptr)
    , nextRegionId(1)
    , isAnalyzing(false)
    , isUpdating(false)
    , hoverFoldLine(-1)
    , mouseOverFoldMarker(false) {
}

CodeFoldingManager::~CodeFoldingManager() {
    disconnectFromEditor();
}

void CodeFoldingManager::initialize(QTextDocument* doc) {
    document = doc;
    foldRegions.clear();
    lineToFoldRegion.clear();
    foldedLines.clear();
    nextRegionId = 1;
    
    if (document) {
        analyzeDocument();
    }
}

void CodeFoldingManager::setConfig(const FoldConfig& newConfig) {
    config = newConfig;
    if (config.enableFolding && document) {
        analyzeDocument();
    }
}

void CodeFoldingManager::analyzeDocument() {
    if (!document || !config.enableFolding || isAnalyzing) {
        return;
    }
    
    isAnalyzing = true;
    foldRegions.clear();
    lineToFoldRegion.clear();
    nextRegionId = 1;
    
    LOG_DEBUG("Analyzing document for fold regions...");
    
    // Analyze different fold types
    analyzeRegions();           // #region blocks
    analyzeMethods();           // Function/method definitions
    analyzeClasses();           // Class definitions
    analyzeInterfaces();        // Interface definitions
    analyzeNamespaces();        // Namespace blocks
    analyzeBlockComments();     /*named*/ block comments
    analyzeLoops();             // for, while, do loops
    analyzeConditionals();      // if, switch blocks
    
    // Build hierarchy
    buildFoldHierarchy();
    calculateNestingLevels();
    
    // Update visual markers
    updateFoldMarkers();
    
    LOG_INFO("Document analysis complete: " + std::to_string(foldRegions.size()) + " fold regions found");
    
    emit documentAnalyzed(foldRegions.size());
    isAnalyzing = false;
}

void CodeFoldingManager::analyzeRegions() {
    if (!config.autoFoldRegions) return;
    
    QRegularExpression regionStartRe(R"(^\s*#region\s+(.+)$)");
    QRegularExpression regionEndRe(R"(^\s*#endregion\s*(.*)$)");
    
    QStack<int> startLines;
    QStack<QString> regionNames;
    
    int blockCount = document->blockCount();
    
    for (int line = 0; line < blockCount; line++) {
        QString lineText = document->findBlockByNumber(line).text();
        
        QRegularExpressionMatch matchStart = regionStartRe.match(lineText);
        if (matchStart.hasMatch()) {
            startLines.push(line);
            regionNames.push(matchStart.captured(1).trimmed());
            continue;
        }
        
        QRegularExpressionMatch matchEnd = regionEndRe.match(lineText);
        if (matchEnd.hasMatch() && !startLines.isEmpty()) {
            int startLine = startLines.pop();
            QString name = regionNames.pop();
            
            FoldRegion region;
            region.id = nextRegionId++;
            region.type = FoldType::Region;
            region.startLine = startLine + 1;  // 1-based for user
            region.endLine = line + 1;
            region.foldLine = startLine + 1;
            region.name = name;
            region.preview = "#region " + name;
            region.isFolded = false;
            region.nestingLevel = startLines.size();
            
            foldRegions.append(region);
            
            for (int l = startLine; l <= line; l++) {
                lineToFoldRegion[l] = region.id;
            }
        }
    }
}

void CodeFoldingManager::analyzeMethods() {
    if (!config.autoFoldMethods) return;
    
    // Pattern: function_name(args) : return_type
    // End pattern: end
    QRegularExpression methodStartRe(R"(^\s*(\w+)\s*\([^)]*\)\s*:\s*\w+\s*$)");
    
    QStack<int> startLines;
    QStack<QString> methodNames;
    QStack<int> nestingLevels;
    
    int blockCount = document->blockCount();
    int currentNesting = 0;
    
    for (int line = 0; line < blockCount; line++) {
        QString lineText = document->findBlockByNumber(line).text();
        QString trimmed = lineText.trimmed();
        
        // Check for method start
        QRegularExpressionMatch matchStart = methodStartRe.match(trimmed);
        if (matchStart.hasMatch()) {
            // Make sure it's not inside a string or comment
            if (!isInCommentOrString(line, matchStart.capturedStart())) {
                startLines.push(line);
                methodNames.push(matchStart.captured(1));
                nestingLevels.push(currentNesting);
                continue;
            }
        }
        
        // Check for end keyword
        if (trimmed == "end") {
            if (!startLines.isEmpty()) {
                int startLine = startLines.pop();
                QString name = methodNames.pop();
                int startNesting = nestingLevels.pop();
                
                // Only create fold region if nesting level matches
                if (currentNesting == startNesting) {
                    FoldRegion region;
                    region.id = nextRegionId++;
                    region.type = FoldType::Method;
                    region.startLine = startLine + 1;
                    region.endLine = line + 1;
                    region.foldLine = startLine + 1;
                    region.name = name + "()";
                    region.preview = trimmed;
                    region.isFolded = false;
                    region.nestingLevel = startNesting;
                    
                    foldRegions.append(region);
                    
                    for (int l = startLine; l <= line; l++) {
                        if (!lineToFoldRegion.contains(l)) {
                            lineToFoldRegion[l] = region.id;
                        }
                    }
                }
            }
            currentNesting = qMax(0, currentNesting - 1);
        }
        
        // Track nesting (simplified - would need proper parser)
        if (trimmed.contains("class") || trimmed.contains("interface") || 
            trimmed.contains("namespace")) {
            currentNesting++;
        }
    }
}

void CodeFoldingManager::analyzeClasses() {
    if (!config.autoFoldClasses) return;
    
    QRegularExpression classStartRe(R"(^\s*(template<[^>]*>\s*)?class\s+(\w+))");
    
    QStack<int> startLines;
    QStack<QString> classNames;
    QStack<int> nestingLevels;
    
    int blockCount = document->blockCount();
    int currentNesting = 0;
    
    for (int line = 0; line < blockCount; line++) {
        QString lineText = document->findBlockByNumber(line).text();
        QString trimmed = lineText.trimmed();
        
        QRegularExpressionMatch matchStart = classStartRe.match(trimmed);
        if (matchStart.hasMatch()) {
            if (!isInCommentOrString(line, matchStart.capturedStart())) {
                startLines.push(line);
                classNames.push(matchStart.captured(2));
                nestingLevels.push(currentNesting);
                continue;
            }
        }
        
        if (trimmed == "end") {
            if (!startLines.isEmpty()) {
                int startLine = startLines.pop();
                QString name = classNames.pop();
                int startNesting = nestingLevels.pop();
                
                if (currentNesting == startNesting) {
                    FoldRegion region;
                    region.id = nextRegionId++;
                    region.type = FoldType::Class;
                    region.startLine = startLine + 1;
                    region.endLine = line + 1;
                    region.foldLine = startLine + 1;
                    region.name = "class " + name;
                    region.preview = trimmed;
                    region.isFolded = false;
                    region.nestingLevel = startNesting;
                    
                    foldRegions.append(region);
                }
            }
            currentNesting = qMax(0, currentNesting - 1);
        }
    }
}

void CodeFoldingManager::analyzeBlockComments() {
    if (!config.autoFoldComments) return;
    
    // Pattern: /*name ... */name
    QRegularExpression commentStartRe(R"(^\s*/\*(\w*)\s*$)");
    QRegularExpression commentEndRe(R"(^\s*\*/(\w*)\s*$)");
    
    QStack<int> startLines;
    QStack<QString> commentNames;
    
    int blockCount = document->blockCount();
    
    for (int line = 0; line < blockCount; line++) {
        QString lineText = document->findBlockByNumber(line).text();
        
        QRegularExpressionMatch matchStart = commentStartRe.match(lineText);
        if (matchStart.hasMatch()) {
            startLines.push(line);
            commentNames.push(matchStart.captured(1));
            continue;
        }
        
        QRegularExpressionMatch matchEnd = commentEndRe.match(lineText);
        if (matchEnd.hasMatch() && !startLines.isEmpty()) {
            QString startName = commentNames.pop();
            QString endName = matchEnd.captured(1);
            
            // Check if names match (if both provided)
            if (startName.isEmpty() || endName.isEmpty() || startName == endName) {
                int startLine = startLines.pop();
                
                FoldRegion region;
                region.id = nextRegionId++;
                region.type = FoldType::BlockComment;
                region.startLine = startLine + 1;
                region.endLine = line + 1;
                region.foldLine = startLine + 1;
                region.name = startName.isEmpty() ? "Comment Block" : "/*" + startName + "*/";
                region.preview = "/* ... */";
                region.isFolded = false;
                region.nestingLevel = startLines.size();
                
                foldRegions.append(region);
            }
        }
    }
}

void CodeFoldingManager::analyzeLoops() {
    QRegularExpression loopStartRe(R"(^\s*(for|while|do)\s*)");
    
    QStack<int> startLines;
    QStack<QString> loopTypes;
    
    int blockCount = document->blockCount();
    
    for (int line = 0; line < blockCount; line++) {
        QString lineText = document->findBlockByNumber(line).text();
        QString trimmed = lineText.trimmed();
        
        QRegularExpressionMatch matchStart = loopStartRe.match(trimmed);
        if (matchStart.hasMatch()) {
            startLines.push(line);
            loopTypes.push(matchStart.captured(1));
            continue;
        }
        
        if (trimmed == "end" && !startLines.isEmpty()) {
            int startLine = startLines.pop();
            QString loopType = loopTypes.pop();
            
            FoldRegion region;
            region.id = nextRegionId++;
            region.type = FoldType::Loop;
            region.startLine = startLine + 1;
            region.endLine = line + 1;
            region.foldLine = startLine + 1;
            region.name = loopType + " loop";
            region.preview = trimmed;
            region.isFolded = false;
            
            foldRegions.append(region);
        }
    }
}

void CodeFoldingManager::analyzeConditionals() {
    QRegularExpression ifStartRe(R"(^\s*(if|elseif|else|switch|case)\s*)");
    
    QStack<int> startLines;
    
    int blockCount = document->blockCount();
    
    for (int line = 0; line < blockCount; line++) {
        QString lineText = document->findBlockByNumber(line).text();
        QString trimmed = lineText.trimmed();
        
        QRegularExpressionMatch matchStart = ifStartRe.match(trimmed);
        if (matchStart.hasMatch()) {
            startLines.push(line);
            continue;
        }
        
        if (trimmed == "end" && !startLines.isEmpty()) {
            int startLine = startLines.pop();
            
            FoldRegion region;
            region.id = nextRegionId++;
            region.type = FoldType::Conditional;
            region.startLine = startLine + 1;
            region.endLine = line + 1;
            region.foldLine = startLine + 1;
            region.name = "Conditional block";
            region.preview = trimmed;
            region.isFolded = false;
            
            foldRegions.append(region);
        }
    }
}

void CodeFoldingManager::buildFoldHierarchy() {
    // Build parent-child relationships
    for (int i = 0; i < foldRegions.size(); i++) {
        FoldRegion& region = foldRegions[i];
        region.parentRegion = -1;
        region.childRegions.clear();
        
        // Find parent region
        for (int j = 0; j < foldRegions.size(); j++) {
            if (i == j) continue;
            
            FoldRegion& potentialParent = foldRegions[j];
            
            // Check if region is inside potentialParent
            if (region.startLine > potentialParent.startLine &&
                region.endLine < potentialParent.endLine) {
                
                // Check if this is the most immediate parent
                if (potentialParent.parentRegion == -1 ||
                    region.startLine > foldRegions[potentialParent.parentRegion].startLine) {
                    region.parentRegion = potentialParent.id;
                    potentialParent.childRegions.append(region.id);
                }
            }
        }
    }
}

void CodeFoldingManager::calculateNestingLevels() {
    for (FoldRegion& region : foldRegions) {
        int level = 0;
        int parentId = region.parentRegion;
        
        while (parentId != -1) {
            level++;
            int foundIndex = -1;
            for (int i = 0; i < foldRegions.size(); i++) {
                if (foldRegions[i].id == parentId) {
                    foundIndex = i;
                    break;
                }
            }
            if (foundIndex != -1) {
                parentId = foldRegions[foundIndex].parentRegion;
            } else {
                break;
            }
        }
        
        region.nestingLevel = level;
    }
}

void CodeFoldingManager::toggleFold(int line) {
    int regionId = lineToFoldRegion.value(line, -1);
    if (regionId != -1) {
        for (FoldRegion& region : foldRegions) {
            if (region.id == regionId && region.foldLine == line) {
                region.isFolded = !region.isFolded;
                updateFoldMarkers();
                emit foldStateChanged(regionId, region.isFolded);
                return;
            }
        }
    }
}

void CodeFoldingManager::foldRegion(int id) {
    for (FoldRegion& region : foldRegions) {
        if (region.id == id) {
            region.isFolded = true;
            updateFoldMarkers();
            emit foldStateChanged(id, true);
            return;
        }
    }
}

void CodeFoldingManager::unfoldRegion(int id) {
    for (FoldRegion& region : foldRegions) {
        if (region.id == id) {
            region.isFolded = false;
            updateFoldMarkers();
            emit foldStateChanged(id, false);
            return;
        }
    }
}

void CodeFoldingManager::foldAll() {
    for (FoldRegion& region : foldRegions) {
        region.isFolded = true;
    }
    updateFoldMarkers();
}

void CodeFoldingManager::unfoldAll() {
    for (FoldRegion& region : foldRegions) {
        region.isFolded = false;
    }
    updateFoldMarkers();
}

void CodeFoldingManager::foldLevel(int level) {
    for (FoldRegion& region : foldRegions) {
        region.isFolded = (region.nestingLevel >= level);
    }
    updateFoldMarkers();
}

void CodeFoldingManager::foldMethods() {
    for (FoldRegion& region : foldRegions) {
        if (region.type == FoldType::Method) {
            region.isFolded = true;
        }
    }
    updateFoldMarkers();
}

void CodeFoldingManager::foldClasses() {
    for (FoldRegion& region : foldRegions) {
        if (region.type == FoldType::Class) {
            region.isFolded = true;
        }
    }
    updateFoldMarkers();
}

void CodeFoldingManager::foldRegions() {
    for (FoldRegion& region : foldRegions) {
        if (region.type == FoldType::Region) {
            region.isFolded = true;
        }
    }
    updateFoldMarkers();
}

void CodeFoldingManager::foldComments() {
    for (FoldRegion& region : foldRegions) {
        if (region.type == FoldType::BlockComment) {
            region.isFolded = true;
        }
    }
    updateFoldMarkers();
}

bool CodeFoldingManager::isLineFolded(int line) const {
    return foldedLines.contains(line);
}

bool CodeFoldingManager::isRegionFolded(int id) const {
    for (const FoldRegion& region : foldRegions) {
        if (region.id == id) {
            return region.isFolded;
        }
    }
    return false;
}

FoldRegion CodeFoldingManager::getFoldRegion(int id) const {
    for (const FoldRegion& region : foldRegions) {
        if (region.id == id) {
            return region;
        }
    }
    return FoldRegion();
}

QVector<FoldRegion> CodeFoldingManager::getFoldRegionsAtLine(int line) const {
    QVector<FoldRegion> result;
    for (const FoldRegion& region : foldRegions) {
        if (line >= region.startLine && line <= region.endLine) {
            result.append(region);
        }
    }
    return result;
}

int CodeFoldingManager::getFoldedLineCount() const {
    return foldedLines.size();
}

void CodeFoldingManager::updateFoldMarkers() {
    if (isUpdating) return;
    isUpdating = true;
    
    foldedLines.clear();
    
    // Calculate which lines should be hidden
    for (const FoldRegion& region : foldRegions) {
        if (region.isFolded) {
            // Hide lines from startLine+1 to endLine-1
            for (int line = region.startLine + 1; line < region.endLine; line++) {
                // Check if this line is the fold line of another region
                bool isFoldLine = false;
                for (const FoldRegion& other : foldRegions) {
                    if (other.foldLine == line && !other.isFolded) {
                        isFoldLine = true;
                        break;
                    }
                }
                
                if (!isFoldLine) {
                    foldedLines.insert(line);
                }
            }
        }
    }
    
    // Notify editor to update visibility
    if (editor) {
        for (auto it = foldedLines.begin(); it != foldedLines.end(); ++it) {
            emit visibilityChanged(it.key(), it.key(), false);
        }
    }
    
    isUpdating = false;
}

QRect CodeFoldingManager::getFoldMarkerRect(int line) const {
    if (!editor) return QRect();
    
    QTextBlock block = editor->document()->findBlockByNumber(line - 1);
    if (!block.isValid()) return QRect();
    
    QRectF rect = editor->blockBoundingGeometry(block);
    int markerSize = 16;
    
    return QRect(5, rect.top() + (rect.height() - markerSize) / 2, markerSize, markerSize);
}

bool CodeFoldingManager::isMouseOverFoldMarker(const QPoint& pos) const {
    if (!editor) return false;
    
    int line = getLineFromMousePos(pos);
    if (line <= 0) return false;
    
    QRect markerRect = getFoldMarkerRect(line);
    return markerRect.contains(pos);
}

int CodeFoldingManager::getLineFromMousePos(const QPoint& pos) const {
    if (!editor) return -1;
    
    QTextBlock block = editor->firstVisibleBlock();
    int line = block.blockNumber() + 1;
    
    while (block.isValid()) {
        QRectF rect = editor->blockBoundingGeometry(block);
        if (rect.contains(pos)) {
            return line;
        }
        if (rect.top() > pos.y()) {
            break;
        }
        block = block.next();
        line++;
    }
    
    return -1;
}

void CodeFoldingManager::saveFoldState(const QString& filePath) {
    if (!config.preserveFoldState) return;
    
    QJsonObject state;
    QJsonArray foldedRegions;
    
    for (const FoldRegion& region : foldRegions) {
        if (region.isFolded) {
            foldedRegions.append(region.id);
        }
    }
    
    state["filePath"] = filePath;
    state["foldedRegions"] = foldedRegions;
    state["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    foldStateCache[filePath] = foldedRegions.toVariantList();
    
    LOG_DEBUG("Fold state saved for: " + filePath.toStdString());
}

void CodeFoldingManager::loadFoldState(const QString& filePath) {
    if (!config.preserveFoldState) return;
    
    auto it = foldStateCache.find(filePath);
    if (it == foldStateCache.end()) {
        return;
    }
    
    QJsonArray foldedRegions = QJsonArray::fromVariantList(it.value());
    
    for (const QJsonValue& value : foldedRegions) {
        int regionId = value.toInt();
        foldRegion(regionId);
    }
    
    LOG_DEBUG("Fold state loaded for: " + filePath.toStdString());
}

void CodeFoldingManager::clearFoldState(const QString& filePath) {
    foldStateCache.remove(filePath);
}

void CodeFoldingManager::connectToEditor(QTextEdit* edit) {
    editor = edit;
    
    if (editor) {
        connect(editor, &QTextEdit::cursorPositionChanged,
                this, [this]() {
            // Update hover state
        });
    }
}

void CodeFoldingManager::disconnectFromEditor() {
    if (editor) {
        disconnect(editor);
        editor = nullptr;
    }
}

QString CodeFoldingManager::generatePreview(const QString& text, int maxLength) const {
    QString preview = text.trimmed();
    if (preview.length() > maxLength) {
        preview = preview.left(maxLength - 3) + "...";
    }
    return preview;
}

QString CodeFoldingManager::getFoldIcon(FoldType type) const {
    switch (type) {
        case FoldType::Method: return "📝";
        case FoldType::Class: return "📦";
        case FoldType::Interface: return "🔌";
        case FoldType::Namespace: return "📁";
        case FoldType::Region: return "📑";
        case FoldType::BlockComment: return "💬";
        case FoldType::Loop