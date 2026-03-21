#include "services/llm/LLMClient.h"

using namespace proxima;

int main() {
    LLMClient llm;
    
    // Настройка
    llm.setServerURL("http://localhost:1234");
    llm.setModel("codellama-7b");
    llm.setMaxTokens(2048);
    llm.setTemperature(0.7);
    llm.setTimeout(30000);
    
    // Проверка соединения
    if (!llm.checkConnection()) {
        std::cout << "Failed to connect to LLM server" << std::endl;
        return 1;
    }
    
    // Загрузка настроек
    llm.loadSettings();
    
    // Пример 1: Объяснение кода
    QString code = R"(
    main() : int32
        print("Hello, Proxima!\n");
        return 0;
    end
    )";
    
    QString explanation = llm.explainCode("main.prx", 1, 5, code);
    std::cout << "Explanation: " << explanation.toStdString() << std::endl;
    
    // Пример 2: Предложения по модификации
    QVector<CodeSuggestion> suggestions = llm.suggestModifications(
        "main.prx",
        1,
        5,
        code,
        "Optimize this code for performance"
    );
    
    std::cout << "Got " << suggestions.size() << " suggestions" << std::endl;
    
    for (const CodeSuggestion& sugg : suggestions) {
        std::cout << "Suggestion " << sugg.blockId << ":" << std::endl;
        std::cout << "  Explanation: " << sugg.explanation.toStdString() << std::endl;
        std::cout << "  Confidence: " << sugg.confidence << std::endl;
        std::cout << "  Code: " << sugg.suggestedCode.toStdString() << std::endl;
    }
    
    // Пример 3: Применение изменений
    if (!suggestions.isEmpty()) {
        llm.applySuggestion(suggestions[0].id);
        
        QVector<CodeSuggestion> accepted = llm.getAcceptedSuggestions();
        std::cout << "Accepted " << accepted.size() << " suggestions" << std::endl;
    }
    
    // Пример 4: Автодополнение
    QString prefix = "function add(a, b)";
    QString completion = llm.completeCode("utils.prx", 1, 0, prefix);
    std::cout << "Completion: " << completion.toStdString() << std::endl;
    
    // Пример 5: Генерация тестов
    LLMResponse testResponse = llm.generateTests("math.prx", 1, 50, code);
    if (testResponse.success) {
        std::cout << "Tests generated successfully" << std::endl;
    }
    
    // Сохранение настроек
    llm.saveSettings();
    
    // Очистка кэша
    llm.clearCache();
    
    return 0;
}