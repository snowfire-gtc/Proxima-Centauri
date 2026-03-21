#include "Collection.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace proxima {
namespace stdlib {

Collection::Collection() {}

Collection::Collection(const std::vector<std::string>& hdr) {
    setHeader(hdr);
}

void Collection::addRow(const std::vector<CollectionValue>& values) {
    if (values.size() != header.size()) {
        throw std::runtime_error("Column count mismatch");
    }
    data.push_back(values);
}

void Collection::removeRow(size_t index) {
    if (index < data.size()) {
        data.erase(data.begin() + index);
    }
}

void Collection::clearRows() {
    data.clear();
}

void Collection::addColumn(const std::string& name) {
    header.push_back(name);
    columnIndex[name] = header.size() - 1;
    
    // Add null values to existing rows
    for (auto& row : data) {
        row.push_back(int64_t(0));
    }
}

void Collection::removeColumn(const std::string& name) {
    auto it = columnIndex.find(name);
    if (it != columnIndex.end()) {
        size_t index = it->second;
        header.erase(header.begin() + index);
        
        for (auto& row : data) {
            row.erase(row.begin() + index);
        }
        
        // Rebuild index
        columnIndex.clear();
        for (size_t i = 0; i < header.size(); i++) {
            columnIndex[header[i]] = i;
        }
    }
}

void Collection::setHeader(const std::vector<std::string>& hdr) {
    header = hdr;
    columnIndex.clear();
    for (size_t i = 0; i < header.size(); i++) {
        columnIndex[header[i]] = i;
    }
}

CollectionValue Collection::get(size_t row, size_t col) const {
    if (row < data.size() && col < header.size()) {
        return data[row][col];
    }
    return int64_t(0);
}

CollectionValue Collection::get(size_t row, const std::string& colName) const {
    size_t col = getColumnIndex(colName);
    return get(row, col);
}

void Collection::set(size_t row, size_t col, const CollectionValue& value) {
    if (row < data.size() && col < header.size()) {
        data[row][col] = value;
    }
}

void Collection::set(size_t row, const std::string& colName, const CollectionValue& value) {
    size_t col = getColumnIndex(colName);
    set(row, col, value);
}

size_t Collection::getColumnIndex(const std::string& name) const {
    auto it = columnIndex.find(name);
    if (it != columnIndex.end()) {
        return it->second;
    }
    throw std::runtime_error("Column not found: " + name);
}

bool Collection::readCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    
    std::string line;
    bool firstLine = true;
    
    while (std::getline(file, line)) {
        std::vector<CollectionValue> row;
        std::stringstream ss(line);
        std::string cell;
        
        while (std::getline(ss, cell, ',')) {
            // Try to parse as number
            try {
                size_t pos;
                double num = std::stod(cell, &pos);
                if (pos == cell.length()) {
                    row.push_back(num);
                } else {
                    row.push_back(cell);
                }
            } catch (...) {
                row.push_back(cell);
            }
        }
        
        if (firstLine) {
            // First line is header
            std::vector<std::string> hdr;
            for (const auto& val : row) {
                if (std::holds_alternative<std::string>(val)) {
                    hdr.push_back(std::get<std::string>(val));
                }
            }
            setHeader(hdr);
            firstLine = false;
        } else {
            addRow(row);
        }
    }
    
    return true;
}

bool Collection::writeCSV(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;
    
    // Write header
    for (size_t i = 0; i < header.size(); i++) {
        file << header[i];
        if (i < header.size() - 1) file << ",";
    }
    file << "\n";
    
    // Write data
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); i++) {
            if (std::holds_alternative<std::string>(row[i])) {
                file << std::get<std::string>(row[i]);
            } else if (std::holds_alternative<double>(row[i])) {
                file << std::get<double>(row[i]);
            } else if (std::holds_alternative<int64_t>(row[i])) {
                file << std::get<int64_t>(row[i]);
            }
            if (i < row.size() - 1) file << ",";
        }
        file << "\n";
    }
    
    return true;
}

std::string Collection::toString() const {
    std::ostringstream oss;
    
    // Header
    for (size_t i = 0; i < header.size(); i++) {
        oss << std::setw(15) << header[i];
    }
    oss << "\n";
    
    // Separator
    for (size_t i = 0; i < header.size(); i++) {
        oss << std::setw(15) << "---------------";
    }
    oss << "\n";
    
    // Data
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); i++) {
            std::string cell;
            if (std::holds_alternative<std::string>(row[i])) {
                cell = std::get<std::string>(row[i]);
            } else if (std::holds_alternative<double>(row[i])) {
                cell = std::to_string(std::get<double>(row[i]));
            } else if (std::holds_alternative<int64_t>(row[i])) {
                cell = std::to_string(std::get<int64_t>(row[i]));
            }
            oss << std::setw(15) << cell;
        }
        oss << "\n";
    }
    
    return oss.str();
}

std::vector<std::vector<std::string>> Collection::toTable() const {
    std::vector<std::vector<std::string>> table;
    
    // Add header
    table.push_back(header);
    
    // Add data
    for (const auto& row : data) {
        std::vector<std::string> stringRow;
        for (const auto& val : row) {
            if (std::holds_alternative<std::string>(val)) {
                stringRow.push_back(std::get<std::string>(val));
            } else if (std::holds_alternative<double>(val)) {
                stringRow.push_back(std::to_string(std::get<double>(val)));
            } else if (std::holds_alternative<int64_t>(val)) {
                stringRow.push_back(std::to_string(std::get<int64_t>(val)));
            } else {
                stringRow.push_back("");
            }
        }
        table.push_back(stringRow);
    }
    
    return table;
}

Collection Collection::filter(const std::string& condition) const {
    // TODO: Implement SQL-like filtering
    return Collection(header);
}

Collection Collection::sort(const std::string& columnName, bool ascending) const {
    Collection result(header);
    result.data = data;
    
    size_t col = getColumnIndex(columnName);
    
    std::sort(result.data.begin(), result.data.end(),
        [col, ascending](const std::vector<CollectionValue>& a, 
                        const std::vector<CollectionValue>& b) {
            if (ascending) {
                return a[col] < b[col];
            } else {
                return a[col] > b[col];
            }
        });
    
    return result;
}

Collection Collection::select(const std::vector<std::string>& columns) const {
    Collection result(columns);
    
    for (const auto& row : data) {
        std::vector<CollectionValue> newRow;
        for (const auto& col : columns) {
            size_t colIndex = getColumnIndex(col);
            newRow.push_back(row[colIndex]);
        }
        result.addRow(newRow);
    }
    
    return result;
}

} // namespace stdlib
} // namespace proxima