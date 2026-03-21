// Пример интеграции отладчика в Runtime
#include "runtime/Debugger.h"

using namespace proxima;

int main() {
    Debugger debugger;
    
    // Настройка
    debugger.setVerboseLevel(3);
    debugger.setMaxMemory(4 * 1024 * 1024 * 1024);
    
    // Добавление точек останова
    debugger.addBreakpoint("main.prx", 42, BreakpointType::UNCONDITIONAL);
    debugger.addBreakpoint("utils.prx", 15, BreakpointType::CONDITIONAL, "x > 100");
    
    // Callbacks
    debugger.onBreakpoint([](int id) {
        std::cout << "Breakpoint " << id << " hit!" << std::endl;
    });
    
    debugger.onPause([]() {
        std::cout << "Debugger paused" << std::endl;
    });
    
    // Запуск отладки
    debugger.start();
    
    // Эмуляция выполнения
    debugger.notifyFunctionEntry("main", "main.prx", 1);
    debugger.notifyLineExecution("main.prx", 10);
    debugger.notifyVariableChange("x", "50");
    debugger.notifyLineExecution("main.prx", 42);  // Точка останова
    debugger.notifyFunctionExit("main");
    
    // Получение состояния
    std::cout << "Current location: " << debugger.getCurrentFile() 
              << ":" << debugger.getCurrentLine() << std::endl;
    
    std::cout << "Variables:" << std::endl;
    auto vars = debugger.getLocalVariables();
    for (const auto& var : vars) {
        std::cout << "  " << var.first << " = " << var.second << std::endl;
    }
    
    // Сериализация для IDE
    std::string state = debugger.serializeState();
    std::cout << "State: " << state << std::endl;
    
    debugger.stop();
    
    return 0;
}