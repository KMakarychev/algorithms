#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <ratio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace TestFramework {

//============================================================
// Version
//============================================================
constexpr int GetTestFrameworkVersion() { return 110; }

//============================================================
// Exceptions
//============================================================
struct TestFrameworkError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct ParseError : TestFrameworkError {
    std::string file;
    size_t line = 0;

    static std::string Compose(std::string_view msg, std::string_view file_, size_t line_) {
        std::string out;
        out.reserve(msg.size() + file_.size() + 32);
        if (!file_.empty()) {
            out.append(file_);
            out.push_back(':');
        }
        if (line_ != 0) {
            out.append(std::to_string(line_));
            out.append(": ");
        }
        out.append(msg);
        return out;
    }

    ParseError(std::string_view msg, std::string file_, size_t line_)
        : TestFrameworkError(Compose(msg, file_, line_)), file(std::move(file_)), line(line_) {}
};

//============================================================
// Safe <cctype> helpers
//============================================================
inline bool tf_isspace(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}
inline bool tf_isdigit(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) != 0;
}
inline char tf_tolower(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

//============================================================
// Error helpers (throwing exceptions)
//============================================================
[[noreturn]] inline void ThrowError(std::string_view msg) {
    throw TestFrameworkError(std::string(msg));
}

inline void ThrowIfConditionFails(bool condition, std::string_view error) {
    if (!condition) throw TestFrameworkError(std::string(error));
}

inline void ThrowIfConditionFails(bool condition, std::string_view error, std::string_view debugMsg) {
    if (condition) return;
    std::string out;
    out.reserve(error.size() + debugMsg.size() + 32);
    out.append(error);
    out.append(" Debug message: <");
    out.append(debugMsg);
    out.push_back('>');
    throw TestFrameworkError(out);
}

//============================================================
// Clock
//============================================================
using Clock = std::chrono::steady_clock;

//============================================================
// class StringSegment
//============================================================
class StringSegment {
public:
    StringSegment() = default;
    explicit StringSegment(std::string_view v) : view(v) {}
    explicit StringSegment(const std::string& s) : view(s) {}

    StringSegment(const std::string& s, size_t begin, size_t end)
        : view(std::string_view(s).substr(begin, end - begin)) {}

    // Copy out
    void CopyTo(std::string& dest) const { dest.assign(view.data(), view.size()); }

    bool CopyTo(char* buffer, size_t buffer_size) const {
        if (buffer_size == 0) return false;
        const size_t count = std::min(view.size(), buffer_size - 1);
        for (size_t i = 0; i < count; ++i) buffer[i] = view[i];
        buffer[count] = '\0';
        return count == view.size();
    }

    std::string ToString() const { return std::string(view); }

    size_t CountChars(char c) const {
        return static_cast<size_t>(std::count(view.begin(), view.end(), c));
    }

    bool IsEmpty() const noexcept { return view.empty(); }
    size_t Length() const noexcept { return view.size(); }

    // Read chars (mutating)
    char ReadLeft() {
        if (view.empty()) return '\0';
        char c = view.front();
        view.remove_prefix(1);
        return c;
    }

    char ReadRight() {
        if (view.empty()) return '\0';
        char c = view.back();
        view.remove_suffix(1);
        return c;
    }

    char FirstChar() const {
        assert(!view.empty());
        return view.front();
    }

    char LastChar() const {
        assert(!view.empty());
        return view.back();
    }

    void RemovePrefix(size_t count) { view.remove_prefix(std::min(count, view.size())); }
    void RemoveSuffix(size_t count) { view.remove_suffix(std::min(count, view.size())); }

    void Trim() { TrimLeft(); TrimRight(); }

    void TrimLeft() {
        while (!view.empty() && (tf_isspace(view.front()) || view.front() == '\r')) {
            view.remove_prefix(1);
        }
    }

    void TrimRight() {
        while (!view.empty() && (tf_isspace(view.back()) || view.back() == '\r')) {
            view.remove_suffix(1);
        }
    }

    bool Match(const char* pattern, bool caseSensitive = false) const {
        if (!pattern) return false;
        const std::string_view p(pattern);
        if (p.size() != view.size()) return false;

        if (caseSensitive) return p == view;

        for (size_t i = 0; i < view.size(); ++i) {
            if (tf_tolower(p[i]) != tf_tolower(view[i])) return false;
        }
        return true;
    }

    bool Match(const StringSegment& pattern, bool caseSensitive = false) const {
        if (pattern.view.size() != view.size()) return false;
        if (caseSensitive) return pattern.view == view;

        for (size_t i = 0; i < view.size(); ++i) {
            if (tf_tolower(pattern.view[i]) != tf_tolower(view[i])) return false;
        }
        return true;
    }

    // Split at first occurrence of delimiter
    // prefix is the chunk before delimiter, and 
    // *this becomes after delimiter (or empty if no delimiter).
    bool Split(char delimiter, StringSegment& prefix) {
        if (view.empty()) {
            prefix.view = {};
            return false;
        }
        const size_t pos = view.find(delimiter);
        if (pos == std::string_view::npos) {
            prefix.view = view;
            view = {};
            return true;
        }
        prefix.view = view.substr(0, pos);
        view.remove_prefix(pos + 1);
        return true;
    }

    char operator[](size_t index) const {
        assert(index < view.size());
        return view[index];
    }

    std::string_view AsView() const noexcept { return view; }

private:
    std::string_view view{};
};

//============================================================
// Encoding helpers
//============================================================
inline size_t IntLen(int value) {
    if (value == 0) return 1;
    size_t len = 0;
    if (value < 0) len = 1;
    while (value != 0) { value /= 10; ++len; }
    return len;
}

inline size_t IntToStrHelper(int value, std::string& result, size_t pos = 0) {
    const size_t len = IntLen(value);
    if (result.size() < pos + len) result.resize(pos + len);

    if (value == 0) {
        result[pos] = '0';
        return pos + 1;
    }

    int sign = 1;
    if (value < 0) {
        result[pos] = '-';
        sign = -1;
    }

    size_t lsPos = pos + len - 1;
    while (value != 0) {
        const int nextValue = value / 10;
        const int digit = sign * (value % 10);
        result[lsPos] = static_cast<char>(digit + '0');
        value = nextValue;
        --lsPos;
    }
    return pos + len;
}

inline void Encode(int value, std::string& out) { (void)IntToStrHelper(value, out); }
inline void Encode(bool value, std::string& out) { out.assign(value ? "yes" : "no"); }

inline void Encode(const std::string& value, std::string& out) {
    out.clear();
    out.reserve(value.size() + 2);
    out.push_back('"');
    out.append(value);
    out.push_back('"');
}

inline void Encode(const std::vector<int>& value, std::string& out) {
    if (value.empty()) { out = "[]"; return; }

    size_t totalLen = 2; // [ ]
    for (int v : value) totalLen += IntLen(v) + 1; // comma
    --totalLen;

    out.resize(totalLen);
    out[0] = '[';

    size_t pos = 1;
    for (int v : value) {
        pos = IntToStrHelper(v, out, pos);
        out[pos++] = ',';
    }
    out[pos - 1] = ']';
}

//============================================================
// Parsing helpers
//============================================================
inline bool Parse(StringSegment segment, int& result) {
    static constexpr int posOverflowGuard = std::numeric_limits<int>::max() / 10;
    static constexpr int posLastDigitGuard = std::numeric_limits<int>::max() % 10;
    static constexpr int negOverflowGuard = std::numeric_limits<int>::min() / 10;
    static constexpr int negLastDigitGuard = std::numeric_limits<int>::min() % 10;

    result = 0;
    segment.Trim();
    if (segment.IsEmpty()) return false;

    int sign = (segment.FirstChar() == '-') ? -1 : 1;
    if (sign == -1) {
        segment.RemovePrefix(1);
        if (segment.IsEmpty()) return false;
    }

    while (!segment.IsEmpty()) {
        const char c = segment.ReadLeft();
        if (!tf_isdigit(c)) return false;

        const int digit = (c - '0') * sign;

        if ((result > posOverflowGuard) || (result < negOverflowGuard) ||
            ((result == posOverflowGuard) && (digit > posLastDigitGuard)) ||
            ((result == negOverflowGuard) && (digit < negLastDigitGuard))) {
            return false;
        }
        result = result * 10 + digit;
    }
    return true;
}

inline bool Parse(StringSegment segment, std::string& result) {
    segment.Trim();
    if (segment.Length() < 2) return false;
    if (segment.FirstChar() != '"' || segment.LastChar() != '"') return false;
    segment.RemovePrefix(1);
    segment.RemoveSuffix(1);
    segment.CopyTo(result);
    return true;
}

inline bool Parse(StringSegment segment, bool& result) {
    segment.Trim();
    if (segment.Match("true") || segment.Match("yes")) { result = true; return true; }
    if (segment.Match("false") || segment.Match("no")) { result = false; return true; }
    return false;
}

inline bool Parse(StringSegment segment, std::vector<int>& result) {
    result.clear();
    segment.Trim();
    if (segment.IsEmpty()) return false;

    const char first = segment.ReadLeft();
    const char last  = segment.ReadRight();
    if (first != '[' || last != ']') return false;

    segment.Trim();
    if (segment.IsEmpty()) return true; // "[]"

    result.reserve(segment.CountChars(',') + 1);

    StringSegment prefix(segment);
    segment.Split(',', prefix);

    while (!prefix.IsEmpty()) {
        int v = 0;
        if (!Parse(prefix, v)) return false;
        result.push_back(v);
        if (!segment.Split(',', prefix)) break;
    }
    return true;
}

//============================================================
// Lowercase helpers
//============================================================
inline void StringToLowerCase(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), 
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        }
    );
}

//============================================================
// Field Adapters
//============================================================
template<class T>
class BaseFieldAdapter {
public:
    virtual ~BaseFieldAdapter() = default;

    virtual bool FromString(T& var, StringSegment s) const = 0;
    virtual bool FromString(T& var, const std::string& str) const {
        return FromString(var, StringSegment(std::string_view(str)));
    }

    virtual void ToString(const T& var, std::string& out) const = 0;
    virtual bool EqualsDefaultValue(const T& var) const = 0;
    virtual void SetDefaultValue(T& var) const = 0;
};

template<class T, class C>
class FieldAdapter : public BaseFieldAdapter<T> {
public:
    explicit FieldAdapter(C T::*ptr)
        : field_pointer(ptr), defaultValue{} {} // value-init default

    FieldAdapter(C T::*ptr, C defaultValue_)
        : field_pointer(ptr), defaultValue(std::move(defaultValue_)) {}

    bool FromString(T& var, StringSegment s) const{
        return Parse(s, var.*field_pointer);
    }

    void ToString(const T& var, std::string& out) const {
        Encode(var.*field_pointer, out);
    }

    bool EqualsDefaultValue(const T& var) const {
        return (var.*field_pointer == defaultValue);
    }

    void SetDefaultValue(T& var) const {
        var.*field_pointer = defaultValue;
    }

private:
    C T::*field_pointer;
    C defaultValue;
};

//============================================================
// Tables
//============================================================
class ITable {
public:
    virtual ~ITable() = default;

    virtual bool NewRow(size_t& row) = 0;
    virtual void SetDefaultValues(size_t row) = 0;
    virtual bool IsFixedSize() const = 0;

    virtual const std::string& GetColumnName(size_t col) const = 0;
    virtual bool GetValue(size_t row, size_t col, std::string& value) const = 0;
    virtual bool SetValue(size_t row, size_t col, StringSegment value) = 0;

    virtual bool GetColumnByName(StringSegment key, size_t& col) const = 0;
    virtual bool EqualsDefaultValue(size_t row, size_t col) const = 0;

    virtual size_t ColumnCount() const = 0;
    virtual size_t RowCount() const = 0;

    virtual bool GetValue(size_t row, StringSegment key, std::string& value) const {
        size_t col = 0;
        if (!GetColumnByName(key, col)) return false;
        return GetValue(row, col, value);
    }

    virtual bool SetValue(size_t row, StringSegment key, StringSegment value) {
        size_t col = 0;
        if (!GetColumnByName(key, col)) return false;
        return SetValue(row, col, value);
    }
};

template<class T>
class AbstractTableAdapter : public ITable {
public:
    using DataType = T;

    bool AddNamedColumn(const char* name, std::shared_ptr<BaseFieldAdapter<T>> field) {
        std::string key = name ? name : "";
        StringToLowerCase(key);
        if (name2id.find(key) != name2id.end()) return false;

        columnSpecs.push_back(ColumnSpec{ name ? name : "", std::move(field) });
        name2id.emplace(std::move(key), columnSpecs.size() - 1);
        return true;
    }

    const std::string& GetColumnName(size_t col) const {
        assert(col < ColumnCount());
        return columnSpecs[col].name;
    }

    bool EqualsDefaultValue(size_t row, size_t col) const {
        assert(col < ColumnCount());
        assert(row < RowCount());
        const auto& rec = GetRecord(row);
        return columnSpecs[col].fieldAdapter->EqualsDefaultValue(rec);
    }

    void SetDefaultValues(size_t row) {
        assert(row < RowCount());
        auto& rec = GetRecord(row);
        for (const auto& spec : columnSpecs) spec.fieldAdapter->SetDefaultValue(rec);
    }

    bool GetValue(size_t row, size_t col, std::string& value) const {
        assert(col < ColumnCount());
        assert(row < RowCount());
        const auto& rec = GetRecord(row);
        columnSpecs[col].fieldAdapter->ToString(rec, value);
        return true;
    }

    bool SetValue(size_t row, size_t col, StringSegment value) {
        assert(col < ColumnCount());
        assert(row < RowCount());
        auto& rec = GetRecord(row);
        return columnSpecs[col].fieldAdapter->FromString(rec, value);
    }

    bool GetColumnByName(StringSegment key, size_t& col) const {
        std::string s;
        key.CopyTo(s);
        StringToLowerCase(s);

        auto it = name2id.find(s);
        if (it == name2id.end()) return false;
        col = it->second;
        return true;
    }

    size_t ColumnCount() const { return columnSpecs.size(); }

protected:
    virtual T& GetRecord(size_t i) = 0;
    virtual const T& GetRecord(size_t i) const = 0;

private:
    struct ColumnSpec {
        std::string name;
        std::shared_ptr<BaseFieldAdapter<T>> fieldAdapter;
    };

    std::unordered_map<std::string, size_t> name2id;
    std::vector<ColumnSpec> columnSpecs;
};

template<class T>
class RecordAdapter : public AbstractTableAdapter<T> {
public:
    using DataType = T;
    explicit RecordAdapter(T& data) : data(data) {}

    bool NewRow(size_t& row) { (void)row; return false; }
    bool IsFixedSize() const { return true; }
    size_t RowCount() const { return 1; }

protected:
    T& GetRecord(size_t i) { assert(i == 0); return data; }
    const T& GetRecord(size_t i) const { assert(i == 0); return data; }

private:
    T& data;
};

template<class T>
class TableAdapter : public AbstractTableAdapter<T> {
public:
    using DataType = T;
    explicit TableAdapter(std::vector<T>& data) : data(data) {}

    bool NewRow(size_t& row) {
        data.emplace_back();
        row = data.size() - 1;
        this->SetDefaultValues(row);
        return true;
    }

    bool IsFixedSize() const { return false; }
    size_t RowCount() const { return data.size(); }

protected:
    T& GetRecord(size_t i) { assert(i < data.size()); return data[i]; }
    const T& GetRecord(size_t i) const { assert(i < data.size()); return data[i]; }

private:
    std::vector<T>& data;
};

//============================================================
// Column helpers
//============================================================
template<class T, class C>
inline void AddColumn(TableAdapter<T>& table, const char* name, C T::*field_pointer) {
    static_assert(std::is_class<C>::value, "This field must be a class.");
    table.AddNamedColumn(name, std::make_shared<FieldAdapter<T, C>>(field_pointer));
}

template<class T, class C>
inline void AddColumn(TableAdapter<T>& table, const char* name, C T::*field_pointer, C defaultValue) {
    table.AddNamedColumn(name, std::make_shared<FieldAdapter<T, C>>(field_pointer, std::move(defaultValue)));
}

template<class T>
inline void AddColumn(TableAdapter<T>& table, const char* name, int T::*field_pointer, int defaultValue = -1) {
    table.AddNamedColumn(name, std::make_shared<FieldAdapter<T, int>>(field_pointer, defaultValue));
}

template<class T, class C>
inline void AddColumn(RecordAdapter<T>& rec, const char* name, C T::*field_pointer) {
    static_assert(std::is_class<C>::value, "This field must be a class.");
    rec.AddNamedColumn(name, std::make_shared<FieldAdapter<T, C>>(field_pointer));
}

template<class T, class C>
inline void AddColumn(RecordAdapter<T>& rec, const char* name, C T::*field_pointer, C defaultValue) {
    rec.AddNamedColumn(name, std::make_shared<FieldAdapter<T, C>>(field_pointer, std::move(defaultValue)));
}

template<class T>
inline void AddColumn(RecordAdapter<T>& rec, const char* name, int T::*field_pointer, int defaultValue = -1) {
    rec.AddNamedColumn(name, std::make_shared<FieldAdapter<T, int>>(field_pointer, defaultValue));
}

//============================================================
// YAML parsing framework
//============================================================
class AbstractLineParser {
public:
    AbstractLineParser() : lineNumber(0), throwOnError(false), isOK(true) {}
    virtual ~AbstractLineParser() = default;

    void ParseFile(const char* filename, bool shouldThrowOnError = false) {
        throwOnError = shouldThrowOnError;
        isOK = true;
        lineNumber = 0;
        currentFile = filename ? filename : "";

        std::ifstream input(filename);
        CheckCondition(input.good(), "Cannot open input file.");
        if (!IsOK()) return;

        PreParse();
        if (!IsOK()) return;

        std::string line;
        while (std::getline(input, line)) {
            ++lineNumber;
            StringSegment seg{std::string_view(line)};
            try {
                ParseLine(seg);
            } catch (const TestFrameworkError& e) {
                if (throwOnError) throw ParseError(e.what(), currentFile, lineNumber);
                isOK = false;
                return;
            }

            if (!IsOK()) return;
        }

        PostParse();
    }

    bool IsOK() const noexcept { return isOK; }

protected:
    void CheckCondition(bool condition, const char* error) {
        if (condition) return;
        isOK = false;
        if (throwOnError) throw ParseError(error ? error : "Parse error", currentFile, lineNumber);
    }

    size_t CurrentLine() const noexcept { return lineNumber; }
    const std::string& CurrentFile() const noexcept { return currentFile; }

private:
    virtual void PreParse() {}
    virtual void PostParse() {}
    virtual void ParseLine(StringSegment s) = 0;

    size_t lineNumber;
    bool throwOnError;
    bool isOK;
    std::string currentFile;
};

//============================================================
// BasicYamlParser
//============================================================
class BasicYamlParser : public AbstractLineParser {
public:
    BasicYamlParser() : AbstractLineParser(), pHeader(nullptr), pTable(nullptr), isHeaderSection(true) {}

    BasicYamlParser(ITable* header, ITable* table)
        : AbstractLineParser(), pHeader(header), pTable(table), isHeaderSection(true) {
        assert(pHeader != nullptr);
        assert(pHeader->RowCount() > 0);
        assert(pTable != nullptr);
        assert(!pTable->IsFixedSize());
    }

    void SetHeaderAdapter(ITable* header) {
        assert(header != nullptr);
        assert(header->RowCount() > 0);
        pHeader = header;
    }

    void SetTableAdapter(ITable* table) {
        assert(table != nullptr);
        assert(!table->IsFixedSize());
        pTable = table;
    }

    void PreParse() override { isHeaderSection = true; }

    void ParseLine(StringSegment s) override {
        CheckCondition(pTable != nullptr, "Table adapter is not set.");
        if (!IsOK()) return;

        s.Trim();
        if (s.IsEmpty()) return;

        const char first = s.FirstChar();
        if (first == '#') return;

        if (s.Match("data:")) {
            isHeaderSection = false;
            return;
        }

        if (first == '-') {
            CheckCondition(!isHeaderSection, "Invalid entry in the header section.");
            if (!IsOK()) return;

            size_t row = 0;
            const bool ok = pTable->NewRow(row);
            CheckCondition(ok, "Cannot create a new table row.");
            if (!IsOK()) return;

            s.RemovePrefix(1);
            s.Trim();
            if (s.IsEmpty()) return;
        }

        StringSegment key(s);
        s.Split(':', key);
        s.Trim();
        key.Trim();

        CheckCondition(!s.IsEmpty() && !key.IsEmpty(), "Key or value is empty.");
        if (!IsOK()) return;

        if (isHeaderSection) {
            CheckCondition(pHeader != nullptr, "Unexpected header.");
            if (!IsOK()) return;

            const bool ok = pHeader->SetValue(0, key, s);
            if (!ok) {
                std::string k, v;
                key.CopyTo(k);
                s.CopyTo(v);
                ThrowError(std::string("Cannot parse header key '") + k + "' with value '" + v + "'");
            }
        } else {
            const size_t n = pTable->RowCount();
            CheckCondition(n > 0, "Data section has no active row.");
            if (!IsOK()) return;

            const bool ok = pTable->SetValue(n - 1, key, s);
            if (!ok) {
                std::string k, v;
                key.CopyTo(k);
                s.CopyTo(v);
                ThrowError(std::string("Cannot parse data key '") + k + "' with value '" + v + "'");
            }
        }
    }

private:
    ITable* pHeader;
    ITable* pTable;
    bool isHeaderSection;
};

//============================================================
// Domain structs
//============================================================
struct ProblemSetHeader {
    int id = -1;
    int problem_count = 0;
    int test_mistakes = -1;
    int time = -1;
    Clock::time_point tStart;
};

struct BasicProblem {
    int id{};
    int correct_answer{};
    int student_answer{};
    std::string helpMsg;
};

//============================================================
// Default columns helpers
//============================================================
inline void AddDefaultProblemSetColumns(RecordAdapter<ProblemSetHeader>& psAdapter) {
    AddColumn(psAdapter, "problem_set_number", &ProblemSetHeader::id, -1);
    AddColumn(psAdapter, "problems", &ProblemSetHeader::problem_count, -1);
    AddColumn(psAdapter, "time", &ProblemSetHeader::time, -1);
    AddColumn(psAdapter, "test_mistakes", &ProblemSetHeader::test_mistakes, -1);
}

template<class T>
inline void AddDefaultProblemColumns(TableAdapter<T>& prAdapter) {
    AddColumn<T, int>(prAdapter, "problem", &T::id, -1);
    AddColumn<T, int>(prAdapter, "correct_answer", &T::correct_answer, -1);
    AddColumn<T, std::string>(prAdapter, "msg", &T::helpMsg);
}

template<class T>
inline void AddDefaultProblemColumnsForOutput(TableAdapter<T>& prAdapter) {
    AddColumn<T, int>(prAdapter, "problem", &T::id, -1);
    AddColumn<T, int>(prAdapter, "student_answer", &T::student_answer, -1);
}

//============================================================
// YAML writing helpers
//============================================================
inline void WriteRecordToStream(std::ostream& out, ITable* table, size_t row,
                                bool writeDefaultValues, bool indent) {
    if (!table) throw TestFrameworkError("WriteRecordToStream: table is null.");

    if (indent) out << "\n";

    const size_t nCols = table->ColumnCount();
    for (size_t j = 0; j < nCols; ++j) {
        if (indent && j == 0) out << " - ";

        if (writeDefaultValues || !table->EqualsDefaultValue(row, j)) {
            if (indent && j != 0) out << "   ";
            out << table->GetColumnName(j) << ": ";

            std::string value;
            if (!table->GetValue(row, j, value))
                throw TestFrameworkError("WriteRecordToStream: cannot get value.");

            out << value << "\n";
        }
    }
}

inline void WriteTableToStream(std::ostream& out,
                               ITable* header, // can be nullptr
                               ITable* table,
                               bool writeDefaultValues) {
    if (!table) throw TestFrameworkError("WriteTableToStream: table is null.");

    if (header != nullptr) {
        WriteRecordToStream(out, header, 0, writeDefaultValues, false);
    }

    out << "\n"
        << "data:\n";

    const size_t nRows = table->RowCount();
    for (size_t i = 0; i < nRows; ++i) {
        WriteRecordToStream(out, table, i, writeDefaultValues, true);
    }
}

inline void WriteTableToFile(const char* filename, ITable* header, ITable* table,
                             bool writeDefaultValues, const char* comments) {
    std::ofstream out(filename);
    ThrowIfConditionFails(out.good(), "Cannot open output file!");

    if (comments) out << comments;
    WriteTableToStream(out, header, table, writeDefaultValues);
}

//============================================================
// ProblemSet helpers (exception-based)
//============================================================
template<class T>
inline void PreprocessProblemSet(int problem_set_id, const std::vector<T>& problems,
                                 ProblemSetHeader& header) {
    ThrowIfConditionFails(header.id == problem_set_id,
                          "Wrong problem set. Check problem set number.");
    ThrowIfConditionFails(header.problem_count == static_cast<int>(problems.size()),
                          "Input file is corrupted.");

    for (int i = 0; i < header.problem_count; ++i) {
        ThrowIfConditionFails(problems[static_cast<size_t>(i)].id == (i + 1),
                              "Input file is corrupted.");
    }

    header.tStart = Clock::now();
}

template<class T>
inline void ProcessResults(const std::vector<T>& problems, ProblemSetHeader& header) {
    using namespace std::chrono;

    const auto tNow = Clock::now();
    const auto time_span = duration_cast<duration<double>>(tNow - header.tStart);
    header.time = static_cast<int>(std::round(1000.0 * time_span.count()));

    int mistakes = 0;
    for (const auto& p : problems) {
        if (p.student_answer != p.correct_answer) {
            ++mistakes;
            std::cout << "\n";
            std::cout << "Mistake in problem #" << p.id << ".\n";
            std::cout << "Correct answer: " << p.correct_answer << ".\n";
            if (!p.helpMsg.empty()) std::cout << p.helpMsg << "\n";
            std::cout << "Your answer: " << p.student_answer << ".\n";
            std::cout << "=========================";
        }
    }

    header.test_mistakes = mistakes;

    if (mistakes > 0) {
        std::cout << "\nYour algorithm made " << mistakes << " mistake(s).\n";
    } else {
        std::cout << "Your algorithm solved all test problems correctly. Congratulations!\n";
    }
}

} // namespace TestFramework

#endif // TEST_FRAMEWORK_H
