#ifndef PROXIMA_STORYMANAGER_H
#define PROXIMA_STORYMANAGER_H

#include <string>
#include <vector>
#include <map>
#include <ctime>
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
 * @brief Запись истории правок
 *
 * Формат хранения в .story файле:
 * [<время изменения>, <add/remove>, <текст правки>]
 */
struct StoryEntry {
    std::time_t timestamp;        // Время изменения
    EditOperation operation;      // Операция (add/remove)
    std::string text;             // Текст правки
    int startLine;                // Начальная строка
    int endLine;                  // Конечная строка
    std::string checksum;         // Контрольная сумма блока

    StoryEntry() : operation(EditOperation::Add), startLine(0), endLine(0) {}

    /**
     * @brief Сериализация в Collection
     * @return Collection представление
     */
    Collection toCollection() const {
        Collection entry;
        char timeBuf[64];
        std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", std::localtime(&timestamp));
        entry.set("timestamp", Collection::fromString(std::string(timeBuf)));
        entry.set("operation", Collection::fromString(operation == EditOperation::Add ? "add" : "remove"));
        entry.set("text", Collection::fromString(text));
        entry.set("startLine", Collection::fromInt(startLine));
        entry.set("endLine", Collection::fromInt(endLine));
        entry.set("checksum", Collection::fromString(checksum));
        return entry;
    }

    /**
     * @brief Десериализация из Collection
     * @param col Collection представление
     */
    static StoryEntry fromCollection(const Collection& col) {
        StoryEntry entry;
        // TODO: реализовать парсинг времени
        entry.operation = col.get("operation").asString() == "add" ? EditOperation::Add : EditOperation::Remove;
        entry.text = col.get("text").asString();
        entry.startLine = static_cast<int>(col.get("startLine").asInt());
        entry.endLine = static_cast<int>(col.get("endLine").asInt());
        entry.checksum = col.get("checksum").asString();
        return entry;
    }
};

/**
 * @brief Менеджер истории правок кода
 * 
 * Согласно requirements/story.txt:
 * - Хранение истории изменений в .story файлах
 * - Поддержка отката изменений
 * - Визуализация истории
 */
class StoryManager {
    
public:
    StoryManager();
    ~StoryManager();
    
    // ========================================================================
    // Основные методы
    // ========================================================================
    
    /**
     * @brief Загрузка истории из файла
     * @param filename Имя файла .story
     * @return true если успешно
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * @brief Сохранение истории в файл
     * @param filename Имя файла .story
     * @return true если успешно
     */
    bool saveToFile(const std::string& filename) const;
    
    /**
     * @brief Добавление записи об изменении
     * @param text Текст правки
     * @param operation Операция
     * @param startLine Начальная строка
     * @param endLine Конечная строка
     */
    void addEntry(const std::string& text, EditOperation operation, int startLine, int endLine);
    
    /**
     * @brief Получение всех записей истории
     * @return Вектор записей
     */
    const std::vector<StoryEntry>& getEntries() const;
    
    /**
     * @brief Откат к указанной версии
     * @param index Индекс записи в истории
     * @return true если успешно
     */
    bool revertTo(size_t index);
    
    /**
     * @brief Очистка истории
     */
    void clear();
    
    /**
     * @brief Количество записей в истории
     * @return Количество
     */
    size_t size() const;
    
private:
    std::vector<StoryEntry> entries;
    std::string currentFile;
    
    /**
     * @brief Вычисление контрольной суммы
     * @param text Текст
     * @return Контрольная сумма
     */
    std::string computeChecksum(const std::string& text) const;
};

} // namespace proxima

#endif // PROXIMA_STORYMANAGER_H
