#include "utils/DocGenerator.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"

using namespace proxima;

int main() {
    DocGenerator docGen;
    
    // Настройка
    docGen.setProjectName("MyProximaProject");
    docGen.setProjectVersion("1.0.0");
    docGen.setOutputDir("./docs");
    docGen.setFormat(DocFormat::HTML);
    docGen.setIncludeExamples(true);
    
    // Добавление исходных файлов
    docGen.addSourceDirectory("./src");
    
    // Компиляция и анализ
    Parser parser;
    SemanticAnalyzer analyzer;
    
    QFile file("./src/main.prx");
    if (file.open(QIODevice::ReadOnly)) {
        QString content = file.readAll();
        file.close();
        
        // Парсинг
        ProgramNodePtr ast = parser.parse(content);
        
        // Семантический анализ
        analyzer.analyze(ast);
        
        // Добавление в генератор документации
        docGen.addAST(ast, "main.prx");
        docGen.addSymbolTable(analyzer.getSymbolTable());
    }
    
    // Генерация
    if (docGen.generate()) {
        std::cout << "Documentation generated successfully!" << std::endl;
        
        // Статистика
        std::cout << "Modules: " << docGen.getModuleCount() << std::endl;
        std::cout << "Items: " << docGen.getItemCount() << std::endl;
        std::cout << "Functions: " << docGen.getFunctionCount() << std::endl;
        std::cout << "Classes: " << docGen.getClassCount() << std::endl;
    } else {
        std::cout << "Documentation generation failed!" << std::endl;
    }
    
    return 0;
}