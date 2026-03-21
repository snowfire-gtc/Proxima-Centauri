#ifndef PROXIMA_COLLECTION_H
#define PROXIMA_COLLECTION_H

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>

namespace proxima {
namespace stdlib {

using CollectionValue = std::variant<
    int64_t,
    double,
    std::string,
    bool,
    std::vector<CollectionValue>,
    std::shared_ptr<class Collection>
>;

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
    std::vector<std::vector<CollectionValue>> data;
    std::unordered_map<std::string, size_t> columnIndex;
    
    size_t getColumnIndex(const std::string& name) const;
};

} // namespace stdlib
} // namespace proxima

#endif // PROXIMA_COLLECTION_H