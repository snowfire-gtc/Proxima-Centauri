#include "IO.h"
#include <iostream>
#include <iomanip>
#include <regex>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace proxima {
namespace stdlib {

File::File() {}

File::File(const std::string& path) {
    open(path);
}

File::~File() {
    close();
}

bool File::open(const std::string& path, const std::string& mode) {
    close();
    
    filePath = path;
    fileMode = mode;
    
    std::ios_base::openmode openMode = std::ios_base::in;
    
    if (mode.find("w") != std::string::npos) {
        openMode = std::ios_base::out | std::ios_base::trunc;
    } else if (mode.find("a") != std::string::npos) {
        openMode = std::ios_base::out | std::ios_base::app;
    } else if (mode.find("r+") != std::string::npos || 
               mode.find("w+") != std::string::npos) {
        openMode = std::ios_base::in | std::ios_base::out;
    }
    
    if (mode.find("b") != std::string::npos) {
        openMode |= std::ios_base::binary;
    }
    
    file.open(path, openMode);
    return file.is_open();
}

void File::close() {
    if (file.is_open()) {
        file.close();
    }
}

std::string File::read() {
    if (!file.is_open()) return "";
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::string File::readLine() {
    if (!file.is_open()) return "";
    
    std::string line;
    std::getline(file, line);
    return line;
}

std::vector<std::string> File::readLines() {
    std::vector<std::string> lines;
    
    if (!file.is_open()) return lines;
    
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

void File::write(const std::string& data) {
    if (!file.is_open()) return;
    file << data;
}

void File::writeLine(const std::string& data) {
    if (!file.is_open()) return;
    file << data << std::endl;
}

void File::writeLines(const std::vector<std::string>& lines) {
    for (const auto& line : lines) {
        writeLine(line);
    }
}

void File::seek(size_t position) {
    if (!file.is_open()) return;
    file.seekg(position);
}

size_t File::position() const {
    if (!file.is_open()) return 0;
    return file.tellg();
}

size_t File::size() const {
    if (!file.is_open()) return 0;
    
    std::streampos current = file.tellg();
    file.seekg(0, std::ios_base::end);
    std::streampos end = file.tellg();
    file.seekg(current);
    
    return static_cast<size_t>(end);
}

bool File::eof() const {
    return file.eof();
}

bool File::exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool File::remove(const std::string& path) {
#ifdef _WIN32
    return _unlink(path.c_str()) == 0;
#else
    return unlink(path.c_str()) == 0;
#endif
}

bool File::copy(const std::string& src, const std::string& dst) {
    File inFile(src);
    if (!inFile.isOpen()) return false;
    
    File outFile(dst, "w");
    if (!outFile.isOpen()) return false;
    
    outFile.write(inFile.read());
    return true;
}

bool File::move(const std::string& src, const std::string& dst) {
    if (copy(src, dst)) {
        return remove(src);
    }
    return false;
}

void Console::print(const std::string& message) {
    std::cout << message;
}

void Console::print(const std::string& format, const std::vector<std::string>& args) {
    std::string result = format;
    
    // Simple format string replacement
    size_t argIndex = 0;
    size_t pos = 0;
    
    while ((pos = result.find("%", pos)) != std::string::npos && 
           argIndex < args.size()) {
        if (pos + 1 < result.length()) {
            char specifier = result[pos + 1];
            std::string replacement;
            
            switch (specifier) {
                case 's':
                    replacement = args[argIndex++];
                    result.replace(pos, 2, replacement);
                    break;
                case 'd':
                case 'i':
                    replacement = args[argIndex++];
                    result.replace(pos, 2, replacement);
                    break;
                case 'f':
                    replacement = args[argIndex++];
                    result.replace(pos, 2, replacement);
                    break;
                default:
                    pos++;
                    break;
            }
        } else {
            pos++;
        }
    }
    
    std::cout << result;
}

void Console::println(const std::string& message) {
    std::cout << message << std::endl;
}

std::string Console::read() {
    std::string input;
    std::cin >> input;
    return input;
}

std::string Console::readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void Console::clear() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Console::setCursorPosition(int x, int y) {
#ifdef _WIN32
    // Windows implementation
#else
    std::cout << "\033[" << y << ";" << x << "H";
#endif
}

void Console::setTextColor(const std::string& color) {
    // ANSI color codes
    if (color == "red") {
        std::cout << "\033[31m";
    } else if (color == "green") {
        std::cout << "\033[32m";
    } else if (color == "yellow") {
        std::cout << "\033[33m";
    } else if (color == "blue") {
        std::cout << "\033[34m";
    }
}

void Console::resetTextColor() {
    std::cout << "\033[0m";
}

std::string Serializer::serialize(const void* data, size_t size) {
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(data), size);
    return oss.str();
}

void* Serializer::deserialize(const std::string& data, size_t& size) {
    size = data.length();
    void* buffer = std::malloc(size);
    std::memcpy(buffer, data.c_str(), size);
    return buffer;
}

} // namespace stdlib
} // namespace proxima