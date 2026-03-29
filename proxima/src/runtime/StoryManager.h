#ifndef PROXIMA_STORYMANAGER_H
#define PROXIMA_STORYMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <ctime>
#include <filesystem>
#include "utils/CollectionParser.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Тип операции правки
 */
enum class EditOperation {
    Add,        // Добавление текста
    Remove      // Удаление текста
};

/**
 * @brief Простая структура цвета для подсветки
 */
struct SimpleColor {
    int r, g, b;
    SimpleColor(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {}
};

/**
 * @brief Запись истории правок
 * 
 * Формат хранения в .story файле:
 * [<время изменения>, <add/remove>, <текст правки>]
 */
struct StoryEntry {
    std::time_t timestamp;        // Время изменения (unix timestamp)
    EditOperation operation;    // Операция (add/remove)
    std::string text;               // Текст правки
    int startLine;              // Начальная строка
    int endLine;                // Конечная строка
    std::string checksum;           // Контрольная сумма блока
    
    StoryEntry() : operation(EditOperation::Add), startLine(0), endLine(0), timestamp(0) {}
    
    /**
     * @brief Сериализация в Collection
     * @return Collection представление
     */
    Collection toCollection() const {
        Collection entry;
        // Конвертация timestamp в строку формата dd/MM/yyyy HH:mm
        std::time_t time = timestamp;
        std::tm* tm_info = std::localtime(&time);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", tm_info);
        entry.set("timestamp", Collection::fromString(std::string(buffer)));
        
        entry.set("operation", Collection::fromString(operation == EditOperation::Add ? "add" : "remove"));
        entry.set("text", Collection::fromString(text));
        entry.set("startLine", Collection::fromNumber(startLine));
        entry.set("endLine", Collection::fromNumber(endLine));
        entry.set("checksum", Collection::fromString(checksum));
        return entry;
    }
    
    /**
     * @brief Десериализация из Collection
     * @param collection Collection данные
     */
    void fromCollection(const Collection& collection) {
        std::string timeStr = collection.get("timestamp").toString();
        std::tm tm = {};
        std::istringstream ss(timeStr);
        ss >> std::get_time(&tm, "%d/%m/%Y %H:%M");
        timestamp = std::mktime(&tm);
        
        std::string op = collection.get("operation").toString();
        operation = (op == "add") ? EditOperation::Add : EditOperation::Remove;
        text = collection.get("text").toString();
        startLine = static_cast<int>(collection.get("startLine").toNumber());
        endLine = static_cast<int>(collection.get("endLine").toNumber());
        checksum = collection.get("checksum").toString();
    }
    
    /**
     * @brief Вычисление возраста правки в минутах
     * @return Возраст в минутах
     */
    int64_t ageInMinutes() const {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        return (now_time - timestamp) / 60;
    }
    
    /**
     * @brief Получение цвета на основе возраста
     * @return Цвет для подсветки
     */
    SimpleColor getAgeColor() const;
    
    /**
     * @brief Получение строкового представления возраста
     * @return Строка с возрастом
     */
    std::string getAgeString() const;
};

/**
 * @brief Менеджер истории правок файлов
 * 
 * Хранит историю в папке .proxima/story/<filename>.story
 */
class StoryManager {
    
public:
    static StoryManager& getInstance();
    
    /**
     * @brief Инициализация менеджера
     * @param projectPath Путь к проекту
     */
    void initialize(const std::string& projectPath);
    
    /**
     * @brief Загрузка истории для файла
     * @param filePath Путь к файлу исходного кода
     * @return true если успешно
     */
    bool loadStory(const std::string& filePath);
    
    /**
     * @brief Сохранение истории для файла
     * @param filePath Путь к файлу исходного кода
     * @return true если успешно
     */
    bool saveStory(const std::string& filePath);
    
    /**
     * @brief Добавление записи в историю
     * @param filePath Путь к файлу
     * @param operation Операция
     * @param text Текст правки
     * @param startLine Начальная строка
     * @param endLine Конечная строка
     */
    void addEditEntry(const std::string& filePath, EditOperation operation, 
                     const std::string& text, int startLine, int endLine);
    
    /**
     * @brief Получение всех записей истории для файла
     * @param filePath Путь к файлу
     * @return Вектор записей истории
     */
    std::vector<StoryEntry> getStoryEntries(const std::string& filePath) const;
    
    /**
     * @brief Получение записи истории для строки
     * @param filePath Путь к файлу
     * @param line Номер строки (1-based)
     * @return Запись истории или nullptr
     */
    StoryEntry* getEntryForLine(const std::string& filePath, int line);
    
    /**
     * @brief Получение информации о возрасте для всех строк
     * @param filePath Путь к файлу
     * @return Карта: номер строки -> время изменения
     */
    std::map<int, std::time_t> getAgeInfo(const std::string& filePath);
    
    /**
     * @brief Отмена последней правки
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool undoLastEdit(const std::string& filePath);
    
    /**
     * @brief Возврат правки
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool redoLastEdit(const std::string& filePath);
    
    /**
     * @brief Очистка истории для файла
     * @param filePath Путь к файлу
     */
    void clearStory(const std::string& filePath);
    
    /**
     * @brief Проверка наличия истории для файла
     * @param filePath Путь к файлу
     * @return true если история существует
     */
    bool hasStory(const std::string& filePath) const;
    
    /**
     * @brief Получение пути к файлу истории
     * @param filePath Путь к файлу исходного кода
     * @return Путь к файлу истории
     */
    std::string getStoryFilePath(const std::string& filePath) const;
    
    /**
     * @brief Вычисление контрольной суммы текста
     * @param text Текст
     * @return Контрольная сумма
     */
    static std::string calculateChecksum(const std::string& text);
    
private:
    StoryManager();
    ~StoryManager();
    
    std::string projectPath;
    std::string storyDirectory;
    std::map<std::string, std::vector<StoryEntry>> stories;  // filePath -> entries
    std::map<std::string, size_t> currentEntryIndex;  // filePath -> current position for undo/redo
    
    void ensureStoryDirectory();
    std::string getRelativeFilePath(const std::string& filePath) const;
};

} // namespace proxima

#endif // PROXIMA_STORYMANAGER_H