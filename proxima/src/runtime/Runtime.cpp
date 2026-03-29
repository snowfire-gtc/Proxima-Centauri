#include "Runtime.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <regex>
#include <filesystem>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <ctime>
#include <cstdint>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "utils/Logger.h"
#include "stdlib/IO.h"
#include "stdlib/Math.h"
#include "stdlib/Collection.h"
#include "stdlib/Time.h"
#include "gem.h"

#ifdef USE_CUDA
#include <cuda_runtime.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#endif

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

Runtime::Runtime()
    : jit(nullptr)
    , context(nullptr)
    , initialized(false)
    , debugMode(false)
    , cudaAvailable(false)
    , verboseLevel(2)
    , memoryLimit(4 * 1024 * 1024 * 1024)
    , currentMemoryUsage(0)
    , cudaAllowed(false)
    , avx2Allowed(false)
    , sse4Allowed(false)
    , startTime(0)
    , stopRequested(false) {

    LOG_INFO("Runtime constructor called");
}

Runtime::~Runtime() {
    // Очистка выделенной памяти
    for (void* ptr : allocatedMemory) {
        std::free(ptr);
    }
    allocatedMemory.clear();

#ifdef USE_CUDA
    // Очистка GPU памяти
    for (void* ptr : gpuMemory) {
        cudaFree(ptr);
    }
    gpuMemory.clear();
#endif

    LOG_INFO("Runtime destructor called");
}

// ============================================================================
// Инициализация
// ============================================================================

bool Runtime::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing Runtime...");

    // Инициализация LLVM JIT
#ifdef HAVE_LLVM
    auto jitOrErr = llvm::orc::LLJITBuilder().create();
    if (!jitOrErr) {
        LOG_ERROR("Failed to create JIT: " + llvm::toString(jitOrErr.takeError()));
        return false;
    }

    jit = std::move(*jitOrErr);
    context = &jit->getContext();
#endif

    // Регистрация стандартной библиотеки
    registerStdLib();

    // Проверка CUDA
#ifdef USE_CUDA
    enableCUDA();
#endif

    initialized = true;
    startTime = getTimestamp();

    LOG_INFO("Runtime initialized successfully");
    return true;
}

bool Runtime::loadModule(std::unique_ptr<llvm::Module> module) {
    if (!initialized) {
        if (!initialize()) {
            return false;
        }
    }

#ifdef HAVE_LLVM
    auto err = jit->addModule(std::move(module));
    if (err) {
        LOG_ERROR("Failed to load module: " + llvm::toString(std::move(err)));
        return false;
    }
#endif

    LOG_INFO("Module loaded successfully");
    return true;
}

int Runtime::execute(const std::string& entryPoint) {
    if (!initialized) {
        LOG_ERROR("Runtime not initialized");
        return -1;
    }

    LOG_INFO("Executing: " + entryPoint);

#ifdef HAVE_LLVM
    auto sym = jit->lookup(entryPoint);
    if (!sym) {
        LOG_ERROR("Function '" + entryPoint + "' not found");
        return -1;
    }

    auto funcPtr = (int(*)())sym->getAddress();
    if (!funcPtr) {
        LOG_ERROR("Failed to get function pointer");
        return -1;
    }

    int result = funcPtr();
    LOG_INFO("Execution completed with result: " + std::to_string(result));

    return result;
#else
    LOG_ERROR("LLVM support not compiled in");
    return -1;
#endif
}

void* Runtime::getFunctionPointer(const std::string& name) {
#ifdef HAVE_LLVM
    auto sym = jit->lookup(name);
    if (sym) {
        return (void*)sym->getAddress();
    }
#endif
    return nullptr;
}

// ============================================================================
// Управление памятью
// ============================================================================

void* Runtime::allocate(size_t size) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    if (currentMemoryUsage + size > memoryLimit) {
        LOG_ERROR("Memory limit exceeded (limit: " + std::to_string(memoryLimit) +
                 ", current: " + std::to_string(currentMemoryUsage) +
                 ", requested: " + std::to_string(size) + ")");
        return nullptr;
    }

    void* ptr = std::malloc(size);
    if (ptr) {
        allocatedMemory.push_back(ptr);
        currentMemoryUsage += size;

        if (debugMode) {
            LOG_DEBUG("Allocated " + std::to_string(size) + " bytes at " +
                     std::to_string(reinterpret_cast<uintptr_t>(ptr)));
        }
    }

    return ptr;
}

void Runtime::deallocate(void* ptr) {
    if (!ptr) return;

    std::lock_guard<std::mutex> lock(memoryMutex);

    auto it = std::find(allocatedMemory.begin(), allocatedMemory.end(), ptr);
    if (it != allocatedMemory.end()) {
        // Получаем размер блока (в реальной реализации нужно хранить метаданные)
        size_t size = 0; // В полной реализации нужно отслеживать размер
        allocatedMemory.erase(it);
        std::free(ptr);

        if (size > 0) {
            currentMemoryUsage -= size;
        }

        if (debugMode) {
            LOG_DEBUG("Deallocated memory at " + std::to_string(reinterpret_cast<uintptr_t>(ptr)));
        }
    }
}

size_t Runtime::getMemoryUsage() const {
    return currentMemoryUsage;
}

void Runtime::setMemoryLimit(size_t limit) {
    memoryLimit = limit;
    LOG_INFO("Memory limit set to: " + std::to_string(limit) + " bytes");
}

// ============================================================================
// Отладка
// ============================================================================

void Runtime::enableDebugMode(bool enable) {
    debugMode = enable;
    LOG_INFO("Debug mode: " + std::string(enable ? "enabled" : "disabled"));
}

bool Runtime::isDebugMode() const {
    return debugMode;
}

void Runtime::setVerboseLevel(int level) {
    verboseLevel = level;
    Logger::getInstance().setLogLevel(static_cast<LogLevel>(level));
    LOG_INFO("Verbose level set to: " + std::to_string(level));
}

int Runtime::getVerboseLevel() const {
    return verboseLevel;
}

// ============================================================================
// GPU поддержка (CUDA)
// ============================================================================

bool Runtime::isCUDAAvailable() const {
    return cudaAvailable;
}

bool Runtime::enableCUDA() {
#ifdef USE_CUDA
    int deviceCount;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);

    if (err == cudaSuccess && deviceCount > 0) {
        cudaAvailable = true;

        // Получение информации о устройстве
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, 0);

        LOG_INFO("CUDA available: " + std::to_string(deviceCount) + " device(s)");
        LOG_INFO("Device name: " + std::string(prop.name));
        LOG_INFO("Compute capability: " + std::to_string(prop.major) + "." +
                std::to_string(prop.minor));
        LOG_INFO("Total global memory: " + std::to_string(prop.totalGlobalMem / 1024 / 1024) + " MB");

        return true;
    } else {
        cudaAvailable = false;
        LOG_WARNING("CUDA not available: " + std::string(cudaGetErrorString(err)));
        return false;
    }
#else
    cudaAvailable = false;
    LOG_WARNING("CUDA support not compiled in");
    return false;
#endif
}

void* Runtime::allocateGPU(size_t size) {
#ifdef USE_CUDA
    if (!cudaAvailable) {
        LOG_ERROR("CUDA not available");
        return nullptr;
    }

    void* ptr;
    cudaError_t err = cudaMalloc(&ptr, size);

    if (err == cudaSuccess) {
        gpuMemory.push_back(ptr);
        LOG_DEBUG("Allocated " + std::to_string(size) + " bytes on GPU at " +
                 std::to_string(reinterpret_cast<uintptr_t>(ptr)));
        return ptr;
    } else {
        LOG_ERROR("CUDA allocation failed: " + std::string(cudaGetErrorString(err)));
        return nullptr;
    }
#else
    LOG_ERROR("CUDA support not compiled in");
    return nullptr;
#endif
}

void Runtime::freeGPU(void* ptr) {
#ifdef USE_CUDA
    if (!ptr || !cudaAvailable) return;

    auto it = std::find(gpuMemory.begin(), gpuMemory.end(), ptr);
    if (it != gpuMemory.end()) {
        gpuMemory.erase(it);
        cudaFree(ptr);
        LOG_DEBUG("Freed GPU memory at " + std::to_string(reinterpret_cast<uintptr_t>(ptr)));
    }
#endif
}

void Runtime::copyToGPU(void* host, void* device, size_t size) {
#ifdef USE_CUDA
    if (!cudaAvailable || !host || !device) {
        LOG_ERROR("Invalid parameters for CUDA copy");
        return;
    }

    cudaError_t err = cudaMemcpy(device, host, size, cudaMemcpyHostToDevice);

    if (err != cudaSuccess) {
        LOG_ERROR("CUDA H2D copy failed: " + std::string(cudaGetErrorString(err)));
    } else {
        LOG_DEBUG("Copied " + std::to_string(size) + " bytes to GPU");
    }
#endif
}

void Runtime::copyFromGPU(void* device, void* host, size_t size) {
#ifdef USE_CUDA
    if (!cudaAvailable || !device || !host) {
        LOG_ERROR("Invalid parameters for CUDA copy");
        return;
    }

    cudaError_t err = cudaMemcpy(host, device, size, cudaMemcpyDeviceToHost);

    if (err != cudaSuccess) {
        LOG_ERROR("CUDA D2H copy failed: " + std::string(cudaGetErrorString(err)));
    } else {
        LOG_DEBUG("Copied " + std::to_string(size) + " bytes from GPU");
    }
#endif
}

// ============================================================================
// Стандартная библиотека
// ============================================================================

void Runtime::registerStdLib() {
    registerPrintFunction();
    registerMathFunctions();
    registerIOFunctions();
    registerCollectionFunctions();
    registerTimeFunctions();
    registerDebugFunctions();
    registerSystemFunctions();
    registerMemoryFunctions();

    LOG_INFO("Standard library registered");
}

void Runtime::registerPrintFunction() {
    LOG_DEBUG("Registered: print()");
}

void Runtime::registerMathFunctions() {
    LOG_DEBUG("Registered: math functions");
}

void Runtime::registerIOFunctions() {
    LOG_DEBUG("Registered: I/O functions");
}

void Runtime::registerCollectionFunctions() {
    LOG_DEBUG("Registered: collection functions");
}

void Runtime::registerTimeFunctions() {
    LOG_DEBUG("Registered: time functions");
}

void Runtime::registerDebugFunctions() {
    LOG_DEBUG("Registered: debug functions");
}

void Runtime::registerSystemFunctions() {
    LOG_DEBUG("Registered: system functions");
}

void Runtime::registerMemoryFunctions() {
    LOG_DEBUG("Registered: memory functions");
}

// ============================================================================
// Системные функции
// ============================================================================

size_t Runtime::getSystemMemoryFree() const {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return status.ullAvailPhys;
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.freeram * info.mem_unit;
    }
#endif
    return 0;
}

size_t Runtime::getSystemMemoryTotal() const {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return status.ullTotalPhys;
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.totalram * info.mem_unit;
    }
#endif
    return 0;
}

double Runtime::getCPUUsage() const {
#ifdef _WIN32
    return 0.0;
#else
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return 0.0;

    std::string line;
    std::getline(file, line);

    std::istringstream iss(line);
    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq;

    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;

    unsigned long long total = user + nice + system + idle + iowait + irq + softirq;
    unsigned long long idleTotal = idle + iowait;

    return 0.0;
#endif
}

size_t Runtime::getDiskFree(const std::string& path) const {
#ifdef _WIN32
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(path.c_str(), &freeBytesAvailable,
                            &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        return totalNumberOfFreeBytes.QuadPart;
    }
#else
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) == 0) {
        return stat.f_bavail * stat.f_frsize;
    }
#endif
    return 0;
}

std::string Runtime::getOSInfo() const {
#ifdef _WIN32
    return "Windows";
#else
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        return std::string(buffer.sysname) + " " + buffer.release;
    }
    return "Unknown";
#endif
}

std::string Runtime::getCPUInfo() const {
#ifdef _WIN32
    return "Unknown";
#else
    std::ifstream file("/proc/cpuinfo");
    if (!file.is_open()) return "Unknown";

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("model name") != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                return line.substr(pos + 2);
            }
        }
    }
    return "Unknown";
#endif
}

int Runtime::getCPUCount() const {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

// ============================================================================
// Отладочные функции (из language.txt пункт 43)
// ============================================================================

void Runtime::dbgstop() {
    if (!debugMode) return;

    LOG_INFO("Debug stop requested");

    // Остановка выполнения
    std::unique_lock<std::mutex> lock(debugMutex);
    debugCondition.wait(lock, [this]() {
        return !debugMode || stopRequested;
    });

    LOG_INFO("Debug stop released");
}

void Runtime::dbgprint(const std::string& message, int level) {
    if (level > verboseLevel) return;

    std::string prefix = "[DEBUG]";
    switch (level) {
        case 0: prefix = "[ERROR]"; break;
        case 1: prefix = "[WARN]"; break;
        case 2: prefix = "[INFO]"; break;
        case 3: prefix = "[DEBUG]"; break;
        case 4: prefix = "[TRACE]"; break;
        default: prefix = "[DEBUG]"; break;
    }

    std::cout << prefix << " " << message << std::endl;
    LOG_INFO(prefix + " " + message);
}

void Runtime::dbgcontext() {
    if (!debugMode) return;

    LOG_INFO("=== Debug Context ===");
    LOG_INFO("Current memory usage: " + std::to_string(currentMemoryUsage) + " bytes");
    LOG_INFO("Allocated blocks: " + std::to_string(allocatedMemory.size()));
    LOG_INFO("Verbose level: " + std::to_string(verboseLevel));
    LOG_INFO("Debug mode: " + std::string(debugMode ? "enabled" : "disabled"));

    // В полной реализации вывод переменных из стека
    LOG_INFO("Stack variables: (not implemented)");
}

void Runtime::dbgstack() {
    if (!debugMode) return;

    LOG_INFO("=== Call Stack ===");

    // В полной реализации вывод стека вызовов
#ifdef _WIN32
    // Windows stack trace
#else
    // Linux stack trace using backtrace()
#endif

    LOG_INFO("(Stack trace not fully implemented)");
}

// ============================================================================
// Параллельное выполнение (CPU)
// ============================================================================

void Runtime::parallelFor(int start, int end, int step,
                         std::function<void(int)> body,
                         int threads, void* array) {
    if (threads <= 0) {
        threads = std::thread::hardware_concurrency();
        if (threads == 0) threads = 4;
    }

    LOG_DEBUG("Parallel for: " + std::to_string(threads) + " threads, " +
             std::to_string(start) + " to " + std::to_string(end) +
             " step " + std::to_string(step));

    std::vector<std::thread> workers;
    std::mutex mutex;

    int range = (end - start) / step;
    int chunkSize = range / threads;

    for (int t = 0; t < threads; t++) {
        int threadStart = start + t * chunkSize * step;
        int threadEnd = (t == threads - 1) ? end : threadStart + chunkSize * step;

        workers.emplace_back([&, threadStart, threadEnd]() {
            for (int i = threadStart; i < threadEnd; i += step) {
                body(i);
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }
}

// ============================================================================
// Сериализация/Десериализация
// ============================================================================

std::string Runtime::serialize(const void* data, size_t size, const std::string& type) {
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(data), size);
    return oss.str();
}

void* Runtime::deserialize(const std::string& data, size_t& size, const std::string& type) {
    size = data.length();
    void* buffer = std::malloc(size);
    std::memcpy(buffer, data.c_str(), size);
    return buffer;
}

// ============================================================================
// Утилиты
// ============================================================================

void Runtime::sleep(int64_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

int64_t Runtime::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
}

void Runtime::exit(int code) {
    LOG_INFO("Runtime exit with code: " + std::to_string(code));
    std::exit(code);
}

int Runtime::getpid() const {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

std::string Runtime::getCWD() const {
    return std::filesystem::current_path().string();
}

bool Runtime::setCWD(const std::string& path) {
    try {
        std::filesystem::current_path(path);
        return true;
    } catch (...) {
        return false;
    }
}

bool Runtime::fileExists(const std::string& path) const {
    return std::filesystem::exists(path);
}

bool Runtime::directoryExists(const std::string& path) const {
    return std::filesystem::is_directory(path);
}

bool Runtime::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (...) {
        return false;
    }
}

bool Runtime::deleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (...) {
        return false;
    }
}

std::vector<std::string> Runtime::listDirectory(const std::string& path) {
    std::vector<std::string> files;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            files.push_back(entry.path().string());
        }
    } catch (...) {
        // Ignore errors
    }

    return files;
}

// ============================================================================
// Логирование
// ============================================================================

void Runtime::log(int level, const std::string& message) {
    if (level <= verboseLevel) {
        std::cout << "[Runtime] " << message << std::endl;
        LOG_INFO("[Runtime] " + message);
    }
}

void Runtime::logError(const std::string& message) {
    std::cerr << "[Runtime ERROR] " << message << std::endl;
    LOG_ERROR("[Runtime] " + message);
}

void Runtime::logWarning(const std::string& message) {
    std::cout << "[Runtime WARNING] " << message << std::endl;
    LOG_WARNING("[Runtime] " + message);
}

void Runtime::logDebug(const std::string& message) {
    if (verboseLevel >= 3) {
        std::cout << "[Runtime DEBUG] " << message << std::endl;
        LOG_DEBUG("[Runtime] " + message);
    }
}

// ============================================================================
// Обработка ошибок
// ============================================================================

void Runtime::setError(const std::string& error) {
    lastError = error;
    LOG_ERROR("Runtime error: " + error);
}

std::string Runtime::getError() const {
    return lastError;
}

bool Runtime::hasError() const {
    return !lastError.empty();
}

void Runtime::clearError() {
    lastError.clear();
}

// ============================================================================
// Конфигурация
// ============================================================================

void Runtime::setAllowCUDA(bool allow) {
    if (allow && !cudaAvailable) {
        enableCUDA();
    }
    cudaAllowed = allow;
    LOG_INFO("CUDA allowed: " + std::string(allow ? "true" : "false"));
}

void Runtime::setAllowAVX2(bool allow) {
    avx2Allowed = allow;
    LOG_INFO("AVX2 allowed: " + std::string(allow ? "true" : "false"));
}

void Runtime::setAllowSSE4(bool allow) {
    sse4Allowed = allow;
    LOG_INFO("SSE4 allowed: " + std::string(allow ? "true" : "false"));
}

bool Runtime::isCUDAAllowed() const {
    return cudaAllowed;
}

bool Runtime::isAVX2Allowed() const {
    return avx2Allowed;
}

bool Runtime::isSSE4Allowed() const {
    return sse4Allowed;
}

// ============================================================================
// Статистика
// ============================================================================

RuntimeStats Runtime::getStats() const {
    RuntimeStats stats;
    stats.memoryUsed = currentMemoryUsage;
    stats.memoryLimit = memoryLimit;
    stats.allocations = allocatedMemory.size();
    stats.gpuAllocations = gpuMemory.size();
    stats.uptime = getTimestamp() - startTime;
    stats.verboseLevel = verboseLevel;
    stats.debugMode = debugMode;
    stats.cudaAvailable = cudaAvailable;
    return stats;
}

void Runtime::resetStats() {
    startTime = getTimestamp();
    currentMemoryUsage = 0;
    LOG_INFO("Runtime stats reset");
}

// ============================================================================
// Временные метки
// ============================================================================

void Runtime::startTimer(const std::string& name) {
    timers[name] = std::chrono::high_resolution_clock::now();
    if (verboseLevel >= 4) {
        LOG_DEBUG("Timer started: " + name);
    }
}

double Runtime::stopTimer(const std::string& name) {
    auto it = timers.find(name);
    if (it != timers.end()) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end - it->second).count();

        if (verboseLevel >= 4) {
            LOG_DEBUG("Timer stopped: " + name + " = " +
                     std::to_string(duration * 1000) + " ms");
        }

        timers.erase(it);
        return duration * 1000; // Return milliseconds
    }
    return 0.0;
}

double Runtime::getTimer(const std::string& name) const {
    auto it = timers.find(name);
    if (it != timers.end()) {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(now - it->second).count() * 1000;
    }
    return 0.0;
}

// ============================================================================
// Runtime Array Access Methods (1-based indexing)
// ============================================================================

RuntimeValue Runtime::getVectorElement(const std::vector<RuntimeValue>& vector, int index) {
    // Конвертация 1-based → 0-based
    int cppIndex = IndexConverter::toCppIndex(index);

    // Проверка валидности
    if (!IndexConverter::isValidProximaIndex(index, static_cast<int>(vector.size()))) {
        std::string error = "Vector index out of range: " + std::to_string(index) +
                           " (valid range: 1-" + std::to_string(vector.size()) + ")";
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    return vector[cppIndex];
}

RuntimeValue Runtime::getMatrixElement(
    const std::vector<std::vector<RuntimeValue>>& matrix,
    int row, int col) {

    // Конвертация 1-based → 0-based
    int cppRow = IndexConverter::toCppIndex(row);
    int cppCol = IndexConverter::toCppIndex(col);

    // Проверка валидности
    if (!IndexConverter::isValidProximaIndex(row, static_cast<int>(matrix.size()))) {
        std::string error = "Matrix row index out of range: " + std::to_string(row);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    if (!IndexConverter::isValidProximaIndex(col, static_cast<int>(matrix[cppRow].size()))) {
        std::string error = "Matrix column index out of range: " + std::to_string(col);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    return matrix[cppRow][cppCol];
}

RuntimeValue Runtime::getLayerElement(
    const std::vector<std::vector<std::vector<RuntimeValue>>>& layer,
    int x, int y, int z) {

    // Конвертация 1-based → 0-based
    int cppX = IndexConverter::toCppIndex(x);
    int cppY = IndexConverter::toCppIndex(y);
    int cppZ = IndexConverter::toCppIndex(z);

    // Проверка валидности
    if (!IndexConverter::isValidProximaIndex(x, static_cast<int>(layer.size()))) {
        std::string error = "Layer X index out of range: " + std::to_string(x);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    if (!IndexConverter::isValidProximaIndex(y, static_cast<int>(layer[cppX].size()))) {
        std::string error = "Layer Y index out of range: " + std::to_string(y);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    if (!IndexConverter::isValidProximaIndex(z, static_cast<int>(layer[cppX][cppY].size()))) {
        std::string error = "Layer Z index out of range: " + std::to_string(z);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    return layer[cppX][cppY][cppZ];
}

void Runtime::setVectorElement(std::vector<RuntimeValue>& vector, int index,
                              const RuntimeValue& value) {
    // Конвертация 1-based → 0-based
    int cppIndex = IndexConverter::toCppIndex(index);

    // Проверка валидности
    if (!IndexConverter::isValidProximaIndex(index, static_cast<int>(vector.size()))) {
        std::string error = "Vector index out of range: " + std::to_string(index);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    vector[cppIndex] = value;
}

void Runtime::setMatrixElement(std::vector<std::vector<RuntimeValue>>& matrix,
                              int row, int col, const RuntimeValue& value) {
    // Конвертация 1-based → 0-based
    int cppRow = IndexConverter::toCppIndex(row);
    int cppCol = IndexConverter::toCppIndex(col);

    // Проверка валидности
    if (!IndexConverter::isValidProximaIndex(row, static_cast<int>(matrix.size()))) {
        std::string error = "Matrix row index out of range: " + std::to_string(row);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    if (!IndexConverter::isValidProximaIndex(col, static_cast<int>(matrix[cppRow].size()))) {
        std::string error = "Matrix column index out of range: " + std::to_string(col);
        LOG_ERROR(error);
        throw std::out_of_range(error);
    }

    matrix[cppRow][cppCol] = value;
}

void Runtime::throwIndexError(const std::string& containerType,
                             int proximaIndex,
                             int size) {
    std::string message = containerType + " index out of range\n";
    message += "  Provided index: " + std::to_string(proximaIndex) + " (1-based)\n";
    message += "  Valid range: 1 to " + std::to_string(size) + "\n";
    message += "  Note: Proxima uses 1-based indexing (like MATLAB), not 0-based (like C++)\n";

    LOG_ERROR(message);
    throw std::out_of_range(message);
}

int Runtime::getVectorSize(const std::vector<RuntimeValue>& vector) const {
    // Возвращаем размер в стиле Proxima (1-based для отображения)
    return static_cast<int>(vector.size());
}

int Runtime::getMatrixRows(const std::vector<std::vector<RuntimeValue>>& matrix) const {
    // Возвращаем количество строк в стиле Proxima
    return static_cast<int>(matrix.size());
}

int Runtime::getMatrixCols(const std::vector<std::vector<RuntimeValue>>& matrix) const {
    if (matrix.empty()) {
        return 0;
    }
    // Возвращаем количество столбцов в стиле Proxima
    return static_cast<int>(matrix[0].size());
}

// ============================================================================
// Runtime GEM Methods
// ============================================================================

GEM* Runtime::createGEMObject(const std::string& type, const std::string& name) {
    GEM* object = new GEM(name);
    object->set_type(type);

    LOG_INFO("GEM object created: " + type +
             " (" + name + ")");

    return object;
}

bool Runtime::initGEMObject(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return false;
    }

    bool success = object->init();

    if (success) {
        LOG_INFO("GEM object initialized: " + object->get_name().toStdString());
    } else {
        LOG_ERROR("GEM object initialization failed: " + object->get_name().toStdString());
    }

    return success;
}

Collection Runtime::updateGEMObject(GEM* object, const Time& currentTime) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return Collection();
    }

    Collection result = object->update(currentTime);

    LOG_DEBUG("GEM object updated: " + object->get_name().toStdString());

    return result;
}

bool Runtime::resetGEMObject(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return false;
    }

    bool success = object->reset();

    if (success) {
        LOG_INFO("GEM object reset: " + object->get_name().toStdString());
    } else {
        LOG_ERROR("GEM object reset failed: " + object->get_name().toStdString());
    }

    return success;
}

Collection Runtime::getGEMMetrics(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return Collection();
    }

    return object->get_metrics();
}

Collection Runtime::getGEMUserMetrics(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return Collection();
    }

    return object->metrics();
}

void Runtime::setGEMParams(GEM* object, const Collection& params) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return;
    }

    object->set_params(params);

    LOG_DEBUG("GEM parameters set: " + object->get_name().toStdString());
}

Collection Runtime::getGEMParams(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return Collection();
    }

    return object->get_params();
}

void Runtime::publishGEMObject(GEM* object, const Collection& doc) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return;
    }

    object->publish(doc);

    LOG_INFO("GEM object published: " + object->get_name().toStdString());
}

Collection Runtime::storeGEMObject(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return Collection();
    }

    Collection state = object->store();

    LOG_INFO("GEM object state stored: " + object->get_name().toStdString());

    return state;
}

void Runtime::restoreGEMObject(GEM* object, const Collection& state) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return;
    }

    object->restore(state);

    LOG_INFO("GEM object state restored: " + object->get_name().toStdString());
}

void Runtime::showGEMObject(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return;
    }

    object->show();

    LOG_DEBUG("GEM object shown: " + object->get_name().toStdString());
}

void Runtime::destroyGEMObject(GEM* object) {
    if (!object) {
        LOG_ERROR("Null GEM object");
        return;
    }

    LOG_INFO("GEM object destroyed: " + object->get_name().toStdString());

    delete object;
}

// ============================================================================
// Type check operators
// ============================================================================

std::string Runtime::normalizeTypeName(const std::string& typeName) {
    std::string normalized = typeName;

    // Удаление пробелов
    normalized.erase(std::remove(normalized.begin(), normalized.end(), ' '), normalized.end());

    // Приведение к нижнему регистру для сравнения
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);

    return normalized;
}

} // namespace proxima
