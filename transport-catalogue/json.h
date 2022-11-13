#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node();
        Node(std::nullptr_t n);
        Node(Array array);
        Node(Dict map);
        Node(std::string value);
        Node(int value);
        Node(double value);
        Node(bool value);


        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        inline bool operator==(const Node& other) const {
            return this->element_ == other.element_;
        }

        inline bool operator!=(const Node& other) const {
            return !(*this == other);
        }

        const Value& GetValue() const { return element_; }

    private:
        Value element_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        inline bool operator==(const Document& other) const {
            return this->root_ == other.root_;
        }

        inline bool operator!=(const Document& other) const {
            return !(*this == other);
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}