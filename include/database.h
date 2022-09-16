#ifndef __DATABASE_H__
#define __DATABASE_H__
#include <vector>
#include <iostream>
#include <utility>
#include <string>
#include <sstream>
#include <functional>
#include <fstream>
#include <regex>
#include <filesystem>
#include <optional>
#include <omp.h>
#include <algorithm>
#include <iterator>
#include <type_traits>


inline size_t match_quote(const std::string &line, size_t quote_index)
{
    if (quote_index > line.size() - 1)
        return std::string::npos;
    if (line[quote_index] != '"')
        return std::string::npos;

    quote_index++;
    char c;
    while (true)
    {
        c = line[quote_index];
        if (c == 0)
        {
            return std::string::npos;
        }
        if (c == '\\')
        {
            quote_index += 2;
            continue;
        }
        if (c == '"')
        {
            break;
        }
        quote_index++;
    }

    return quote_index;
}

inline size_t match_bracket(const std::string &line, size_t bracket_index)
{
    if (bracket_index > line.size() - 1)
        return std::string::npos;
    char open = line[bracket_index];
    char close;
    switch (open)
    {
    case '[':
        close = ']';
        break;
    case '{':
        close = '}';
        break;
    case '(':
        close = ')';
        break;
    default:
        return std::string::npos;
    }

    bracket_index++;
    char c;
    size_t depth = 1;
    while (true)
    {
        c = line[bracket_index];
        if (c == 0)
        {
            return std::string::npos;
        }
        if (c == open)
        {
            depth++;
        }
        if (c == close)
        {
            depth--;
        }
        if (c == '"')
        {
            bracket_index = match_quote(line, bracket_index);
            if (bracket_index == std::string::npos) return line.size() -1;
        }
        if (depth == 0)
        {
            break;
        }
        bracket_index++;
    }

    return bracket_index;
}

inline std::string de_whitespace_json(const std::string &line)
{
    std::string ret;
    ret.reserve(line.size());

    size_t back;
    for (size_t i = 0; i < line.size(); i++)
    {
        if (line[i] == '"')
        {
            back = match_quote(line, i);
            while (i < back)
                ret.push_back(line[i++]);
        }
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n')
            ret.push_back(line[i]);
    }

    return ret;
}

// this function assumes well-formatted json, which will be checked at input by a later defined function
inline std::vector<std::string> tokenize_json(const std::string &json)
{
    std::vector<std::string> ret;
    if (json.size() == 0 || json == "{}")
    {
        return ret;
    }

    size_t front = 0;
    size_t back = 0;
    while (back != json.size() - 1)
    {
        front = back + 1;
        back = match_quote(json, front);
        front++;
        ret.push_back(json.substr(front, back - front));
        front = back + 2;

        switch (json[front])
        {
        case '"':
            back = match_quote(json, front) + 1;
            break;
        case '[':
        case '{':
            back = match_bracket(json, front) + 1;
            break;
        default:
            back = front + 1;
            while (json[back] != ',' && json[back] != '}')
                back++;
        }
        ret.push_back(json.substr(front, back - front));
    }
    return ret;
}

inline std::string smash_json(const std::vector<std::string> &tokens)
{
    if (tokens.size() == 0)
        return "{}"; // empty json
    if ((tokens.size() & 0x01) == 0x01)
        return ""; // unbalanced fields

    std::stringstream ss;

    ss << '{';
    for (size_t i = 0; i < tokens.size(); i += 2)
    {
        ss << '"' << tokens[i] << "\":" << tokens[i + 1] << ',';
    }

    std::string ret = ss.str();
    ret[ret.size() - 1] = '}';
    return ret;
}

inline std::vector<std::string> tokenize_array(const std::string &array)
{
    std::vector<std::string> ret;
    if (array.size() == 0 || array == "[]")
    {
        return ret;
    }

    size_t front = 1;
    size_t back = 0;
    while (back != array.size() - 1)
    {
        front = back + 1;
        switch (array[front])
        {
        case '"':
            back = match_quote(array, front) + 1;
            break;
        case '[':
        case '{':
            back = match_bracket(array, front) + 1;
            break;
        default:
            back = front + 1;
            while (array[back] != ',' && array[back] != ']')
                back++;
        }
        ret.push_back(array.substr(front, back - front));
    }

    return ret;
}

inline std::string smash_array(const std::vector<std::string> &tokens)
{
    if (tokens.size() == 0)
        return "[]"; // empty json

    std::stringstream ss;

    ss << '[';
    for (size_t i = 0; i < tokens.size(); i++)
    {
        ss << tokens[i] << ',';
    }

    std::string ret = ss.str();
    ret[ret.size() - 1] = ']';
    return ret;
}

// must be passed de_whitespaced json, for efficiency of usage
// non-verbose version, does not diagnose error in json, can be changed later if desired
inline std::optional<std::string> verify_json(const std::string& json)
{
    std::string ret;
    std::function<std::optional<std::string>(const std::string&)> verify_data = [](const std::string& data) -> std::optional<std::string>
    {
        // string
        if (data[0] == '"')
        {
            if (match_quote(data, 0) != data.size() - 1) return std::string("missing final quote");
            return std::nullopt;
        }

        // number
        if ((data[0] >= '0' && data[0] <= '9') || (data[0] == '-'))
        {
            bool has_decimal = false;
            bool has_exponent = false;
            size_t i;
            for (i = 1; i < data.size(); i++)
            {
                if (data[i] == '.')
                {
                    has_decimal = true;
                    i++;
                    break;
                }
                if (data[i] == 'e' || data[i] == 'E')
                {
                    has_exponent = true;
                    i++;
                    break;
                }
                if (data[i] < '0' || data[i] > '9')
                {
                    return std::string("non-numeric character in number");
                }
            }

            if (has_decimal)
            {
                for (; i < data.size(); i++)
                {
                    if (data[i] == 'e' || data[i] == 'E')
                    {
                        has_exponent = true;
                        i++;
                        break;
                    }
                    if (data[i] < '0' || data[i] > '9')
                    {
                        return std::string("non-numeric character in number");
                    }
                }
            }
            if (has_exponent)
            {
                if (data[i] != '+' && data[i] != '-' && !(data[i] >= '0' && data[i] <= '9'))
                    return std::string("non-numeric character in number");
                i++;
                for (; i < data.size(); i++)
                {
                    if (data[i] < '0' || data[i] > '9')
                    {
                        return std::string("non-numeric character in number");
                    }
                }
            }
            return std::nullopt;
        }

        // bool or null
        if (data != "true" && data != "false" && data != "null" && data != "delete")
        {
            for (char c : data) { if (c == '"') return std::string("missing initial quote"); }
            return std::string("cannot verify type");
        }
        return std::nullopt;
    };

    std::function<std::optional<std::string>(const std::string&)> verify_array;
    std::function<std::optional<std::string>(const std::string&)> verify_object = [&](const std::string& data) -> std::optional<std::string>
    {
        if (data == "{}") return std::nullopt;
        if (data.front() != '{') return std::string("missing opening brace");
        if (match_bracket(data, 0) != data.size() - 1) return std::string("missing closing brace");

        std::vector<std::string> fields;
        size_t front = 1;
        size_t back = front;
        size_t temp;
        while (back < data.size()) // split into pairs
        {
            switch (data[back])
            {
                case '"':
                    temp = match_quote(data, back);
                    break;
                case '[':
                case '{':
                    temp = match_bracket(data, back);
                    break;
                case ',':
                case '}':
                    fields.push_back(data.substr(front, back - front));
                    front = back + 1;
                    temp = back;
                    break;
                default:
                    temp = back;
            }
            if (temp == std::string::npos) // bad match. Break and find issue in field parsing
            {
                fields.push_back(data.substr(front));
                break;
            }
            if (front == data.size() - 1) return std::string("trailing comma in object");
            back = temp + 1;
        }

        std::optional<std::string> res;
        for (const auto& f : fields)
        {
            if (f.size() == 0) return std::string("Missing key value pair");
            if (f[0] != '"') return std::string("Error in object element: " + f + " key missing inital quote");

            back = match_quote(f, 0);
            if (back == std::string::npos || match_quote(f, back) == f.size() - 1)
                return std::string("Error in object element: " + f + " key missing final quote");

            back++;
            if (f[back] != ':') return std::string("Error in object element: " + f + " missing ':' delimiter");

            back++;
            switch (f[back])
            {
                case '[': res = verify_array(f.substr(back)); break;
                case '{': res = verify_object(f.substr(back)); break;
                default: res = verify_data(f.substr(back)); break;
            }

            if (res) // failure
            {
                return std::string("Error in object value: " + f + " " + *res);
            }
        }
        return std::nullopt;
    };


    verify_array = [&](const std::string& data) -> std::optional<std::string>
    {
        if (data == "[]") return std::nullopt;
        if (data.front() != '[') return std::string("missing opening bracket");
        if (match_bracket(data, 0) != data.size() - 1) return std::string("missing closing bracket");

        std::vector<std::string> fields;
        size_t front = 1;
        size_t back = front;
        size_t temp;
    while (back < data.size())
    {
        switch (data[back])
        {
            case '"':
                temp = match_quote(data, back);
                break;
            case '[':
            case '{':
                temp = match_bracket(data, back);
                break;
            case ',':
            case ']':
                fields.push_back(data.substr(front, back - front));
                front = back + 1;
                temp = back;
                break;
            default:
                temp = back;
        }
        if (back == std::string::npos) // bad match. Break and find issue in field parsing
        {
            fields.push_back(data.substr(front));
            break;
        }
        if (front == data.size() - 1) return std::string("trailing comma in array");
        back = temp + 1;
    }

    std::optional<std::string> res;
    for (const auto& f : fields)
    {
        switch (f[0])
        {
            case '[': res = verify_array(f); break;
            case '{': res = verify_object(f); break;
            default: res = verify_data(f); break;
        }

        if (res) // failure
        {
            return std::string("Error in array field: " + f + " " + *res);
        }
    }
    return std::nullopt;
};

if (json.empty()) return std::string("json is an empty string");
std::optional<std::string> status = verify_object(json);
if (status) return std::string("json verification failed: " + *status);
return std::nullopt;
}

inline bool json_is_null(const std::string &data, const std::string &field)
{
    auto fields = tokenize_json(data);
    for (size_t i = 0; i < fields.size(); i += 2)
    {
        if (fields[i] == field)
        {
            if (fields[i + 1] == "null")
            {
                return true;
            }

            return false;
        }
    }
    throw std::runtime_error("Field does not exist");
}

class json_array
{
public:
json_array()
{
    data = "";
}

json_array(const std::string &data)
{
    this->data = data;
    }

    template <typename T>
    T get(size_t field) const
    {
        throw std::runtime_error("Type provided is not a legal type for json data");
    }

    bool is_null(size_t field)
    {
        auto fields = tokenize_array(data);
        if (field >= fields.size())
        {
            throw std::runtime_error("Index out of bounds");
        }

        if (fields[field] == "null")
        {
            return true;
        }
        return false;
    }

private:
    std::string data;
    friend class Document;
    std::string get_as_string(size_t field)
    {
        auto fields = tokenize_array(data);
        if (field >= fields.size())
            throw std::runtime_error("Index out of bounds");

        return fields[field];
    }
};

class json_object
{
public:
    json_object()
    {
        data = "";
    }

    json_object(const std::string &data)
    {
        this->data = data;
    }

    template <typename T>
    T get(const std::string &field) const
    {
        throw std::runtime_error("Type provided is not a legal type for json data");
    }

    bool is_null(const std::string &field)
    {
        return json_is_null(data, field);
    }

private:
    std::string data;
    friend class Document;
    std::string get_as_string(const std::string& field)
    {
        auto fields = tokenize_json(data);
        for (size_t i = 0; i < fields.size(); i += 2)
        {
            if (fields[i] == field)
            {
                return fields[i + 1];
            }
        }
        throw std::runtime_error("Field does not exist");
    }
};

template <>
inline std::string json_array::get<std::string>(size_t field) const
{
    auto fields = tokenize_array(data);
    if (field >= fields.size())
    {
        throw std::runtime_error("Index out of bounds");
    }

    if (fields[field][0] != '"')
    {
        throw std::runtime_error("Field is not a string");
    }

    return fields[field];
}

template <>
inline int json_array::get<int>(size_t field) const
{
    auto fields = tokenize_array(data);
    if (field >= fields.size())
    {
        throw std::runtime_error("Index out of bounds");
    }

    if (fields[field][0] == '"' || fields[field][0] == '[' || fields[field][0] == '{' || fields[field] == "null" || fields[field] == "true" || fields[field] == "false")
    {
        throw std::runtime_error("Field is not a number");
    }

    return std::stod(fields[field]);
}

template <>
inline double json_array::get<double>(size_t field) const
{
    auto fields = tokenize_array(data);
    if (field >= fields.size())
    {
        throw std::runtime_error("Index out of bounds");
    }

    if (fields[field][0] == '"' || fields[field][0] == '[' || fields[field][0] == '{' || fields[field] == "null" || fields[field] == "true" || fields[field] == "false")
    {
        throw std::runtime_error("Field is not a number");
    }

    return std::stod(fields[field]);
}

template <>
inline bool json_array::get<bool>(size_t field) const
{
    auto fields = tokenize_array(data);
    if (field >= fields.size())
    {
        throw std::runtime_error("Index out of bounds");
    }

    if (fields[field] != "true" && fields[field] != "false")
    {
        throw std::runtime_error("Field is not a bool");
    }

    return (fields[field] == "true");
}

template <>
inline json_object json_array::get<json_object>(size_t field) const
{
    auto fields = tokenize_array(data);
    if (field >= fields.size())
    {
        throw std::runtime_error("Index out of bounds");
    }

    if (fields[field][0] != '{')
    {
        throw std::runtime_error("Field is not an object");
    }

    return json_object(fields[field]);
}

template <>
inline json_array json_array::get<json_array>(size_t field) const
{
    auto fields = tokenize_array(data);
    if (field >= fields.size())
    {
        throw std::runtime_error("Index out of bounds");
    }

    if (fields[field][0] != '[')
    {
        throw std::runtime_error("Field is not an array");
    }

    return json_array(fields[field]);
}

inline json_object json_extract_object(const std::string &data, const std::string &field)
{
    auto fields = tokenize_json(data);
    for (size_t i = 0; i < fields.size(); i += 2)
    {
        if (fields[i] == field)
        {
            if (fields[i + 1][0] != '{')
            {
                throw std::runtime_error("Field is not an object");
            }

            return json_object(fields[i + 1]);
        }
    }
    throw std::runtime_error("Field does not exist");
}

inline json_array json_extract_array(const std::string &data, const std::string &field)
{
    auto fields = tokenize_json(data);
    for (size_t i = 0; i < fields.size(); i += 2)
    {
        if (fields[i] == field)
        {
            if (fields[i + 1][0] != '[')
            {
                throw std::runtime_error("Field is not an array");
            }

            return json_array(fields[i + 1]);
        }
    }
    throw std::runtime_error("Field does not exist");
}

inline std::string json_extract_string(const std::string &data, const std::string &field)
{
    auto fields = tokenize_json(data);
    for (size_t i = 0; i < fields.size(); i += 2)
    {
        if (fields[i] == field)
        {
            if (fields[i + 1][0] != '"')
            {
                throw std::runtime_error("Field is not a string");
            }

            return fields[i + 1];
        }
    }
    throw std::runtime_error("Field does not exist");
}

inline int json_extract_int(const std::string &data, const std::string &field)
{
    auto fields = tokenize_json(data);
    for (size_t i = 0; i < fields.size(); i += 2)
    {
        if (fields[i] == field)
        {
            if (fields[i + 1][0] == '"' || fields[i + 1][0] == '[' || fields[i + 1][0] == '{' || fields[i + 1] == "null" || fields[i + 1] == "true" || fields[i + 1] == "false")
            {
                throw std::runtime_error("Field is not a number");
            }

            return std::stod(fields[i + 1]);
        }
    }
    throw std::runtime_error("Field does not exist");
}

inline double json_extract_double(const std::string &data, const std::string &field)
{
    auto fields = tokenize_json(data);
    for (size_t i = 0; i < fields.size(); i += 2)
    {
        if (fields[i] == field)
        {
            if (fields[i + 1][0] == '"' || fields[i + 1][0] == '[' || fields[i + 1][0] == '{' || fields[i + 1] == "null" || fields[i + 1] == "true" || fields[i + 1] == "false")
            {
                throw std::runtime_error("Field is not a number");
            }

            return std::stod(fields[i + 1]);
        }
    }
    throw std::runtime_error("Field does not exist");
}

inline bool json_extract_bool(const std::string &data, const std::string &field)
{
    auto fields = tokenize_json(data);
    for (size_t i = 0; i < fields.size(); i += 2)
    {
        if (fields[i] == field)
        {
            if (fields[i + 1] != "true" && fields[i + 1] != "false")
            {
                throw std::runtime_error("Field is not a bool");
            }

            return (fields[i + 1] == "true");
        }
    }
    throw std::runtime_error("Field does not exist");
}

template <>
inline std::string json_object::get<std::string>(const std::string &field) const
{
    return json_extract_string(data, field);
}

template <>
inline int json_object::get<int>(const std::string &field) const
{
    return json_extract_int(data, field);
}

template <>
inline double json_object::get<double>(const std::string &field) const
{
    return json_extract_double(data, field);
}

template <>
inline bool json_object::get<bool>(const std::string &field) const
{
    return json_extract_bool(data, field);
}

template <>
inline json_object json_object::get<json_object>(const std::string &field) const
{
    return json_extract_object(data, field);
}

template <>
inline json_array json_object::get<json_array>(const std::string &field) const
{
    return json_extract_array(data, field);
}

class Document
{
public:
    Document()
    {
        id = next_id++;
    }
    Document(const std::string &json)
    {
        data = de_whitespace_json(json);
        auto failure = verify_json(data);
        if (failure) throw std::runtime_error(*failure);
        id = next_id++;
    }
    Document(size_t id, const std::string &json)
    {
        data = de_whitespace_json(json);
        auto failure = verify_json(data);
        if (failure) throw std::runtime_error(*failure);
        this->id = id;
    }

    template <typename T>
    T get(const std::string &field) const
    {
        throw std::runtime_error("Type provided is not a legal type for json data");
    }

    template <typename T>
    T query(const std::string &path) const
    {
        throw std::runtime_error("Type provided is not a legal type for json data");
    }

    bool is_null(const std::string &field)
    {
        return json_is_null(data, field);
    }

    size_t get_id() const
    {
        return id;
    }

    std::string to_string() const
    {
        std::stringstream ss;

        auto print_tabs = [&ss](size_t n)
        {
            for (size_t i = 0; i < n; i++)
            {
                ss << '\t';
            }
        };

        std::function<void(const std::string &, size_t)> stream_formatted_object;

        std::function<void(const std::string &, size_t)> stream_formatted_array = [&](const std::string &data, size_t depth)
        {
            ss << '[';
            auto tokens = tokenize_array(data);
            if (tokens.size() == 0)
            {
                ss << ']';
                return;
            }
            for (const auto &t : tokens)
            {
                ss << '\n';
                print_tabs(depth + 1);
                switch (t[0])
                {
                case '[':
                    stream_formatted_array(t, depth + 1);
                    break;
                case '{':
                    stream_formatted_object(t, depth + 1);
                    break;
                default:
                    ss << t;
                }
                ss << ',';
            }
            ss.seekp(-1, ss.cur);
            ss << '\n';
            print_tabs(depth);
            ss << ']';
        };

        stream_formatted_object = [&](const std::string &data, size_t depth)
        {
            ss << '{';
            auto tokens = tokenize_json(data);
            if (tokens.size() == 0)
            {
                ss << '}';
                return;
            }
            for (size_t i = 0; i < tokens.size(); i += 2)
            {
                ss << '\n';
                print_tabs(depth + 1);
                ss << tokens[i] << ": ";
                switch (tokens[i + 1][0])
                {
                case '[':
                    stream_formatted_array(tokens[i + 1], depth + 1);
                    break;
                case '{':
                    stream_formatted_object(tokens[i + 1], depth + 1);
                    break;
                default:
                    ss << tokens[i + 1];
                }
                ss << ',';
            }
            ss.seekp(-1, ss.cur);
            ss << '\n';
            print_tabs(depth);
            ss << '}';
        };

        ss << "Document " << id << ": ";
        stream_formatted_object(data, 0);
        return ss.str();
    }

private:
    size_t id; // index in Collection, but when in a smaller subset will need access
    static size_t next_id;
    std::string data; // as json

    std::string query_as_string(const std::string &path) const;

    friend class Collection;
    friend class uCollection;
};

inline std::pair<std::vector<std::string>, std::vector<std::string>> tokenize_pattern(std::string pattern)
{
    // "field1"."field2"[2].data1

    pattern = de_whitespace_json(pattern);

    if (pattern.size() == 0)
    {
        throw std::runtime_error("no pattern");
    }

    //"field 1" = true & "field 2" = 3
    size_t front = 0;
    size_t back = 0;

    // parse pattern
    std::vector<std::string> keys;
    std::vector<std::string> vals;

    // split at '&'
    while (back != pattern.size())
    {
        switch (pattern[back])
        {
        case '"':
            back = match_quote(pattern, back);
            break;
        case '{':
        case '[':
            back = match_bracket(pattern, back);
            break;
        case '&':
            keys.push_back(pattern.substr(front, back - front));
            front = back + 1;
            break;
        default:
            break;
        }
        back++;
    }
    keys.push_back(pattern.substr(front));
    for (size_t i = 0; i < keys.size(); i++)
    {
        back = 0;
        while (back < keys.at(i).size())
        {
            if (keys.at(i)[back] == '"')
            {
                back = match_quote(keys.at(i), back);
            }
            if (keys.at(i)[back] == '=')
            {
                break;
            }
            back++;
        }
        if (back == keys.size())
        {
            throw std::runtime_error("syntax issue: missing equal sign");
        }
        if (back == 0)
        {
            throw std::runtime_error("syntax issue: no key");
        }
        vals.push_back(keys.at(i).substr(back + 1));
        keys.at(i) = keys.at(i).substr(0, back);
    }
    return std::make_pair(keys, vals);
}

inline std::pair<std::string, std::string> get_first_field(std::string query)
{
    std::regex regex{R"(((".+?")|(\[[0-9]+?\])))"};

    std::smatch match;

    std::regex_search(query, match, regex);
    if (match.suffix().str() != "")
    {
        if (match.suffix().str()[0] == '.')
        {
            return {match.str(1), match.suffix().str().substr(1)};
        }
    }

    return {match.str(1), match.suffix().str()};
}

template <>
inline std::string Document::get<std::string>(const std::string &field) const
{
    return json_extract_string(data, field);
}

template <>
inline int Document::get<int>(const std::string &field) const
{
    return json_extract_int(data, field);
}

template <>
inline double Document::get<double>(const std::string &field) const
{
    return json_extract_double(data, field);
}

template <>
inline bool Document::get<bool>(const std::string &field) const
{
    return json_extract_bool(data, field);
}

template <>
inline json_object Document::get<json_object>(const std::string &field) const
{
    return json_extract_object(data, field);
}

template <>
inline json_array Document::get<json_array>(const std::string &field) const
{
    return json_extract_array(data, field);
}

template <>
inline std::string Document::query<std::string>(const std::string &field) const
{
    json_object o_temp;
    json_array a_temp;

    // get key and remain
    auto [key, remain] = get_first_field(field);
    key = key.substr(1, key.size() - 2);
    int i;
    if (remain != "")
    {
        if (remain[0] == '"')
        {
            o_temp = this->get<json_object>(key);
            i = 1;
        }
        if (remain[0] == '[')
        {
            a_temp = this->get<json_array>(key);
            i = 2;
        }
    }

    std::tie(key, remain) = get_first_field(remain);
    key = key.substr(1, key.size() - 2);
    while (remain != "")
    {
        if (i == 1)
        {
            if (remain[0] == '"')
            {
                o_temp = o_temp.get<json_object>(key);
            }

            if (remain[0] == '[')
            {
                a_temp = o_temp.get<json_array>(key);
                i = 2;
            }
        }
        else if (i == 2)
        {

            if (remain[0] == '"')
            {
                o_temp = a_temp.get<json_object>(std::stoi(key));
                i = 1;
            }

            if (remain[0] == '[')
            {
                a_temp = a_temp.get<json_array>(std::stoi(key));
            }
        }
        std::tie(key, remain) = get_first_field(remain);
        key = key.substr(1, key.size() - 2);
    }

    if (i == 1)
    {
        return o_temp.get<std::string>(key);
    }
    return a_temp.get<std::string>(std::stoi(key));
}

template <>
inline int Document::query<int>(const std::string &path) const
{
    json_object o_temp;
    json_array a_temp;

    // get key and remain
    auto [key, remain] = get_first_field(path);
    key = key.substr(1, key.size() - 2);
    int i;
    if (remain != "")
    {
        if (remain[0] == '"')
        {
            o_temp = this->get<json_object>(key);
            i = 1;
        }
        if (remain[0] == '[')
        {
            a_temp = this->get<json_array>(key);
            i = 2;
        }
    }

    std::tie(key, remain) = get_first_field(remain);
    key = key.substr(1, key.size() - 2);
    while (remain != "")
    {
        if (i == 1)
        {
            if (remain[0] == '"')
            {
                o_temp = o_temp.get<json_object>(key);
            }

            if (remain[0] == '[')
            {
                a_temp = o_temp.get<json_array>(key);
                i = 2;
            }
        }
        else if (i == 2)
        {

            if (remain[0] == '"')
            {
                o_temp = a_temp.get<json_object>(std::stoi(key));
                i = 1;
            }

            if (remain[0] == '[')
            {
                a_temp = a_temp.get<json_array>(std::stoi(key));
            }
        }
        std::tie(key, remain) = get_first_field(remain);
        key = key.substr(1, key.size() - 2);
    }

    if (i == 1)
    {
        return o_temp.get<int>(key);
    }
    return a_temp.get<int>(std::stoi(key));
}

template <>
inline double Document::query<double>(const std::string &field) const
{
    json_object o_temp;
    json_array a_temp;

    // get key and remain
    auto [key, remain] = get_first_field(field);
    key = key.substr(1, key.size() - 2);
    int i;
    if (remain != "")
    {
        if (remain[0] == '"')
        {
            o_temp = this->get<json_object>(key);
            i = 1;
        }
        if (remain[0] == '[')
        {
            a_temp = this->get<json_array>(key);
            i = 2;
        }
    }

    std::tie(key, remain) = get_first_field(remain);
    key = key.substr(1, key.size() - 2);
    while (remain != "")
    {
        if (i == 1)
        {
            if (remain[0] == '"')
            {
                o_temp = o_temp.get<json_object>(key);
            }

            if (remain[0] == '[')
            {
                a_temp = o_temp.get<json_array>(key);
                i = 2;
            }
        }
        else if (i == 2)
        {

            if (remain[0] == '"')
            {
                o_temp = a_temp.get<json_object>(std::stoi(key));
                i = 1;
            }

            if (remain[0] == '[')
            {
                a_temp = a_temp.get<json_array>(std::stoi(key));
            }
        }
        std::tie(key, remain) = get_first_field(remain);
        key = key.substr(1, key.size() - 2);
    }

    if (i == 1)
    {
        return o_temp.get<double>(key);
    }
    return a_temp.get<double>(std::stoi(key));
}

template <>
inline bool Document::query<bool>(const std::string &field) const
{
    json_object o_temp;
    json_array a_temp;

    // get key and remain
    auto [key, remain] = get_first_field(field);
    key = key.substr(1, key.size() - 2);
    int i;
    if (remain != "")
    {
        if (remain[0] == '"')
        {
            o_temp = this->get<json_object>(key);
            i = 1;
        }
        if (remain[0] == '[')
        {
            a_temp = this->get<json_array>(key);
            i = 2;
        }
    }

    std::tie(key, remain) = get_first_field(remain);
    key = key.substr(1, key.size() - 2);
    while (remain != "")
    {
        if (i == 1)
        {
            if (remain[0] == '"')
            {
                o_temp = o_temp.get<json_object>(key);
            }

            if (remain[0] == '[')
            {
                a_temp = o_temp.get<json_array>(key);
                i = 2;
            }
        }
        else if (i == 2)
        {
            if (remain[0] == '"')
            {
                o_temp = a_temp.get<json_object>(std::stoi(key));
                i = 1;
            }

            if (remain[0] == '[')
            {
                a_temp = a_temp.get<json_array>(std::stoi(key));
            }
        }
        std::tie(key, remain) = get_first_field(remain);
        key = key.substr(1, key.size() - 2);
    }

    if (i == 1)
    {
        return o_temp.get<bool>(key);
    }
    return a_temp.get<bool>(std::stoi(key));
}

template <>
inline json_object Document::query<json_object>(const std::string &field) const
{
    json_object o_temp;
    json_array a_temp;

    // get key and remain
    auto [key, remain] = get_first_field(field);
    key = key.substr(1, key.size() - 2);
    int i;
    if (remain != "")
    {
        if (remain[0] == '"')
        {
            o_temp = this->get<json_object>(key);
            i = 1;
        }
        if (remain[0] == '[')
        {
            a_temp = this->get<json_array>(key);
            i = 2;
        }
    }

    std::tie(key, remain) = get_first_field(remain);
    key = key.substr(1, key.size() - 2);
    while (remain != "")
    {
        if (i == 1)
        {
            if (remain[0] == '"')
            {
                o_temp = o_temp.get<json_object>(key);
            }

            if (remain[0] == '[')
            {
                a_temp = o_temp.get<json_array>(key);
                i = 2;
            }
        }
        else if (i == 2)
        {

            if (remain[0] == '"')
            {
                o_temp = a_temp.get<json_object>(std::stoi(key));
                i = 1;
            }

            if (remain[0] == '[')
            {
                a_temp = a_temp.get<json_array>(std::stoi(key));
            }
        }
        std::tie(key, remain) = get_first_field(remain);
        key = key.substr(1, key.size() - 2);
    }

    if (i == 1)
    {
        return o_temp.get<json_object>(key);
    }
    return a_temp.get<json_object>(std::stoi(key));
}

template <>
inline json_array Document::query<json_array>(const std::string &field) const
{
    json_object o_temp;
    json_array a_temp;

    // get key and remain
    auto [key, remain] = get_first_field(field);
    key = key.substr(1, key.size() - 2);
    int i;
    if (remain != "")
    {
        if (remain[0] == '"')
        {
            o_temp = this->get<json_object>(key);
            i = 1;
        }
        if (remain[0] == '[')
        {
            a_temp = this->get<json_array>(key);
            i = 2;
        }
    }

    std::tie(key, remain) = get_first_field(remain);
    key = key.substr(1, key.size() - 2);
    while (remain != "")
    {
        if (i == 1)
        {
            if (remain[0] == '"')
            {
                o_temp = o_temp.get<json_object>(key);
            }

            if (remain[0] == '[')
            {
                a_temp = o_temp.get<json_array>(key);
                i = 2;
            }
        }
        else if (i == 2)
        {

            if (remain[0] == '"')
            {
                o_temp = a_temp.get<json_object>(std::stoi(key));
                i = 1;
            }

            if (remain[0] == '[')
            {
                a_temp = a_temp.get<json_array>(std::stoi(key));
            }
        }
        std::tie(key, remain) = get_first_field(remain);
        key = key.substr(1, key.size() - 2);
    }

    if (i == 1)
    {
        return o_temp.get<json_array>(key);
    }
    return a_temp.get<json_array>(std::stoi(key));
}

inline size_t Document::next_id = 0;

inline std::string Document::query_as_string(const std::string &path) const
{
    json_object o_temp;
    json_array a_temp;

    // get key and remainder
    auto [key, remain] = get_first_field(path);
    key = key.substr(1, key.size() -2);

    if (remain == "")
    { 
        auto fields = tokenize_json(data);
        for (size_t i = 0; i < fields.size(); i += 2)
        {
            if (fields[i] == key)
            {
                return fields[i + 1];
            }
        }
        throw std::runtime_error("Field does not exist");
    }

    int i;
    if (remain[0] == '"')
    {
        o_temp = this->get<json_object>(key);
        i = 1;
    }
    if (remain[0] == '[')
    {
        a_temp = this->get<json_array>(key);
        i = 2;
    }

    std::tie(key, remain) = get_first_field(remain);
    key = key.substr(1, key.size() - 2);
    while (remain.size() != 0)
    {
        if (i == 1)
        {
            if (remain[0] == '"')
            {
                o_temp = o_temp.get<json_object>(key);
            }
            if (remain[0] == '{')
            {
                a_temp = o_temp.get<json_array>(key);
                i = 2;
            }
        }
        else
        {
            if (remain[0] == '"')
            {
                o_temp = a_temp.get<json_object>(std::stoi(key));
                i = 1;
            }
            if (remain[0] == '{')
            {
                a_temp = a_temp.get<json_array>(std::stoi(key));
            }
        }
        std::tie(key, remain) = get_first_field(remain);
        key = key.substr(1, key.size() - 2);
    }
    std::string s;
    if (i == 1)
        s = o_temp.get_as_string(key);
    else
        s = a_temp.get_as_string(std::stoi(key));
    return s;
    
}


class Collection
{
public:
    Collection(const std::string &name)
    { // TODO: Israel
        this->name = name;
    }

    Collection(const std::string &name, const std::string &filepath)
    { // TODO: Israel
        this->name = name;
        load_file = filepath;
        if (!std::filesystem::exists(filepath))
        {
            throw std::runtime_error("filepath: " + filepath + " does not exist");
        }
        
    }

    void load(const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filepath);

        std::string buffer, filedata;
        while (std::getline(file, buffer))
        {
            filedata += buffer;
        }
        auto e = tokenize_json(filedata);
        for (size_t i = 0; i < e.size(); i += 2)
        {
            documents.emplace_back(std::stoi(e[i]), e[i + 1]);
        }
        file.close();
    }

    void cache(const std::string &filepath)
    {
        std::ofstream file(filepath);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filepath);
        file << "{\n";
        for (const auto &d : documents)
        {
            file << '"' << d.id << "\":" << d.data << ",\n";
        }
        file.seekp(file.tellp() - 2l);
        file << "\n}";
        file.close();
    }

    void read(const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filepath);

        std::string buffer, filedata;
        std::getline(file, buffer);
        if (buffer.size() == 0) // empty file
        {
            file.close();
            return;
        }
        if (buffer[0] == '[') // data as array
        {
            do
            {
                filedata += buffer;
            }
            while (std::getline(file, buffer));

            filedata = de_whitespace_json(filedata);

            auto entries = tokenize_array(filedata);
            for (const auto &entry : entries)
            {
                documents.emplace_back(entry);
            }
            file.close();
            return;
        }

        // data as undelimited objects
        do
        {
            filedata += buffer;
            size_t i = match_bracket(filedata, 0);

            if (i != std::string::npos)
            {
                i++;
                documents.emplace_back(filedata.substr(0, i));
                filedata = filedata.substr(i);
            }
        }
        while(std::getline(file, buffer));

        file.close();
    }

    void save(const std::string &filepath)
    {
        std::ofstream file(filepath);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filepath);
        file << "[\n";
        if (documents.size() == 0)
        {
            file << ']';
            file.close();
            return;
        }

        for (size_t i = 0; i < documents.size() - 1; i++)
        {
            file << '\t' << documents[i].data << ",\n";
        }
        file << '\t' << documents.back().data << "\n]";

        file.close();
    }

    const std::string &get_name()
    {
        return name;
    }

    void change_name(const std::string &new_name)
    {
        name = new_name;
    } // TODO:

    // C
    size_t add_document(const std::string &json)
    {

        documents.emplace_back(json);
        return documents.back().get_id();
    } // TODO:

    // R
    const Document &get_document(size_t id)
    {
        size_t left = 0;
        size_t right = documents.size()-1;
        size_t center;
        size_t c_id;
        while(true){
            center = (right + left)/2;
            c_id = documents[center].id;
            if(left >= right){
                if(c_id != id){
                    throw std::runtime_error("Could not find document with id: " + std::to_string(id));
                }
            }
            if(c_id == id){
                return documents[center];
            }
            else if(c_id > id){
                right = center - 1;  
            }
            else{
                left = center + 1;
            }

        }
    }

    const std::vector<Document> get_documents(const std::string &pattern, bool parallel)
    {
        // check if documents exist in collection
        if (documents.size() == 0)
        {
            throw std::runtime_error("no documents exist in collection");
        }

        auto [keys, vals] = tokenize_pattern(de_whitespace_json(pattern));

        // Single threaded implementation
        if (!parallel)
        {
            std::vector<Document> result_vector;
            // iterate through all documents in documents vector
            for (Document &d : documents)
            {
                bool match = true;
                for (size_t i = 0; i < keys.size(); i++)
                {
                    try // This is bad. We need to replace it with a call to verify keys
                    {
                        if (d.query_as_string(keys.at(i)) != vals.at(i))
                        {
                            match = false;
                            break;
                        }
                    }
                    catch(...)
                    {
                        match = false;
                        break;
                    }
                }
                if (match == true)
                {
                    result_vector.emplace_back(d);
                }
            }

            // if document containing pattern is found, emplace_back into result vector
            // return result vector
            return result_vector;
        }


        // Parallel implementation
        
        int num_threads = omp_get_num_procs();
        std::vector<std::vector<Document>> result_vector(num_threads);
        
        for(auto& v : result_vector)
        {
            v.reserve(documents.size() / num_threads + 1);
        }

        #pragma omp parallel shared(documents) shared(result_vector)
        {
            int id = omp_get_thread_num();

            #pragma omp for 
            for (Document &d : documents)
            {
                bool match = true;
                for (size_t i = 0; i < keys.size(); i++)
                {
                    try // This is bad. We need to replace it with a call to verify keys
                    {
                        if (d.query_as_string(keys.at(i)) != vals.at(i))
                        {
                            match = false;
                            break;
                        }
                    }
                    catch(...)
                    {
                        match = false;
                        break;
                    }
                }
                if (match == true)
                {
                    result_vector[id].push_back(d);
                }
            }
        }

        for(size_t i = 1; i < result_vector.size(); i++)
        {
            result_vector[0].reserve(result_vector[i].size() + result_vector[0].size());
            std::move(result_vector[i].begin(), result_vector[i].end(), std::back_inserter(result_vector[0]));
        }

        // if document containing pattern is found, emplace_back into result vector
        // return result vector
        result_vector[0].shrink_to_fit();
        return result_vector[0];
    }

    // U
    void update_document(size_t id, const std::string &data)
    {
        std::function<void(std::string &, const std::string &)> replace_array_field;

        std::function<void(std::string &, const std::string &)> replace_object_field = [&](std::string &old_data, const std::string &new_data)
        {
            auto fields = tokenize_json(old_data);
            auto new_fields = tokenize_json(new_data);
            bool replaced = false;
            for (size_t i = 0; i < new_fields.size(); i += 2)
            {
                for (size_t j = 0; j < fields.size(); j += 2)
                {
                    if (new_fields[i] == fields[j])
                    {
                        switch (new_fields[i + 1][0])
                        {
                        case '{':
                            replace_object_field(fields[j + 1], new_fields[i + 1]);
                            break;
                        case '[':
                            replace_array_field(fields[j + 1], new_fields[i + 1]);
                            break;
                        default:
                            if (new_fields[i + 1] == "delete")
                            {
                                fields.erase(fields.begin() + j, fields.begin() + j + 2);
                                j--;
                                replaced = true;
                                continue;
                            }
                            else
                            {
                                fields[j + 1] = new_fields[i + 1];
                                replaced = true;
                                continue;
                            }
                        }
                    }
                }
                if (!replaced)
                {
                    fields.push_back(new_fields[i]);
                    fields.push_back(new_fields[i + 1]);
                }
            }
            old_data = smash_json(fields);
        };

        replace_array_field = [&](std::string &old_data, const std::string &new_data)
        {
            auto fields = tokenize_array(old_data);
            size_t back = new_data.find(':'); // not robust, needs check for formatting correctness
            size_t index = std::stoi(new_data.substr(1, back));
            auto val = new_data.substr(back + 1, new_data.size() - back - 2);

            switch (val[0])
            {
            case '{':
                replace_object_field(old_data, new_data);
                break;
            case '[':
                replace_array_field(old_data, new_data);
                break;
            default:
                fields[index] = val;
            }

            old_data = smash_array(fields);
        };

        auto formatted_data = de_whitespace_json(data);
        auto failure  = verify_json(formatted_data);
        if (failure) throw std::runtime_error(*failure);

        size_t left = 0;
        size_t right = documents.size()-1;
        size_t center;
        size_t c_id;
        while(true){
            center = (right + left)/2;
            c_id = documents[center].id;
            if(left >= right){
                if(c_id != id){
                    throw std::runtime_error("Could not find document with id: " + std::to_string(id));
                }
            }
            if(c_id == id){
                replace_object_field(documents[center].data,formatted_data);
                return;
            }
            else if(c_id > id){
                right = center - 1;  
            }
            else{
                left = center + 1;
            }

        }
    }

    void update_documents(const std::string &pattern, const std::string &data, bool parallel)
    {

        // check if documents exist in collection
        if (documents.size() == 0)
        {
            throw std::runtime_error("no documents exist in collection");
        }

        auto [keys, vals] = tokenize_pattern(de_whitespace_json(pattern));


        if (!parallel)
        {
            // iterate through all documents in documents vector
            for (Document &d : documents)
            {
                bool match = true;
                for (size_t i = 0; i < keys.size(); i++)
                {
                    try // This is bad. We need to replace it with a call to verify keys
                    {
                        std::string v = d.query_as_string(keys.at(i));
                        std::string e = vals.at(i);
                        if (v != e)
                        {
                            match = false;
                            break;
                        }
                    }
                    catch(...)
                    {
                        match = false;
                        break;
                    }
                }
                if (match == true)
                {
                    update_document(d.id, data);
                }
            }
            return;
        }


        // iterate through all documents in documents vector
        #pragma omp parallel shared(documents) shared(keys) shared(vals)
        {   
            #pragma omp for
            for (Document &d : documents)
            {
                bool match = true;
                for (size_t i = 0; i < keys.size(); i++)
                {
                    try // This is bad. We need to replace it with a call to verify keys
                    {
                        std::string v = d.query_as_string(keys.at(i));
                        std::string e = vals.at(i);
                        if (v != e)
                        {
                            match = false;
                            break;
                        }
                    }
                    catch(...)
                    {
                        match = false;
                        break;
                    }
                }
                if (match == true)
                {
                    update_document(d.id, data);
                }
            }
        }
    }

    // D
    void remove_document(size_t id)
    {
        size_t left = 0;
        size_t right = documents.size()-1;
        size_t center;
        size_t c_id;
        while(true){
            center = (right + left)/2;
            c_id = documents[center].id;
            if(left >= right){
                if(c_id != id){
                    throw std::runtime_error("Could not find document with id: " + std::to_string(id));
                }
            }
            if(c_id == id){
                documents.erase(documents.begin()+center);
                return;
            }
            else if(c_id > id){
                right = center - 1;  
            }
            else{
                left = center + 1;
            }

        }
    }

    void remove_documents(const std::string &pattern, bool parallel)
    {

        // check if documents exist in collection
        if (documents.size() == 0)
        {
            throw std::runtime_error("no documents exist in collection");
        }

        auto [keys, vals] = tokenize_pattern(de_whitespace_json(pattern));

        if (!parallel)
        { 
            // iterate through all documents in documents vector
            for (Document &d : documents)
            {
                bool match = true;
                for (size_t i = 0; i < keys.size(); i++)
                {
                    try // This is bad. We need to replace it with a call to verify keys
                    {
                        std::string v = d.query_as_string(keys.at(i));
                        std::string e = vals.at(i);
                        if (v != e)
                        {
                            match = false;
                            break;
                        }
                    }
                    catch(...)
                    {
                        match = false;
                        break;
                    }
                }
                if (match == true)
                {
                    remove_document(d.id);
                }
            }
            return;
        }


        // iterate through all documents in documents vector
        int num_threads = omp_get_num_procs();
        std::vector<std::vector<Document>> result_vector(num_threads);

        for(auto& v : result_vector)
        {
            v.reserve(documents.size() / num_threads + 1);
        }

        #pragma omp parallel shared(documents) shared(result_vector)
        {
            int id = omp_get_thread_num();
            
            #pragma omp for
            for (Document &d : documents)
            {
                bool match = true;
                for (size_t i = 0; i < keys.size(); i++)
                {
                    try // This is bad. We need to replace it with a call to verify keys
                    {
                        std::string v = d.query_as_string(keys.at(i));
                        std::string e = vals.at(i);
                        if (v != e)
                        {
                            match = false;
                            break;
                        }
                    }
                    catch(...)
                    {
                        match = false;
                        break;
                    }
                }
                if (match == false)
                {
                    result_vector[id].emplace_back(d);
                }
            }
        }

        for(size_t i = 1; i < result_vector.size(); i++)
        {
            result_vector[0].reserve(result_vector[i].size() + result_vector[0].size());
            std::move(result_vector[i].begin(), result_vector[i].end(), std::back_inserter(result_vector[0]));
        }
        
        result_vector[0].shrink_to_fit();
        documents = result_vector[0];
    }

private:
    std::string name;
    std::vector<Document> documents;
    friend class Database;
    void clear_from_ram() { documents.clear(); }
    std::string cache_file;
    std::string load_file;
};

class Database
{
public:
    Database(const std::string &filepath)
    {
        current_collection = collections.end();
        current_collection_set = false;
        temp_filepath = filepath;
        if (!std::filesystem::exists(filepath))
        {
            throw std::runtime_error("filepath: " + filepath + " does not exist");
        }
    } // TODO Israel

    ~Database()
    {
        for (auto &c : collections)
        {
            std::filesystem::remove(temp_filepath + '/' + c.get_name() + ".json.tmp");
        }
    }

    const std::vector<std::string> get_collection_names()
    {

        std::vector<std::string> name_vector;
        for (Collection &c : collections)
        {

            name_vector.emplace_back(c.get_name());
        }

        return name_vector;
    } // possibly add caching, and return reference

    void change_collection_name(const std::string &old_name, const std::string &new_name)
    {

        for (Collection &c : collections)
        {
            if (c.get_name() == new_name)
            {
                return;
            }
        }

        for (Collection &c : collections)
        {
            if (c.get_name() == old_name)
            {
                c.change_name(new_name);
            }
        }
    }

    void set_current_collection(const std::string &name)
    {
        if (current_collection_set)
        {
            if (current_collection->get_name() == name)
            {
                return;
            }
        }
        for (auto c = collections.begin(); c != collections.end(); c++)
        {
            if (c->get_name() == name)
            {
                if (current_collection_set)
                {
                    current_collection->cache(temp_filepath + '/' + current_collection->get_name() + ".json.tmp");
                    current_collection->clear_from_ram();
                }
                current_collection_set = true;
                current_collection = c;
                if(c->load_file.empty())
                {
                    if (std::filesystem::exists(temp_filepath + '/' + c->get_name() + ".json.tmp"))
                        current_collection->load(temp_filepath + '/' + c->get_name() + ".json.tmp");
                }
                else
                {
                    if (std::filesystem::exists(c->load_file))
                        current_collection->read(c->load_file);
                    c->load_file = "";
                }
                return;
            }
        }
        throw std::runtime_error("No collection with name");
    }

    void add_collection(const std::string &name)
    {

        for (auto &c : collections)
        {
            if (c.get_name() == name)
            {
                throw std::runtime_error("name already exists");
            }
        }
        if (current_collection_set && collections.size() == collections.capacity())
        {
            std::string cc_name = current_collection->get_name();
            collections.emplace_back(name);
            for (auto c = collections.begin(); c != collections.end(); c++)
            {
                if (c->get_name() == cc_name)
                {
                    current_collection = c;
                    return;
                }
            }
        }
        collections.emplace_back(name);
    }

    void remove_collection(const std::string name)
    {
        for (auto c = collections.begin(); c != collections.end(); c++)
        {
            if (c->get_name() == name)
            {
                collections.erase(c);
                if (current_collection > c)
                {
                    current_collection--;
                }
            }
        }
        throw std::runtime_error("Collection does not exist");
    }

    void add_collection_from_file(const std::string& name, const std::string& filepath)
    {
        for(auto& c:collections)
        {
            if(c.get_name() == name)
            {
                throw std::runtime_error("Name already exists");
            }
        }

        if (current_collection_set && collections.size() == collections.capacity())
        {
            std::string cc_name = current_collection->get_name();
            collections.emplace_back(name);
            for (auto c = collections.begin(); c != collections.end(); c++)
            {
                if (c->get_name() == cc_name)
                {
                    current_collection = c;
                    return;
                }
            }
        }
        collections.emplace_back(name,filepath);
    }

    void save_current_collection(const std::string &filepath){
        if(current_collection_set == false){
            throw std::runtime_error("no current collection cannot save");
        }
        current_collection->save(filepath);
    }
    //add save all collections? would require collections to store filepath

    void load_current_collection(const std::string &filepath)
    {
        if(current_collection_set == false){
            throw std::runtime_error("no current collection cannot load");
        }
        current_collection->load(filepath);
    }

    std::vector<size_t>get_ids()
    {
        std::vector<size_t> ret;
        if(current_collection_set == false){
            throw std::runtime_error("no current collection could not get ids");
        }
        for(const auto& d : current_collection->documents){
            ret.push_back(d.get_id());
        }
        return ret;
    }

    std::vector<Document>::const_iterator begin() const
    {
        if(current_collection_set == false){
            throw std::runtime_error("no current collection");
        }
        return current_collection->documents.cbegin();
    }

    std::vector<Document>::const_iterator end() const
    {
        if(current_collection_set == false){
            throw std::runtime_error("no current collection");
        }
        return current_collection->documents.cend();
    }

    // top level copies of CRUD operations for direct user interface
    // C
    size_t add_document(const std::string &json)
    {
        if (collections.size() == 0)
        {
            throw std::runtime_error("No collections");
        }
        if (current_collection_set == false)
        {
            throw std::runtime_error("No current collection");
        }

        return current_collection->add_document(json);
    }

    // R
    const Document &get_document(size_t id)
    {

        // calls the analogous function call in the collection and returns the result of that

        if (current_collection_set == false)
        {
            throw std::runtime_error("no active collection");
        }
        try
        {
            return current_collection->get_document(id);
        }
        catch (std::runtime_error &e)
        {
            throw e;
        }
    }

    const std::vector<Document> get_documents(const std::string &pattern, bool parallel = true)
    { // faheds

        if (collections.size() == 0)
        {
            throw std::runtime_error("No collections");
        }
        if (current_collection_set == false)
        {
            throw std::runtime_error("No current collection");
        }

        return current_collection->get_documents(pattern, parallel);
    }

    // U
    void update_document(size_t id, const std::string &data)
    {
        if (collections.size() == 0)
        {
            throw std::runtime_error("No collections");
        }
        if (current_collection_set == false)
        {
            throw std::runtime_error("no active collection");
        }

        try
        {
            return current_collection->update_document(id, data);
        }
        catch (std::runtime_error &e)
        {
            throw e;
        }
    }

    void update_documents(const std::string &pattern, const std::string &data, bool parallel = true)
    { // fahed

        if (collections.size() == 0)
        {
            throw std::runtime_error("No collections");
        }
        if (current_collection_set == false)
        {
            throw std::runtime_error("No current collection");
        }

        current_collection->update_documents(pattern, data, parallel);
    }

    // D
    void remove_document(size_t id)
    {
        if (collections.size() == 0)
        {
            throw std::runtime_error("No collections");
        }
        if (current_collection_set == false)
        {
            throw std::runtime_error("No current collection");
        }
        try
        {
            current_collection->remove_document(id);
        }
        catch (std::runtime_error &e)
        {
            throw e;
        }
    }

    void remove_documents(const std::string &pattern, bool parallel = true)
    {

        if (collections.size() == 0)
        {
            throw std::runtime_error("No collections");
        }
        if (current_collection_set == false)
        {
            throw std::runtime_error("No current collection");
        }

        current_collection->remove_documents(pattern, true);
    }

private:
    std::string temp_filepath;
    std::vector<Collection> collections;
    std::vector<Collection>::iterator current_collection; // change to pointer? same syntax mostly
    bool current_collection_set;
};

#endif //__DATABASE_H__
