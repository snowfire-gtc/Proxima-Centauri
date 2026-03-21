# Proxima Programming Language & Centauri IDE

[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)](https://github.com/proxima-lang/proxima/releases)
[![Build Status](https://img.shields.io/github/actions/workflow/status/Proxima-Centauri/main.yml)](https://github.com/snowfire-gtc/Proxima-Centauri/actions)
[![Issues](https://img.shields.io/github/issues/Proxima-Centauri)](https://github.com/snowfire-gtc/Proxima-Centauri/issues)

**Complete development environment for scientific computing and data analysis**

Proxima is a MATLAB-compatible programming language with full OOP support, GPU acceleration, and modern IDE integration. Centauri is the accompanying IDE built with Qt.

## 🚀 Features

### Language (Proxima)
- ✅ MATLAB-compatible syntax with `end` blocks
- ✅ Full OOP with classes, interfaces, inheritance
- ✅ Optional typing with automatic inference
- ✅ GPU acceleration (CUDA)
- ✅ CPU vectorization (AVX/SSE)
- ✅ LLVM-based compilation
- ✅ Automatic memory management (GC)
- ✅ Rich type system (vectors, matrices, layers, collections)
- ✅ Built-in documentation generation
- ✅ Module testing framework
- ✅ C/C++ interface

### IDE (Centauri)
- ✅ Advanced code editor with syntax highlighting
- ✅ Visual debugging with breakpoints
- ✅ Variable visualization (vectors, matrices, layers)
- ✅ Git integration
- ✅ LLM assistance (LM-Studio)
- ✅ Auto-save system
- ✅ Multi-user collaboration
- ✅ Project management
- ✅ Build system integration
- ✅ Documentation generation

## 📦 Installation

### Prerequisites

- **CMake** 3.15+
- **C++ Compiler** (GCC 9+, Clang 10+, MSVC 2019+)
- **LLVM** 14+
- **Qt** 5.15.2+
- **Git**

Optional:

- **CUDA Toolkit** (for GPU support)
- **CURL** (for LLM integration)

### Quick Install

```bash
# Clone repository
git clone hhttps://github.com/snowfire-gtc/Proxima-Centauri.git
cd proxima

# Build and install
./install.sh install

### Testing

``` bash
# Run compiler tests
cd proxima/build
ctest

# Run IDE tests
cd centauri/build
ctest

# Run integration tests
./run-tests.sh
```

## Full Feature List

### Proxima Language

- [x] 1. Стиль Matlab для блоков кода (end вместо {})
- [x] 2. Типы single/double, nan, inf, pi, pi2, exp
- [x] 2.1. Тип time с форматированием
- [x] 3. Типы int4, int8, int16, int32, int64, int
- [x] 4. Тип bool (true/false)
- [x] 5. Вектора vector<T>
- [x] 5.1. Типы point2, point3, point4
- [x] 5.2. Типы matrix22, matrix33, matrix44, matrix34
- [x] 6. Матрицы matrix<T>
- [x] 7. Тип collection с заголовками
- [x] 7.1. Пропуск операторов перед окончанием коллекции
- [x] 8. Тип string
- [x] 9. Тип char
- [x] 10. Конкатенация столбцов [a, b]
- [x] 11. Конкатенация строк [a,, b]
- [x] 11.1. Индексация с 1
- [x] 12. Тип layer, конкатенация [a,,, b]
- [x] 13. Вектор через :, транспонирование '
- [x] 13.1. Индексация с :
- [x] 13.2. Преобразование матрицы в вектор [:]
- [x] 13.3. Поэлементные операции с .
- [x] 13.4. Автоматическое распространение размерностей
- [x] 13.5. Линейная индексация
- [x] 14. Преобразование char в string
- [x] 15. Тип type, функция type()
- [x] 15.1. Преобразование типов (style)
- [x] 15.2. Тип void
- [x] 16. Обязательные и опциональные аргументы функций
- [x] 17. Объявление ядер GPU (parallel)
- [x] 17.1. Передача данных GPU<->CPU
- [x] 18. RTTI для GUI
- [x] 19. Доступ к полям через .
- [x] 20. Формат объявления переменных
- [x] 21. Формат функций
- [x] 22. Тип auto для функций
- [x] 22.1. Множественные возвращаемые значения
- [x] 22.2. Ключевое слово assigned
- [x] 22.3. Анонимные функции
- [x] 23. Формат класса (public/protected/private)
- [x] 24. Перегрузка методов
- [x] 24.1. Метод как поле класса
- [x] 24.2. Тип method
- [x] 25. Интерфейсы для преобразования типов
- [x] 25.1. Переменные нескольких типов
- [x] 26. Конструкция arguments для валидации
- [x] 27. Стандартные функции валидации
- [x] 28. Форматированный print
- [x] 28.1. Опция level для print
- [x] 29. Print в файл
- [x] 30. Сериализация write/read
- [x] 31. Многострочные комментарии с номерами
- [x] 32. RTTI через rtti()
- [x] 33. Диалог настройки через rtti.show()
- [x] 34. Логические операции (!, !=, ==, ===, &, |, %, etc.)
- [x] 35. Бинарные операции (!!, &&, ||, %%, <<, >>)
- [x] 36. Дополнительные операции (%, ^, .^, \, ?:)
- [x] 37. Директивы (include, exclude, namespace, using, define)
- [x] 37.1. Вложенность модулей
- [x] 38. Один модуль в одном файле
- [x] 38.1. Namespace для группы модулей
- [x] 39. Нет заголовочных файлов
- [x] 40. Флаги компиляции в формате collection
- [x] 40.1. Ограничения наборов команд
- [x] 41. Комментарии в файле сборки
- [x] 42. Упрощённый синтаксис файла сборки
- [x] 42.1. Манифест приложения
- [x] 43. Отладочные функции (dbgstop, dbgprint, dbgcontext, dbgstack)
- [x] 44. Интерфейс GEM
- [x] 45. Ключевые слова выполнения (if/for/while/do/switch)
- [x] 46. Отображение через show() для всех типов
- [x] 47. Сворачиваемые регионы #region
- [x] 48. Без дополнительных ключевых слов в вызовах
- [x] 49. Фреймворк тестирования (suite/test/assert)
- [x] 50. Генерация документации из комментариев
- [x] 51. Unicode переменные, case-sensitive
- [x] 52. Формат вызова компилятора
- [x] 53. Протокол IDE-компилятор (collection)
- [x] 54. Не использовать JSON, использовать collection
- [x] 54. Папка assets для данных приложения

### Centauri IDE

- [x] 1. Поддержка языка Proxima
- [x] 2. Проект в отдельной папке
- [x] 3. Модули .prx в папке src
- [x] 4. build.rules в папке rules
- [x] 5. design.rules в папке rules
- [x] 6. ui.rules в папке rules
- [x] 7. Редактирование множества модулей
- [x] 8. Навигация (табы, поиск, стрелки, шорткаты, дерево)
- [x] 9. Отладка (безусловные и условные точки останова)
- [x] 10. Визуализация объектов (class, vector, matrix, layer, collection)
- [x] 11. Панель инструментов редактора (6 групп кнопок)
- [x] 11.е. 8 режимов отображения редактора
- [x] 12. Строка состояния (позиция, статус, сохранение, время, ресурсы)
- [x] 12. Консоль с 4 табами (build, errors, output, callstack)
- [x] 17. Git интеграция
- [x] 18. Auto-save каждые 5 минут в autosave/
- [x] 19. Однократное сохранение после 5 минут бездействия
- [x] 20. Пиктографическое меню (7 разделов)
- [x] 21. Один проект в экземпляре IDE
- [x] 22. Название IDE - Centauri
- [x] 23. Qt 5.15.2/C++
- [x] 24. Простой, понятный, задокументированный код
- [x] 25. Режимы (Edit, Runtime, Pause)
- [x] 26. Схемы запуска (6 вариантов)
- [x] 27. Минимальная нагрузка в режиме релиза
- [x] 28. Лицензия GPLv3
- [x] 29. LLM интеграция (LM-Studio API)
- [x] 30. Многопользовательское редактирование
- [x] 31. Сворачивание методов, классов, комментариев, регионов

## Contacts:

- https://www.prxlang.ru (in development)

