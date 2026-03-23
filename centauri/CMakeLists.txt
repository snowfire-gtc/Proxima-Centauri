cmake_minimum_required(VERSION 3.15)
project(CentauriIDE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Найти Qt 5.15.2
find_package(Qt5 5.15.2 REQUIRED COMPONENTS Widgets Core Network Gui)

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
    src/ui/console/ConsoleWidget.cpp
    src/ui/console/ConsoleTab.cpp
    src/ui/project/ProjectTree.cpp
    src/ui/project/FileExplorer.cpp
    src/ui/debugger/BreakpointManager.cpp
    src/ui/debugger/VariableInspector.cpp
    src/ui/debugger/visualizers/VectorPlot.cpp
    src/ui/debugger/visualizers/MatrixView.cpp
    src/ui/debugger/visualizers/Layer3DView.cpp
    src/ui/debugger/visualizers/CollectionTable.cpp
    src/ui/menu/MainMenu.cpp
    src/core/Project.cpp
    src/core/Module.cpp
    src/core/AutoSaveManager.cpp
    src/core/CompilerConnector.cpp
    src/services/git/GitService.cpp
    src/services/llm/LLMService.cpp
    src/services/collaboration/CollaborationService.cpp
    src/utils/Logger.cpp
    src/utils/Config.cpp
    src/utils/Helpers.cpp
)

set(IDE_HEADERS
    src/app/Application.h
    src/app/MainWindow.h
    src/app/Settings.h
    src/ui/editor/CodeEditor.h
    src/ui/editor/SyntaxHighlighter.h
    src/ui/editor/EditorToolbar.h
    src/ui/editor/StatusBar.h
    src/ui/console/ConsoleWidget.h
    src/ui/console/ConsoleTab.h
    src/ui/project/ProjectTree.h
    src/ui/project/FileExplorer.h
    src/ui/debugger/BreakpointManager.h
    src/ui/debugger/VariableInspector.h
    src/ui/debugger/visualizers/VectorPlot.h
    src/ui/debugger/visualizers/MatrixView.h
    src/ui/debugger/visualizers/Layer3DView.h
    src/ui/debugger/visualizers/CollectionTable.h
    src/ui/menu/MainMenu.h
    src/core/Project.h
    src/core/Module.h
    src/core/AutoSaveManager.h
    src/core/CompilerConnector.h
    src/services/git/GitService.h
    src/services/llm/LLMService.h
    src/services/collaboration/CollaborationService.h
    src/services/plugins/PluginManager.h
    src/utils/Logger.h
    src/utils/Config.h
    src/utils/Helpers.h
    docs/DocGenerator.h
)

set(IDE_RESOURCES
    resources/icons.qrc
    resources/themes.qrc
    resources/translations.qrc
)

# Создать исполняемый файл IDE
add_executable(centauri WIN32 ${IDE_SOURCES} ${IDE_HEADERS} ${IDE_RESOURCES})

target_link_libraries(centauri
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Gui
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
install(DIRECTORY resources/ DESTINATION share/centauri/resources)

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
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/resources/icons/centauri.ico")
include(CPack)
