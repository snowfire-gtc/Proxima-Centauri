#ifndef PROXIMA_TIME_H
#define PROXIMA_TIME_H

#include <string>
#include <cstdint>
#include <vector>

namespace proxima {
namespace stdlib {

class Time {
public:
    // Конструкторы
    Time();
    Time(int64_t milliseconds);
    Time(const std::string& formatted, const std::string& format = "");
    
    // ========================================================================
    // Статические методы класса (вместо отдельных функций)
    // ========================================================================
    
    // Получение текущего времени
    static Time now();
    
    // Получение timestamp в миллисекундах
    static int64_t epoch();
    
    // Создание времени из Unix timestamp (секунды)
    static Time fromUnixTimestamp(int64_t seconds);
    
    // Создание времени из компонентов
    static Time fromLocalTime(int hour, int minute, int second,
                             int day, int month, int year, int millisecond = 0);
    
    // Сон (блокировка выполнения)
    static void sleep(int64_t milliseconds);
    
    // Парсинг строки в время
    static Time parse(const std::string& str, const std::string& format = "");
    
    // ========================================================================
    // Методы экземпляра
    // ========================================================================
    
    // Получение timestamp
    int64_t getTimestamp() const;
    void setTimestamp(int64_t ms);
    
    // Форматирование
    std::string formatted(const std::string& format = "") const;
    
    // Компоненты времени
    int getHour() const;
    int getMinute() const;
    int getSecond() const;
    int getMillisecond() const;
    int getDay() const;
    int getMonth() const;
    int getYear() const;
    int getDayOfWeek() const;
    int getDayOfYear() const;
    
    // Арифметика
    Time add(int64_t milliseconds) const;
    Time subtract(int64_t milliseconds) const;
    int64_t difference(const Time& other) const;
    
    // Операторы
    Time& operator+=(int64_t ms);
    Time& operator-=(int64_t ms);
    int64_t operator-(const Time& other) const;
    bool operator==(const Time& other) const;
    bool operator!=(const Time& other) const;
    bool operator<(const Time& other) const;
    bool operator<=(const Time& other) const;
    bool operator>(const Time& other) const;
    bool operator>=(const Time& other) const;
    
    // Временные зоны
    Time toUTC() const;
    Time toLocal() const;
    std::string getTimezone() const;
    
    // Валидация
    bool isValid() const;
    bool isToday() const;
    bool isPast() const;
    bool isFuture() const;
    
    // Сериализация
    std::string toJson() const;
    static Time fromJson(const std::string& json);
    
    // RTTI для IDE
    static std::string getClassName() { return "time"; }
    static std::vector<std::string> getStaticMethods();
    static std::vector<std::string> getInstanceMethods();
    static std::vector<std::string> getProperties();
    
private:
    int64_t timestamp; // Миллисекунды с эпохи
    
    static int64_t parseAuto(const std::string& str);
};

} // namespace stdlib
} // namespace proxima

#endif // PROXIMA_TIME_H