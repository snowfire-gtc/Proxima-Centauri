#ifndef PROXIMA_RUNTIME_H
#define PROXIMA_RUNTIME_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <filesystem>
#ifdef HAVE_LLVM
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>
#endif
#include "utils/Logger.h"
#include "utils/CollectionParser.h"
#include "stdlib/Collection.h"
#include "stdlib/Time.h"
#include "gem.h"

#ifdef USE_CUDA
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#endif

namespace proxima {

/**
 * @brief Конвертация индекса 1-based ↔ 0-based
 *
 * Согласно language.txt пункт 11.1:
 * "Все массивы индексируются, начиная с единичного индекса"
 */
class IndexConverter {
public:
    /**
     * @brief Конвертация индекса из Proxima (1-based) в C++ (0-based)
     * @param proximaIndex Индекс в стиле Proxima (начинается с 1)
     * @return Индекс в стиле C++ (начинается с 0)
     */
    static inline int toCppIndex(int proximaIndex) {
        return proximaIndex - 1;
    }

    /**
     * @brief Конвертация индекса из C++ (0-based) в Proxima (1-based)
     * @param cppIndex Индекс в стиле C++ (начинается с 0)
     * @return Индекс в стиле Proxima (начинается с 1)
     */
    static inline int toProximaIndex(int cppIndex) {
        return cppIndex + 1;
    }

    /**
     * @brief Проверка валидности индекса Proxima
     * @param proximaIndex Индекс в стиле Proxima
     * @param size Размер массива
     * @return true если индекс валиден
     */
    static inline bool isValidProximaIndex(int proximaIndex, int size) {
        return proximaIndex >= 1 && proximaIndex <= size;
    }

    /**
     * @brief Проверка валидности диапазона индексов Proxima
     * @param start Начальный индекс (1-based)
     * @param end Конечный индекс (1-based)
     * @param size Размер массива
     * @return true если диапазон валиден
     */
    static inline bool isValidProximaRange(int start, int end, int size) {
        return start >= 1 && end <= size && start <= end;
    }
};

/**
 * @brief Структура статистики Runtime
 */
struct RuntimeStats {
    size_t memoryUsed;
    size_t memoryLimit;
    size_t allocations;
    size_t gpuAllocations;
    int64_t uptime;
    int verboseLevel;
    bool debugMode;
    bool cudaAvailable;
};

/**
 * @brief Класс Runtime с поддержкой GEM и отладки
 */
class Runtime {
public:
    Runtime();
    ~Runtime();

    // ========================================================================
    // Initialization
    // ========================================================================

    bool initialize();
    bool loadModule(std::unique_ptr<llvm::Module> module);
    int execute(const std::string& entryPoint = "main");
    void* getFunctionPointer(const std::string& name);

    // ========================================================================
    // Memory management
    // ========================================================================

    void* allocate(size_t size);
    void deallocate(void* ptr);
    size_t getMemoryUsage() const;
    size_t getMemoryLimit() const { return memoryLimit; }
    void setMemoryLimit(size_t limit);

    // ========================================================================
    // Debug support
    // ========================================================================

    void enableDebugMode(bool enable);
    bool isDebugMode() const { return debugMode; }
    void setVerboseLevel(int level);
    int getVerboseLevel() const { return verboseLevel; }

    // ========================================================================
    // GPU support
    // ========================================================================

    bool isCUDAAvailable() const { return cudaAvailable; }
    bool enableCUDA();
    void* allocateGPU(size_t size);
    void freeGPU(void* ptr);
    void copyToGPU(void* host, void* device, size_t size);
    void copyFromGPU(void* device, void* host, size_t size);

    // ========================================================================
    // Standard library
    // ========================================================================

    void registerStdLib();

    // ========================================================================
    // System info
    // ========================================================================

    size_t getSystemMemoryFree() const;
    size_t getSystemMemoryTotal() const;
    double getCPUUsage() const;
    size_t getDiskFree(const std::string& path) const;
    std::string getOSInfo() const;
    std::string getCPUInfo() const;
    int getCPUCount() const;

    // ========================================================================
    // Debug functions (language.txt #43)
    // ========================================================================

    void dbgstop();
    void dbgprint(const std::string& message, int level = 0);
    void dbgcontext();
    void dbgstack();

    // ========================================================================
    // GEM interface (language.txt #44)
    // ========================================================================

    /**
     * @brief Создание GEM объекта
     * @param type Тип объекта
     * @param name Имя объекта
     * @return Указатель на GEM объект
     */
    GEM* createGEMObject(const std::string& type, const std::string& name = "");

    /**
     * @brief Инициализация GEM объекта
     * @param object GEM объект
     * @return true если успешно
     */
    bool initGEMObject(GEM* object);

    /**
     * @brief Обновление GEM объекта
     * @param object GEM объект
     * @param currentTime Текущее время
     * @return Результат обновления
     */
    Collection updateGEMObject(GEM* object, const Time& currentTime);

    /**
     * @brief Сброс GEM объекта
     * @param object GEM объект
     * @return true если успешно
     */
    bool resetGEMObject(GEM* object);

    /**
     * @brief Получение метрик GEM объекта
     * @param object GEM объект
     * @return Метрики
     */
    Collection getGEMMetrics(GEM* object);

    /**
     * @brief Получение пользовательских метрик GEM объекта
     * @param object GEM объект
     * @return Пользовательские метрики
     */
    Collection getGEMUserMetrics(GEM* object);

    /**
     * @brief Установка параметров GEM объекта
     * @param object GEM объект
     * @param params Параметры
     */
    void setGEMParams(GEM* object, const Collection& params);

    /**
     * @brief Получение параметров GEM объекта
     * @param object GEM объект
     * @return Параметры
     */
    Collection getGEMParams(GEM* object);

    /**
     * @brief Публикация GEM объекта
     * @param object GEM объект
     * @param doc Документ для публикации
     */
    void publishGEMObject(GEM* object, const Collection& doc);

    /**
     * @brief Сохранение состояния GEM объекта
     * @param object GEM объект
     * @return Состояние
     */
    Collection storeGEMObject(GEM* object);

    /**
     * @brief Восстановление состояния GEM объекта
     * @param object GEM объект
     * @param state Состояние
     */
    void restoreGEMObject(GEM* object, const Collection& state);

    /**
     * @brief Отображение GEM объекта
     * @param object GEM объект
     */
    void showGEMObject(GEM* object);

    /**
     * @brief Уничтожение GEM объекта
     * @param object GEM объект
     */
    void destroyGEMObject(GEM* object);

    // ========================================================================
    // Parallel execution
    // ========================================================================

    void parallelFor(int start, int end, int step,
                    std::function<void(int)> body,
                    int threads = 4, void* array = nullptr);

    // ========================================================================
    // Serialization
    // ========================================================================

    std::string serialize(const void* data, size_t size, const std::string& type);
    void* deserialize(const std::string& data, size_t& size, const std::string& type);

    // ========================================================================
    // Utilities
    // ========================================================================

    void sleep(int64_t milliseconds);
    int64_t getTimestamp() const;
    void exit(int code);
    int getpid() const;
    std::string getCWD() const;
    bool setCWD(const std::string& path);
    bool fileExists(const std::string& path) const;
    bool directoryExists(const std::string& path) const;
    bool createDirectory(const std::string& path);
    bool deleteFile(const std::string& path);
    std::vector<std::string> listDirectory(const std::string& path);

    // ========================================================================
    // Configuration
    // ========================================================================

    void setAllowCUDA(bool allow);
    void setAllowAVX2(bool allow);
    void setAllowSSE4(bool allow);
    bool isCUDAAllowed() const;
    bool isAVX2Allowed() const;
    bool isSSE4Allowed() const;

    // ========================================================================
    // Statistics
    // ========================================================================

    RuntimeStats getStats() const;
    void resetStats();

    // ========================================================================
    // Timing
    // ========================================================================

    void startTimer(const std::string& name);
    double stopTimer(const std::string& name);
    double getTimer(const std::string& name) const;

    // ========================================================================
    // Error handling
    // ========================================================================

    void setError(const std::string& error);
    std::string getError() const;
    bool hasError() const;
    void clearError();
    void throwIndexError(const std::string& containerType, int proximaIndex, int size);

    // ========================================================================
    // 1-based indexing access methods
    // ========================================================================

    /**
     * @brief Доступ к элементу вектора с индексацией с 1
     * @param vector Вектор
     * @param index Индекс (1-based)
     * @return Значение элемента
     * @throws std::out_of_range если индекс вне диапазона
     */
    RuntimeValue getVectorElement(const std::vector<RuntimeValue>& vector, int index);

    /**
     * @brief Доступ к элементу матрицы с индексацией с 1
     * @param matrix Матрица
     * @param row Номер строки (1-based)
     * @param col Номер столбца (1-based)
     * @return Значение элемента
     * @throws std::out_of_range если индекс вне диапазона
     */
    RuntimeValue getMatrixElement(const std::vector<std::vector<RuntimeValue>>& matrix,
                                 int row, int col);

    /**
     * @brief Доступ к элементу слоя с индексацией с 1
     * @param layer Слой
     * @param x Координата X (1-based)
     * @param y Координата Y (1-based)
     * @param z Координата Z (1-based)
     * @return Значение элемента
     * @throws std::out_of_range если индекс вне диапазона
     */
    RuntimeValue getLayerElement(const std::vector<std::vector<std::vector<RuntimeValue>>>& layer,
                                int x, int y, int z);

    /**
     * @brief Установка элемента вектора с индексацией с 1
     * @param vector Вектор
     * @param index Индекс (1-based)
     * @param value Значение
     * @throws std::out_of_range если индекс вне диапазона
     */
    void setVectorElement(std::vector<RuntimeValue>& vector, int index, const RuntimeValue& value);

    /**
     * @brief Установка элемента матрицы с индексацией с 1
     * @param matrix Матрица
     * @param row Номер строки (1-based)
     * @param col Номер столбца (1-based)
     * @param value Значение
     * @throws std::out_of_range если индекс вне диапазона
     */
    void setMatrixElement(std::vector<std::vector<RuntimeValue>>& matrix,
                         int row, int col, const RuntimeValue& value);

    /**
     * @brief Получение размера вектора для отображения (1-based)
     * @param vector Вектор
     * @return Размер в стиле Proxima
     */
    int getVectorSize(const std::vector<RuntimeValue>& vector) const;

    /**
     * @brief Получение размера матрицы для отображения (1-based)
     * @param matrix Матрица
     * @return Количество строк в стиле Proxima
     */
    int getMatrixRows(const std::vector<std::vector<RuntimeValue>>& matrix) const;

    /**
     * @brief Получение размера матрицы для отображения (1-based)
     * @param matrix Матрица
     * @return Количество столбцов в стиле Proxima
     */
    int getMatrixCols(const std::vector<std::vector<RuntimeValue>>& matrix) const;

    // ========================================================================
    // Type check operators (ИСПРАВЛЕНО)
    // ========================================================================

    /**
     * @brief Проверка равенства ТОЛЬКО типов (===)
     * @param leftType Тип левого операнда
     * @param rightType Тип правого операнда
     * @return true если типы равны (значения НЕ проверяются)
     */
    inline bool typeEqual(const std::string& leftType, const std::string& rightType) {
        return normalizeTypeName(leftType) == normalizeTypeName(rightType);
    }

    /**
     * @brief Проверка неравенства ТОЛЬКО типов (!==)
     * @param leftType Тип левого операнда
     * @param rightType Тип правого операнда
     * @return true если типы разные (значения НЕ проверяются)
     */
    inline bool typeNotEqual(const std::string& leftType, const std::string& rightType) {
        return !typeEqual(leftType, rightType);
    }

    /**
     * @brief Нормализация имени типа
     * @param typeName Имя типа
     * @return Нормализованное имя типа
     */
    std::string normalizeTypeName(const std::string& typeName);

private:
    void registerPrintFunction();
    void registerMathFunctions();
    void registerIOFunctions();
    void registerCollectionFunctions();
    void registerTimeFunctions();
    void registerDebugFunctions();
    void registerSystemFunctions();
    void registerMemoryFunctions();

    void log(int level, const std::string& message);
    void logError(const std::string& message);
    void logWarning(const std::string& message);
    void logDebug(const std::string& message);

#ifdef HAVE_LLVM
    std::unique_ptr<llvm::orc::LLJIT> jit;
    llvm::LLVMContext* context;
#endif

    bool initialized;
    bool debugMode;
    bool cudaAvailable;
    bool cudaAllowed;
    bool avx2Allowed;
    bool sse4Allowed;
    int verboseLevel;
    size_t memoryLimit;
    size_t currentMemoryUsage;
    int64_t startTime;

    std::vector<void*> allocatedMemory;
    std::vector<void*> gpuMemory;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> timers;

    std::mutex memoryMutex;
    std::mutex debugMutex;
    std::condition_variable debugCondition;
    bool stopRequested;

    std::string lastError;
};

} // namespace proxima

#endif // PROXIMA_RUNTIME_H
