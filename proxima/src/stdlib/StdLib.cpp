#include "StdLib.h"
#include <algorithm>
#include "utils/Logger.h"

namespace proxima {
namespace stdlib {

StdLibRegistry& StdLibRegistry::getInstance() {
    static StdLibRegistry instance;
    return instance;
}

StdLibRegistry::StdLibRegistry() {
    initializeStdLib();
}

void StdLibRegistry::initializeStdLib() {
    // ========================================================================
    // Класс time
    // ========================================================================
    registerClass("time",
        Time::getStaticMethods(),
        Time::getInstanceMethods(),
        Time::getProperties(),
        "Класс для работы с временем и датами. Получение текущего времени, "
        "форматирование, арифметика времени, временные зоны.");
    
    // Добавляем описания методов для time
    ClassInfo& timeInfo = classes["time"];
    timeInfo.methodDescriptions["now"] = "Статический метод. Возвращает текущее время системы. "
        "Пример: t:time = time.now()";
    timeInfo.methodDescriptions["epoch"] = "Статический метод. Возвращает timestamp в миллисекундах "
        "с начала эпохи Unix. Пример: ms:int64 = time.epoch()";
    timeInfo.methodDescriptions["sleep"] = "Статический метод. Задерживает выполнение на указанное "
        "количество миллисекунд. Пример: time.sleep(1000)";
    timeInfo.methodDescriptions["formatted"] = "Форматирует время в строку. Поддерживает спецификаторы: "
        "%H (часы), %M (минуты), %S (секунды), %d (день), %m (месяц), %Y (год). "
        "Пример: s:string = t.formatted(\"%H:%M:%S\")";
    timeInfo.methodDescriptions["add"] = "Добавляет миллисекунды к времени. Возвращает новое время. "
        "Пример: t2:time = t1.add(3600000) // +1 час";
    timeInfo.methodDescriptions["difference"] = "Вычисляет разницу в миллисекундах между двумя временами. "
        "Пример: delta:int64 = t2.difference(t1)";
    timeInfo.methodDescriptions["isPast"] = "Проверяет, является ли время в прошлом относительно текущего. "
        "Пример: if t.isPast() ... end";
    
    // ========================================================================
    // Класс file
    // ========================================================================
    registerClass("file",
        {"open", "close", "exists", "remove", "copy", "move"},
        {"read", "write", "seek", "tell", "eof", "flush", "size"},
        {"path", "mode", "isOpen"},
        "Класс для работы с файлами. Открытие, чтение, запись, управление файлами.");
    
    ClassInfo& fileInfo = classes["file"];
    fileInfo.methodDescriptions["open"] = "Открывает файл. Режимы: \"r\" (чтение), \"w\" (запись), "
        "\"a\" (добавление), \"b\" (бинарный). Пример: f.open(\"data.txt\", \"r\")";
    fileInfo.methodDescriptions["read"] = "Читает содержимое файла. Пример: content:string = f.read()";
    fileInfo.methodDescriptions["write"] = "Записывает строку в файл. Пример: f.write(\"Hello\")";
    
    // ========================================================================
    // Класс vector
    // ========================================================================
    registerClass("vector",
        {},
        {"size", "resize", "push_back", "pop_back", "clear", "empty", 
         "begin", "end", "data", "capacity"},
        {"length", "isEmpty"},
        "Одномерный массив элементов типа T. Поддержка индексации с 1, "
        "итерации, срезов через оператор :.");
    
    // ========================================================================
    // Класс matrix
    // ========================================================================
    registerClass("matrix",
        {"zeros", "ones", "eye", "rand"},
        {"rows", "cols", "size", "transpose", "det", "inv", "get", "set"},
        {"rowCount", "colCount", "isEmpty"},
        "Двумерный массив элементов типа T. Поддержка матричных операций, "
        "транспонирования ('), поэлементных операций (.+, .*, ./).");
    
    // ========================================================================
    // Класс collection
    // ========================================================================
    registerClass("collection",
        {"read", "write"},
        {"addRow", "removeRow", "getColumn", "setColumn", "sort", "filter",
         "rowCount", "colCount", "getHeader", "setHeader"},
        {"headers", "rowCount", "colCount"},
        "Таблица с заголовками столбцов. Хранение разнородных данных, "
        "чтение/запись CSV, фильтрация, сортировка.");
    
    // ========================================================================
    // Класс string
    // ========================================================================
    registerClass("string",
        {},
        {"length", "substr", "find", "replace", "split", "join", 
         "toUpperCase", "toLowerCase", "trim", "format"},
        {"length", "isEmpty"},
        "Строка символов. Поддержка Unicode, форматирования, поиска, замены.");
    
    // ========================================================================
    // Класс parallel (GPU)
    // ========================================================================
    registerClass("parallel",
        {"set", "get", "copy"},
        {"configure", "launch", "synchronize", "getMemoryInfo"},
        {"gridSize", "blockSize", "memoryUsed"},
        "Класс для параллельных вычислений на GPU (CUDA) и CPU (threads). "
        "Конфигурация ядер, передача данных хост<->device.");
    
    // ========================================================================
    // Класс rtti
    // ========================================================================
    registerClass("rtti",
        {},
        {"getName", "getFields", "getMethods", "show", "getType"},
        {"name", "type", "fields"},
        "Runtime Type Information. Получение информации о типе объекта, "
        "полях, методах. Отображение диалога настройки через show().");
    
    // ========================================================================
    // Класс dbg (отладка)
    // ========================================================================
    registerClass("dbg",
        {"stop", "print", "context", "stack"},
        {},
        {},
        "Отладочные функции. Работают только в режиме отладки. "
        "Пример: dbg.stop(), dbg.print(\"msg\", level=3)");
    
    LOG_INFO("StdLib registry initialized with " + std::to_string(classes.size()) + " classes");
}

void StdLibRegistry::registerClass(const std::string& name,
                                   const std::vector<std::string>& staticMethods,
                                   const std::vector<std::string>& instanceMethods,
                                   const std::vector<std::string>& properties,
                                   const std::string& description) {
    ClassInfo info;
    info.staticMethods = staticMethods;
    info.instanceMethods = instanceMethods;
    info.properties = properties;
    info.description = description;
    classes[name] = info;
}

std::vector<std::string> StdLibRegistry::getStaticMethods(const std::string& className) const {
    auto it = classes.find(className);
    if (it != classes.end()) {
        return it->second.staticMethods;
    }
    return {};
}

std::vector<std::string> StdLibRegistry::getInstanceMethods(const std::string& className) const {
    auto it = classes.find(className);
    if (it != classes.end()) {
        return it->second.instanceMethods;
    }
    return {};
}

std::vector<std::string> StdLibRegistry::getProperties(const std::string& className) const {
    auto it = classes.find(className);
    if (it != classes.end()) {
        return it->second.properties;
    }
    return {};
}

std::string StdLibRegistry::getClassDescription(const std::string& className) const {
    auto it = classes.find(className);
    if (it != classes.end()) {
        return it->second.description;
    }
    return "";
}

std::string StdLibRegistry::getMethodDescription(const std::string& className,
                                                 const std::string& methodName) const {
    auto it = classes.find(className);
    if (it != classes.end()) {
        auto methodIt = it->second.methodDescriptions.find(methodName);
        if (methodIt != it->second.methodDescriptions.end()) {
            return methodIt->second;
        }
    }
    return "";
}

bool StdLibRegistry::classExists(const std::string& className) const {
    return classes.find(className) != classes.end();
}

std::vector<std::string> StdLibRegistry::getAllClasses() const {
    std::vector<std::string> result;
    for (const auto& pair : classes) {
        result.push_back(pair.first);
    }
    return result;
}

} // namespace stdlib
} // namespace proxima