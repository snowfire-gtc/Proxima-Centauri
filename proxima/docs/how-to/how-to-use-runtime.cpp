#include "runtime/Runtime.h"

using namespace proxima;

int main() {
    Runtime runtime;
    
    // Инициализация
    runtime.initialize();
    runtime.setVerboseLevel(3);
    runtime.enableDebugMode(true);
    runtime.setMemoryLimit(2 * 1024 * 1024 * 1024); // 2GB
    
    // Информация о системе
    std::cout << "OS: " << runtime.getOSInfo() << std::endl;
    std::cout << "CPU: " << runtime.getCPUInfo() << std::endl;
    std::cout << "CPU Count: " << runtime.getCPUCount() << std::endl;
    std::cout << "Memory Free: " << runtime.getSystemMemoryFree() / 1024 / 1024 << " MB" << std::endl;
    std::cout << "Memory Total: " << runtime.getSystemMemoryTotal() / 1024 / 1024 << " MB" << std::endl;
    
    // Выделение памяти
    void* ptr = runtime.allocate(1024);
    if (ptr) {
        std::cout << "Allocated 1KB at " << ptr << std::endl;
        std::cout << "Memory usage: " << runtime.getMemoryUsage() << " bytes" << std::endl;
        runtime.deallocate(ptr);
    }
    
    // CUDA (если доступно)
    if (runtime.enableCUDA()) {
        void* gpuPtr = runtime.allocateGPU(1024);
        if (gpuPtr) {
            std::cout << "Allocated 1KB on GPU" << std::endl;
            runtime.freeGPU(gpuPtr);
        }
    }
    
    // Отладочные функции
    runtime.dbgprint("Debug message", 3);
    runtime.dbgcontext();
    
    // Параллельное выполнение
    std::vector<double> data(1000);
    runtime.parallelFor(0, 1000, 1, [&data](int i) {
        data[i] = i * 1.0;
    }, 4, data.data());
    
    // Таймеры
    runtime.startTimer("test");
    runtime.sleep(100);
    double elapsed = runtime.stopTimer("test");
    std::cout << "Elapsed: " << elapsed << " ms" << std::endl;
    
    // Статистика
    RuntimeStats stats = runtime.getStats();
    std::cout << "Uptime: " << stats.uptime << " ms" << std::endl;
    
    return 0;
}