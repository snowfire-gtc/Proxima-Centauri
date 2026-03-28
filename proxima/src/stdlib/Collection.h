#ifndef PROXIMA_COLLECTION_H
#define PROXIMA_COLLECTION_H

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>

namespace proxima {
namespace stdlib {

// Forward declaration
class Collection;

// Use a wrapper struct to allow recursive type definition
struct CollectionValue;

using CollectionValueArray = std::vector<CollectionValue>;

struct CollectionValue {
    using VariantType = std::variant<
        int64_t,
        double,
        std::string,
        bool,
        std::shared_ptr<CollectionValueArray>,
        std::shared_ptr<Collection>
    >;
    
    VariantType value;
    
    CollectionValue() : value(int64_t(0)) {}
    CollectionValue(int64_t v) : value(v) {}
    CollectionValue(double v) : value(v) {}
    CollectionValue(const std::string& v) : value(v) {}
    CollectionValue(const char* v) : value(std::string(v)) {}
    CollectionValue(bool v) : value(v) {}
    CollectionValue(const std::shared_ptr<CollectionValueArray>& v) : value(v) {}
    CollectionValue(const CollectionValueArray& v) : value(std::make_shared<CollectionValueArray>(v)) {}
    CollectionValue(const std::shared_ptr<Collection>& v) : value(v) {}
    
    // Helper methods for type checking
    bool isNumber() const { return std::holds_alternative<int64_t>(value) || std::holds_alternative<double>(value); }
    bool isString() const { return std::holds_alternative<std::string>(value); }
    bool isBoolean() const { return std::holds_alternative<bool>(value); }
    bool isArray() const { return std::holds_alternative<std::shared_ptr<CollectionValueArray>>(value); }
    bool isCollection() const { return std::holds_alternative<std::shared_ptr<Collection>>(value); }
    
    // Accessors
    int64_t asInt() const { 
        if (std::holds_alternative<int64_t>(value)) return std::get<int64_t>(value);
        if (std::holds_alternative<double>(value)) return static_cast<int64_t>(std::get<double>(value));
        return 0; 
    }
    double asDouble() const {
        if (std::holds_alternative<double>(value)) return std::get<double>(value);
        if (std::holds_alternative<int64_t>(value)) return static_cast<double>(std::get<int64_t>(value));
        return 0.0;
    }
    std::string asString() const { 
        return std::holds_alternative<std::string>(value) ? std::get<std::string>(value) : ""; 
    }
    bool asBool() const { 
        return std::holds_alternative<bool>(value) ? std::get<bool>(value) : false; 
    }
    const CollectionValueArray& asArray() const { 
        static const CollectionValueArray empty;
        return std::holds_alternative<std::shared_ptr<CollectionValueArray>>(value) 
            ? *std::get<std::shared_ptr<CollectionValueArray>>(value) 
            : empty;
    }
    std::shared_ptr<Collection> asCollection() const {
        return std::holds_alternative<std::shared_ptr<Collection>>(value) 
            ? std::get<std::shared_ptr<Collection>>(value) 
            : nullptr;
    }
};

class Collection {
public:
    Collection();
    Collection(const std::vector<std::string>& header);
    
    // Row operations
    void addRow(const std::vector<CollectionValue>& values);
    void removeRow(size_t index);
    void clearRows();
    
    // Column operations
    void addColumn(const std::string& name);
    void removeColumn(const std::string& name);
    void setHeader(const std::vector<std::string>& header);
    
    // Access
    CollectionValue get(size_t row, size_t col) const;
    CollectionValue get(size_t row, const std::string& colName) const;
    void set(size_t row, size_t col, const CollectionValue& value);
    void set(size_t row, const std::string& colName, const CollectionValue& value);
    
    // Properties
    size_t rowCount() const { return data.size(); }
    size_t colCount() const { return header.size(); }
    std::vector<std::string> getHeader() const { return header; }
    
    // File I/O
    bool readCSV(const std::string& filename);
    bool writeCSV(const std::string& filename) const;
    bool readJSON(const std::string& filename);
    bool writeJSON(const std::string& filename) const;
    
    // Conversion
    std::string toString() const;
    std::vector<std::vector<std::string>> toTable() const;
    
    // Query
    Collection filter(const std::string& condition) const;
    Collection sort(const std::string& columnName, bool ascending = true) const;
    Collection select(const std::vector<std::string>& columns) const;
    
private:
    std::vector<std::string> header;
    std::vector<std::shared_ptr<CollectionValueArray>> data;
    std::unordered_map<std::string, size_t> columnIndex;
    
    size_t getColumnIndex(const std::string& name) const;
};

} // namespace stdlib
} // namespace proxima

#endif // PROXIMA_COLLECTION_H