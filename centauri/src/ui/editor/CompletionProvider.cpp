#include "CompletionProvider.h"
#include <algorithm>
#include <QRegularExpression>
#include "utils/Logger.h"

namespace proxima {

CompletionProvider& CompletionProvider::getInstance() {
    static CompletionProvider instance;
    return instance;
}

CompletionProvider::CompletionProvider() {
    initializeStdLibCompletions();
    initializeKeywordCompletions();
    initializeSnippetCompletions();
}

CompletionProvider::~CompletionProvider() {}

void CompletionProvider::initializeStdLibCompletions() {
    // Инициализация информации о стандартных классах
    using namespace stdlib;
    
    auto& registry = StdLibRegistry::getInstance();
    auto classes = registry.getAllClasses();
    
    for (const auto& className : classes) {
        TypeInfo info;
        info.staticMethods = QStringList::fromVector(
            QVector<QString>::fromStdVector(registry.getStaticMethods(className)));
        info.instanceMethods = QStringList::fromVector(
            QVector<QString>::fromVector(registry.getInstanceMethods(className)));
        info.properties = QStringList::fromVector(
            QVector<QString>::fromVector(registry.getProperties(className)));
        info.description = QString::fromStdString(registry.getClassDescription(className));
        
        // Добавляем описания методов
        for (const auto& method : info.staticMethods) {
            info.methodDescriptions[method] = QString::fromStdString(
                registry.getMethodDescription(className, method.toStdString()));
        }
        for (const auto& method : info.instanceMethods) {
            info.methodDescriptions[method] = QString::fromStdString(
                registry.getMethodDescription(className, method.toStdString()));
        }
        
        typeInfo[className] = info;
        
        LOG_DEBUG("Registered class: " + className.toStdString() + 
                 " with " + QString::number(info.staticMethods.size()) + " static methods, " +
                 QString::number(info.instanceMethods.size()) + " instance methods");
    }
}

void CompletionProvider::initializeKeywordCompletions() {
    // Ключевые слова языка
    QStringList keywords = {
        "if", "elseif", "else", "end", "for", "in", "while", "do",
        "switch", "case", "default", "return", "continue", "break",
        "class", "interface", "template", "public", "protected", "private",
        "constructor", "destructor", "namespace", "using", "include",
        "auto", "void", "type", "arguments", "parallel", "true", "false",
        "null", "nan", "inf", "pi", "region", "endregion", "suite", "test",
        "assert", "gem", "int4", "int8", "int16", "int32", "int64", "int",
        "single", "double", "bool", "char", "string", "vector", "matrix",
        "layer", "collection", "point2", "point3", "point4", "time", "file"
    };
    
    for (const auto& keyword : keywords) {
        CompletionItem item;
        item.text = keyword;
        item.type = "keyword";
        item.description = "Ключевое слово языка Proxima";
        item.relevance = 100;
        keywordCompletions.append(item);
    }
}

void CompletionProvider::initializeSnippetCompletions() {
    // Сниппеты кода
    struct Snippet {
        QString trigger;
        QString content;
        QString description;
    };
    
    QVector<Snippet> snippets = {
        {"main", "main(): int32\n    ${1:// code}\n    return 0;\nend", 
                "Главная функция программы"},
        {"func", "${1:name}(${2:args}): ${3:returnType}\n    ${4:// code}\nend",
                "Объявление функции"},
        {"class", "class ${1:ClassName}\npublic:\n    constructor()\n        ${2:// init}\n    end\nend",
                "Объявление класса"},
        {"for", "for ${1:i} in ${2:range}\n    ${3:// code}\nend",
                "Цикл for"},
        {"if", "if ${1:condition}\n    ${2:// code}\nend",
                "Условие if"},
        {"print", "print(\"${1:format}\", ${2:args});",
                "Вывод в консоль"},
        {"time.now", "t:time = time.now();",
                "Получение текущего времени"},
        {"matrix.zeros", "m:matrix<double> = zeros(${1:rows}, ${2:cols});",
                "Создание нулевой матрицы"}
    };
    
    for (const auto& snippet : snippets) {
        CompletionItem item;
        item.text = snippet.trigger;
        item.type = "snippet";
        item.description = snippet.description;
        item.detail = snippet.content;
        item.relevance = 90;
        snippetCompletions.append(item);
    }
}

CompletionContext CompletionProvider::analyzeContext(const QString& text, int cursorPosition) {
    CompletionContext context;
    context.cursorPosition = cursorPosition;
    
    // Находим текущее слово
    int wordStart = cursorPosition;
    while (wordStart > 0 && 
           (text[wordStart - 1].isLetterOrNumber() || text[wordStart - 1] == '_')) {
        wordStart--;
    }
    context.currentWord = text.mid(wordStart, cursorPosition - wordStart);
    
    // Анализируем предшествующий текст
    int checkPos = wordStart - 1;
    while (checkPos >= 0 && text[checkPos].isSpace()) {
        checkPos--;
    }
    
    // Проверяем на доступ к члену (.)
    if (checkPos >= 0 && text[checkPos] == '.') {
        context.isMemberAccess = true;
        
        // Находим имя класса/объекта перед точкой
        int nameEnd = checkPos - 1;
        while (nameEnd >= 0 && text[nameEnd].isSpace()) {
            nameEnd--;
        }
        
        int nameStart = nameEnd;
        while (nameStart >= 0 && 
               (text[nameStart].isLetterOrNumber() || text[nameStart] == '_')) {
            nameStart--;
        }
        nameStart++;
        
        QString objectName = text.mid(nameStart, nameEnd - nameStart + 1);
        
        // Проверяем, начинается ли с заглавной буквы (класс)
        if (!objectName.isEmpty() && objectName[0].isUpper()) {
            context.isClassAccess = true;
            context.className = objectName;
        } else {
            context.objectType = objectName; // Нужно определить тип из контекста
        }
    }
    
    // Определяем номер строки и колонки
    context.lineNumber = text.left(cursorPosition).count('\n') + 1;
    int lastNewline = text.lastIndexOf('\n', cursorPosition - 1);
    context.columnNumber = cursorPosition - lastNewline;
    
    context.precedingText = text.left(wordStart);
    
    return context;
}

QVector<CompletionItem> CompletionProvider::getCompletions(const CompletionContext& context) {
    QVector<CompletionItem> completions;
    
    if (context.isMemberAccess || context.isClassAccess) {
        // Автодополнение после "."
        if (context.isClassAccess && typeInfo.contains(context.className)) {
            // Статические методы класса (например: time.now())
            completions += getClassMemberCompletions(context.className, true);
        } else if (typeInfo.contains(context.objectType)) {
            // Методы экземпляра (например: t.formatted())
            completions += getClassMemberCompletions(context.objectType, false);
        }
    } else {
        // Обычное автодополнение
        completions += getKeywordCompletions(context.currentWord);
        completions += getVariableCompletions(context.currentWord);
        completions += getSnippetCompletions(context.currentWord);
        
        // Добавляем имена классов
        for (auto it = typeInfo.begin(); it != typeInfo.end(); ++it) {
            CompletionItem item;
            item.text = it.key();
            item.type = "class";
            item.description = it->description;
            item.relevance = 95;
            completions.append(item);
        }
    }
    
    // Фильтрация и сортировка
    completions = filterCompletions(completions, context.currentWord);
    completions = sortCompletions(completions, context.currentWord);
    
    return completions;
}

QVector<CompletionItem> CompletionProvider::getClassMemberCompletions(
    const QString& className, bool isStatic) {
    
    QVector<CompletionItem> completions;
    
    if (!typeInfo.contains(className)) {
        return completions;
    }
    
    const TypeInfo& info = typeInfo[className];
    const QStringList& methods = isStatic ? info.staticMethods : info.instanceMethods;
    
    for (const auto& method : methods) {
        CompletionItem item;
        item.text = method;
        item.type = "method";
        item.isStatic = isStatic;
        item.signature = buildMethodSignature(method, className);
        item.description = info.methodDescriptions.value(method, "Метод класса " + className);
        item.detail = isStatic ? 
            QString("%1.%2()").arg(className).arg(method) :
            QString("instance.%1()").arg(method);
        item.relevance = 100;
        completions.append(item);
    }
    
    // Добавляем свойства (только для экземпляров)
    if (!isStatic) {
        for (const auto& prop : info.properties) {
            CompletionItem item;
            item.text = prop;
            item.type = "property";
            item.description = "Свойство класса " + className;
            item.detail = QString("%1.%2").arg(className).arg(prop);
            item.relevance = 90;
            completions.append(item);
        }
    }
    
    return completions;
}

QVector<CompletionItem> CompletionProvider::getKeywordCompletions(const QString& currentWord) {
    QVector<CompletionItem> result;
    
    for (const auto& item : keywordCompletions) {
        if (currentWord.isEmpty() || item.text.startsWith(currentWord)) {
            result.append(item);
        }
    }
    
    return result;
}

QVector<CompletionItem> CompletionProvider::getSnippetCompletions(const QString& currentWord) {
    QVector<CompletionItem> result;
    
    for (const auto& item : snippetCompletions) {
        if (currentWord.isEmpty() || item.text.startsWith(currentWord)) {
            result.append(item);
        }
    }
    
    return result;
}

QVector<CompletionItem> CompletionProvider::getVariableCompletions(const QString& currentWord) {
    // В реальной реализации здесь был бы доступ к таблице символов
    // Для примера возвращаем пустой список
    return QVector<CompletionItem>();
}

QString CompletionProvider::buildMethodSignature(const QString& methodName,
                                                 const QString& className) const {
    // В реальной реализации сигнатуры загружаются из метаданных компилятора
    // Для примера возвращаем заглушку
    
    if (className == "time") {
        if (methodName == "now") return "(): time";
        if (methodName == "epoch") return "(): int64";
        if (methodName == "sleep") return "(milliseconds: int64): void";
        if (methodName == "formatted") return "(format: string = \"\"): string";
        if (methodName == "add") return "(milliseconds: int64): time";
        if (methodName == "difference") return "(other: time): int64";
    }
    
    if (className == "file") {
        if (methodName == "open") return "(path: string, mode: string = \"r\"): bool";
        if (methodName == "read") return "(): string";
        if (methodName == "write") return "(data: string): void";
        if (methodName == "close") return "(): void";
    }
    
    return "(...): auto";
}

QString CompletionProvider::getMethodDescription(const QString& className,
                                                 const QString& methodName) const {
    if (typeInfo.contains(className)) {
        return typeInfo[className].methodDescriptions.value(methodName, "");
    }
    return "";
}

QString CompletionProvider::getLearningTip(const QString& className) const {
    if (!typeInfo.contains(className)) {
        return "";
    }
    
    const TypeInfo& info = typeInfo[className];
    
    QString tip = QString("Класс **%1**:\n\n").arg(className);
    tip += info.description + "\n\n";
    
    if (!info.staticMethods.isEmpty()) {
        tip += "**Статические методы:**\n";
        for (int i = 0; i < qMin(5, info.staticMethods.size()); i++) {
            tip += QString("- `%1()`\n").arg(info.staticMethods[i]);
        }
        if (info.staticMethods.size() > 5) {
            tip += QString("- ... и ещё %1\n").arg(info.staticMethods.size() - 5);
        }
        tip += "\n";
    }
    
    if (!info.instanceMethods.isEmpty()) {
        tip += "**Методы экземпляра:**\n";
        for (int i = 0; i < qMin(5, info.instanceMethods.size()); i++) {
            tip += QString("- `%1()`\n").arg(info.instanceMethods[i]);
        }
    }
    
    tip += "\n*Нажмите Ctrl+Space для автодополнения*";
    
    return tip;
}

QString CompletionProvider::getExampleUsage(const QString& className,
                                           const QString& methodName) const {
    if (className == "time" && methodName == "now") {
        return "// Получение текущего времени\nt:time = time.now();\nprint(\"Current time: %s\\n\", t.formatted());";
    }
    if (className == "time" && methodName == "sleep") {
        return "// Задержка на 1 секунду\ntime.sleep(1000);";
    }
    if (className == "file" && methodName == "open") {
        return "// Открытие файла\nf:file = \"data.txt\";\nf.open(\"r\");\ncontent:string = f.read();\nf.close();";
    }
    
    return "";
}

QVector<CompletionItem> CompletionProvider::filterCompletions(
    const QVector<CompletionItem>& items, const QString& filter) {
    
    if (filter.isEmpty()) {
        return items;
    }
    
    QVector<CompletionItem> filtered;
    for (const auto& item : items) {
        if (item.text.startsWith(filter, Qt::CaseInsensitive)) {
            filtered.append(item);
        }
    }
    return filtered;
}

QVector<CompletionItem> CompletionProvider::sortCompletions(
    QVector<CompletionItem>& items, const QString& currentWord) {
    
    std::sort(items.begin(), items.end(), 
        [&currentWord](const CompletionItem& a, const CompletionItem& b) {
            // Сначала по релевантности
            if (a.relevance != b.relevance) {
                return a.relevance > b.relevance;
            }
            // Затем по совпадению начала слова
            bool aStarts = a.text.startsWith(currentWord, Qt::CaseInsensitive);
            bool bStarts = b.text.startsWith(currentWord, Qt::CaseInsensitive);
            if (aStarts != bStarts) {
                return aStarts;
            }
            // Затем по алфавиту
            return a.text < b.text;
        });
    
    return items;
}

void CompletionProvider::registerUserType(const QString& typeName,
                                         const QStringList& methods,
                                         const QStringList& properties,
                                         const QString& description) {
    TypeInfo info;
    info.instanceMethods = methods;
    info.properties = properties;
    info.description = description;
    typeInfo[typeName] = info;
}

void CompletionProvider::updateTypeInfo(const QString& typeName,
                                       const QStringList& staticMethods,
                                       const QStringList& instanceMethods,
                                       const QStringList& properties) {
    if (!typeInfo.contains(typeName)) {
        TypeInfo info;
        typeInfo[typeName] = info;
    }
    
    typeInfo[typeName].staticMethods = staticMethods;
    typeInfo[typeName].instanceMethods = instanceMethods;
    typeInfo[typeName].properties = properties;
}

} // namespace proxima