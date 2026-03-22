#ifndef PROXIMA_STORYMANAGER_H
#define PROXIMA_STORYMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>
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
    QDateTime timestamp;        // Время изменения
    EditOperation operation;    // Операция (add/remove)
    QString text;               // Текст правки
    int startLine;              // Начальная строка
    int endLine;                // Конечная строка
    QString checksum;           // Контрольная сумма блока
    
    StoryEntry() : operation(EditOperation::Add), startLine(0), endLine(0) {}
    
    /**
     * @brief Сериализация в Collection
     * @return Collection представление
     */
    Collection toCollection() const {
        Collection entry;
        entry.set("timestamp", Collection::fromString(timestamp.toString("dd/MM/yyyy HH:mm")));
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
        timestamp = QDateTime::fromString(collection.get("timestamp").toString(), "dd/MM/yyyy HH:mm");
        QString op = collection.get("operation").toString();
        operation = (op == "add") ? EditOperation::Add : EditOperation::Remove;
        text = collection.get("text").toString();
        startLine = collection.get("startLine").toNumber();
        endLine = collection.get("endLine").toNumber();
        checksum = collection.get("checksum").toString();
    }
    
    /**
     * @brief Вычисление возраста правки в минутах
     * @return Возраст в минутах
     */
    qint64 ageInMinutes() const {
        return timestamp.secsTo(QDateTime::currentDateTime()) / 60;
    }
    
    /**
     * @brief Получение цвета на основе возраста
     * @return Цвет для подсветки
     */
    QColor getAgeColor() const;
    
    /**
     * @brief Получение строкового представления возраста
     * @return Строка с возрастом
     */
    QString getAgeString() const;
};

/**
 * @brief Менеджер истории правок файлов
 * 
 * Хранит историю в папке .proxima/story/<filename>.story
 */
class StoryManager : public QObject {
    Q_OBJECT
    
public:
    static StoryManager& getInstance();
    
    /**
     * @brief Инициализация менеджера
     * @param projectPath Путь к проекту
     */
    void initialize(const QString& projectPath);
    
    /**
     * @brief Загрузка истории для файла
     * @param filePath Путь к файлу исходного кода
     * @return true если успешно
     */
    bool loadStory(const QString& filePath);
    
    /**
     * @brief Сохранение истории для файла
     * @param filePath Путь к файлу исходного кода
     * @return true если успешно
     */
    bool saveStory(const QString& filePath);
    
    /**
     * @brief Добавление записи в историю
     * @param filePath Путь к файлу
     * @param operation Операция
     * @param text Текст правки
     * @param startLine Начальная строка
     * @param endLine Конечная строка
     */
    void addEditEntry(const QString& filePath, EditOperation operation, 
                     const QString& text, int startLine, int endLine);
    
    /**
     * @brief Получение всех записей истории для файла
     * @param filePath Путь к файлу
     * @return Вектор записей истории
     */
    QVector<StoryEntry> getStoryEntries(const QString& filePath) const;
    
    /**
     * @brief Получение записи истории для строки
     * @param filePath Путь к файлу
     * @param line Номер строки (1-based)
     * @return Запись истории или nullptr
     */
    StoryEntry* getEntryForLine(const QString& filePath, int line);
    
    /**
     * @brief Получение информации о возрасте для всех строк
     * @param filePath Путь к файлу
     * @return Карта: номер строки -> время изменения
     */
    QMap<int, QDateTime> getAgeInfo(const QString& filePath);
    
    /**
     * @brief Отмена последней правки
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool undoLastEdit(const QString& filePath);
    
    /**
     * @brief Возврат правки
     * @param filePath Путь к файлу
     * @return true если успешно
     */
    bool redoLastEdit(const QString& filePath);
    
    /**
     * @brief Очистка истории для файла
     * @param filePath Путь к файлу
     */
    void clearStory(const QString& filePath);
    
    /**
     * @brief Проверка наличия истории для файла
     * @param filePath Путь к файлу
     * @return true если история существует
     */
    bool hasStory(const QString& filePath) const;
    
    /**
     * @brief Получение пути к файлу истории
     * @param filePath Путь к файлу исходного кода
     * @return Путь к файлу истории
     */
    QString getStoryFilePath(const QString& filePath) const;
    
    /**
     * @brief Вычисление контрольной суммы текста
     * @param text Текст
     * @return Контрольная сумма
     */
    static QString calculateChecksum(const QString& text);
    
signals:
    void storyLoaded(const QString& filePath);
    void storySaved(const QString& filePath);
    void entryAdded(const QString& filePath, const StoryEntry& entry);
    void editUndone(const QString& filePath);
    void editRedone(const QString& filePath);
    
private:
    StoryManager(QObject *parent = nullptr);
    ~StoryManager();
    
    QString projectPath;
    QString storyDirectory;
    QMap<QString, QVector<StoryEntry>> stories;  // filePath -> entries
    QMap<QString, int> currentEntryIndex;  // filePath -> current position for undo/redo
    
    void ensureStoryDirectory();
    QString getRelativeFilePath(const QString& filePath) const;
};

} // namespace proxima

#endif // PROXIMA_STORYMANAGER_H