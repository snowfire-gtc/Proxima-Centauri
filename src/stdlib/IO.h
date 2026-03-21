#ifndef PROXIMA_STDIO_H
#define PROXIMA_STDIO_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace proxima {
namespace stdlib {

class File {
public:
    File();
    File(const std::string& path);
    ~File();
    
    bool open(const std::string& path, const std::string& mode = "r");
    void close();
    bool isOpen() const { return file.is_open(); }
    
    std::string read();
    std::string readLine();
    std::vector<std::string> readLines();
    
    void write(const std::string& data);
    void writeLine(const std::string& data);
    void writeLines(const std::vector<std::string>& lines);
    
    void seek(size_t position);
    size_t position() const;
    size_t size() const;
    bool eof() const;
    
    std::string path() const { return filePath; }
    std::string mode() const { return fileMode; }
    
    static bool exists(const std::string& path);
    static bool remove(const std::string& path);
    static bool copy(const std::string& src, const std::string& dst);
    static bool move(const std::string& src, const std::string& dst);
    
private:
    std::fstream file;
    std::string filePath;
    std::string fileMode;
};

class Console {
public:
    static void print(const std::string& message);
    static void print(const std::string& format, const std::vector<std::string>& args);
    static void println(const std::string& message);
    
    static std::string read();
    static std::string readLine();
    
    static void clear();
    static void setCursorPosition(int x, int y);
    static void setTextColor(const std::string& color);
    static void resetTextColor();
};

class Serializer {
public:
    static std::string serialize(const void* data, size_t size);
    static void* deserialize(const std::string& data, size_t& size);
    
    template<typename T>
    static std::string serializeValue(const T& value) {
        std::ostringstream oss;
        oss.write(reinterpret_cast<const char*>(&value), sizeof(T));
        return oss.str();
    }
    
    template<typename T>
    static T deserializeValue(const std::string& data) {
        T value;
        std::istringstream iss(data);
        iss.read(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
    }
};

} // namespace stdlib
} // namespace proxima

#endif // PROXIMA_STDIO_H