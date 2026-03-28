cmake_minimum_required(VERSION 3.16)
project(CentauriIDE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Найти Qt 6
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Network Gui)

# Найти Proxima Compiler (опционально)
find_package(ProximaCompiler QUIET)

# Исходные файлы IDE
set(IDE_SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/app/MainWindow.cpp
    src/app/Settings.cpp
    src/ui/editor/CodeEditor.cpp
    src/ui/editor/SyntaxHighlighter.cpp
    src/ui/editor/EditorToolbar.cpp
    src/ui/editor/StatusBar.cpp
    src/ui/editor/CodeFoldingManager.cpp
    src/ui/editor/CompletionProvider.cpp
    src/ui/editor/SnippetManager.cpp
    src/ui/console/ConsoleWidget.cpp
    src/ui/console/ConsoleTab.cpp
    src/ui/console/REPLConsoleTab.cpp
    src/ui/console/REPLSyntaxHighlighter.cpp
    src/ui/dialogs/SearchWidget.cpp
    src/ui/dialogs/AdvancedSearchWidget.cpp
    src/ui/dialogs/BuildDialog.cpp
    src/ui/dialogs/LLMAssistantDialog.cpp
    src/ui/dialogs/SettingsDialog.cpp
    src/ui/dialogs/BuildReportDialog.cpp
    src/ui/dialogs/CollaborationDialog.cpp
    src/ui/dialogs/CompatibilityReportDialog.cpp
    src/ui/dialogs/FoldingSettingsDialog.cpp
    src/ui/dialogs/SnippetManagerDialog.cpp
    src/ui/menu/MainMenu.cpp
    src/ui/debugger/visualizers/VectorPlot.cpp
    src/ui/debugger/visualizers/MatrixView.cpp
    src/ui/debugger/visualizers/Layer3DView.cpp
    src/ui/debugger/visualizers/CollectionTable.cpp
    src/ui/debugger/visualizers/ObjectInspector.cpp
    src/core/Project.cpp
    src/core/Module.cpp
    src/core/AutoSaveManager.cpp
    src/core/CompilerConnector.cpp
    src/services/git/GitService.cpp
    src/services/llm/LLMService.cpp
    src/services/collaboration/CollaborationService.cpp
    src/services/network/NetworkAuthService.cpp
    src/services/plugins/PluginManager.cpp
    src/docs/DocGenerator.cpp
    src/testing/TestFramework.cpp
)

set(IDE_HEADERS
    src/app/MainWindow.h
    src/app/Settings.h
    src/ui/editor/CodeEditor.h
    src/ui/editor/SyntaxHighlighter.h
    src/ui/editor/EditorToolbar.h
    src/ui/editor/StatusBar.h
    src/ui/editor/CodeFoldingManager.h
    src/ui/editor/CompletionProvider.h
    src/ui/editor/SnippetManager.h
    src/ui/console/ConsoleWidget.h
    src/ui/console/ConsoleTab.h
    src/ui/console/REPLConsoleTab.h
    src/ui/console/REPLSyntaxHighlighter.h
    src/ui/dialogs/SearchWidget.h
    src/ui/dialogs/AdvancedSearchWidget.h
    src/ui/dialogs/BuildDialog.h
    src/ui/dialogs/LLMAssistantDialog.h
    src/ui/dialogs/SettingsDialog.h
    src/ui/dialogs/BuildReportDialog.h
    src/ui/dialogs/CollaborationDialog.h
    src/ui/dialogs/CompatibilityReportDialog.h
    src/ui/dialogs/FoldingSettingsDialog.h
    src/ui/dialogs/SnippetManagerDialog.h
    src/ui/menu/MainMenu.h
    src/ui/debugger/BreakpointManager.h
    src/ui/debugger/VariableInspector.h
    src/ui/debugger/visualizers/VectorPlot.h
    src/ui/debugger/visualizers/MatrixView.h
    src/ui/debugger/visualizers/Layer3DView.h
    src/ui/debugger/visualizers/CollectionTable.h
    src/core/Project.h
    src/core/Module.h
    src/core/AutoSaveManager.h
    src/core/CompilerConnector.h
    src/services/git/GitService.h
    src/services/llm/LLMService.h
    src/services/collaboration/CollaborationService.h
    src/services/network/NetworkAuthService.h
    src/services/plugins/PluginManager.h
    src/docs/DocGenerator.h
    src/testing/TestFramework.h
)

# Создать исполняемый файл IDE
add_executable(centauri ${IDE_SOURCES} ${IDE_HEADERS})

target_link_libraries(centauri
    Qt6::Widgets
    Qt6::Core
    Qt6::Network
    Qt6::Gui
)

if(ProximaCompiler_FOUND)
    target_link_libraries(centauri Proxima::Compiler)
    target_compile_definitions(centauri PRIVATE HAVE_PROXIMA_COMPILER)
endif()

target_include_directories(centauri PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../proxima-compiler/src
)

# Установка
install(TARGETS centauri DESTINATION bin)

# Создание ярлыков (Windows)
if(WIN32)
    set_target_properties(centauri PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# macOS bundle
if(APPLE)
    set_target_properties(centauri PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.proxima.centauri"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
endif()

# CPack для пакетирования
set(CPACK_PACKAGE_NAME "Centauri IDE")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IDE for Proxima Programming Language")
set(CPACK_PACKAGE_VENDOR "Proxima Development Team")
include(CPack)
