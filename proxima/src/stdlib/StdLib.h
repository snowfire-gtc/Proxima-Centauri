#ifndef PROXIMA_STDLIB_H
#define PROXIMA_STDLIB_H

#include "Time.h"
#include "IO.h"
#include "Math.h"
#include "Collection.h"

namespace proxima {
namespace stdlib {

// ============================================================================
// Реестр всех стандартных классов для autocomplete
// ============================================================================

class StdLibRegistry {
public:
    static StdLibRegistry& getInstance();
    
    // Регистрация классов
    void registerClass(const std::string& name,
                      const std::vector<std::string>& staticMethods,
                      const std::vector<std::string>& instanceMethods,
                      const std::vector<std::string>& properties,
                      const std::string& description);
    
    // Получение информации о классе
    std::vector<std::string> getStaticMethods(const std::string& className) const;
    std::vector<std::string> getInstanceMethods(const std::string& className) const;
    std::vector<std::string> getProperties(const std::string& className) const;
    std::string getClassDescription(const std::string& className) const;
    std::string getMethodDescription(const std::string& className, 
                                    const std::string& methodName) const;
    
    // Проверка существования класса
    bool classExists(const std::string& className) const;
    
    // Получение всех зарегистрированных классов
    std::vector<std::string> getAllClasses() const;
    
    // Инициализация стандартных классов
    void initializeStdLib();
    
private:
    StdLibRegistry();
    
    struct ClassInfo {
        std::vector<std::string> staticMethods;
        std::vector<std::string> instanceMethods;
        std::vector<std::string> properties;
        std::string description;
        std::map<std::string, std::string> methodDescriptions;
    };
    
    std::map<std::string, ClassInfo> classes;
};

// ============================================================================
// Глобальные функции (те, которые должны остаться функциями)
// ============================================================================

// Print (остаётся функцией)
void print(const std::string& format, ...);
void print(FILE* file, const std::string& format, ...);

// Write/Read (остаётся функцией)
void write(FILE* file, const void* data, size_t size);
void read(FILE* file, void* data, size_t size);

// Математические функции (можно оставить как функции для удобства)
// Или сделать методами класса Math
double abs(double x);
double sqrt(double x);
double sin(double x);
double cos(double x);
// ...

} // namespace stdlib
} // namespace proxima

#endif // PROXIMA_STDLIB_H