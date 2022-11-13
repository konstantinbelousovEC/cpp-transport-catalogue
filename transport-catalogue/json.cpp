#include "json.h"
#include <variant>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char e;
            if (!(input >> e)) {
                throw ParsingError("Invalid json format");
            } else {
                input.putback(e);
            }
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            return Node(move(result));
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }
            return Node(s);
        }

        Node LoadNull(istream& input, char c) {
            std::string res;
            res += c;
            for (int i = 0; input >> c && i < 3; i++) {
                res += c;
            }
            if (res == "null") {
                return Node(nullptr);
            } else {
                throw ParsingError("Invalid format of null");
            }
        }

        Node LoadBool(istream& input, char c) {
            if (c == 't') {
                std::string res;
                res += c;
                for (int i = 0; i < 3; i++) {
                    input >> c;
                    res += c;
                }
                if (res == "true") {
                    return Node(true);
                } else {
                    throw ParsingError("Invalid bool format /true/");
                }
            } else if (c == 'f') {
                std::string res;
                res += c;
                for (int i = 0; i < 4; i++) {
                    input >> c;
                    res += c;
                }
                if (res == "false") {
                    return Node(false);
                } else {
                    throw ParsingError("Invalid bool format /false/");
                }
            } else {
                throw ParsingError("Invalid bool format");
            }
        }

        Node LoadDict(istream& input) {
            Dict result;
            char e;
            if (!(input >> e)) {
                throw ParsingError("Invalid json format");
            } else {
                input.putback(e);
            }
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            return Node(move(result));
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 'n') {
                return LoadNull(input, c);
            } else if (c == 't' || c == 'f') {
                return LoadBool(input, c);
            } else if (c == ']' || c == '}') {
                throw ParsingError("invalid json format");
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }

    Node::Node()
            : element_(nullptr) {}

    Node::Node(std::nullptr_t n)
            : element_(n) {}

    Node::Node(Array array)
            : element_(move(array)) {}

    Node::Node(Dict map)
            : element_(move(map)) {}

    Node::Node(string value)
            : element_(move(value)) {}

    Node::Node(int value)
            : element_(value) {}

    Node::Node(double value)
            : element_(value) {}

    Node::Node(bool value)
            : element_(value) {}

    Document::Document(Node root)
            : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    bool Node::IsInt() const {
        return element_.index() == 4;
    }

    bool Node::IsDouble() const {
        return element_.index() == 4 || element_.index() == 5;
    }

    bool Node::IsPureDouble() const {
        return element_.index() == 5;
    }

    bool Node::IsBool() const {
        return element_.index() == 3;
    }

    bool Node::IsString() const {
        return element_.index() == 6;
    }

    bool Node::IsNull() const {
        return element_.index() == 0;
    }

    bool Node::IsArray() const {
        return element_.index() == 1;
    }

    bool Node::IsMap() const {
        return element_.index() == 2;
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(element_);
        } else {
            throw std::logic_error("the wrong method is selected");
        }
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(element_);
        } else {
            throw std::logic_error("the wrong method is selected");
        }
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(std::get<int>(element_));
        } else if (IsPureDouble()) {
            return std::get<double>(element_);
        } else {
            throw std::logic_error("the wrong method is selected");
        }
    }

    const std::string &Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(element_);
        } else {
            throw std::logic_error("the wrong method is selected");
        }
    }

    const Array &Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(element_);
        } else {
            throw std::logic_error("the wrong method is selected");
        }
    }

    const Dict &Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(element_);
        } else {
            throw std::logic_error("the wrong method is selected");
        }
    }

    void ReplaceAll(std::string& inout, std::string_view what, std::string_view with) {
        for (std::string::size_type pos{};
             inout.npos != (pos = inout.find(what.data(), pos, what.length()));
             pos += with.length())
        {
            inout.replace(pos, what.length(), with.data(), with.length());
        }
    }

    void PrintNode(const Node& node, std::ostream& out, int i);

    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out, [[maybe_unused]] int i) {
        out << value;
    }

    void PrintValue(std::nullptr_t, std::ostream& out, [[maybe_unused]] int i) {
        out << "null"s;
    }

    void PrintValue(const bool value, std::ostream& out, [[maybe_unused]] int i) {
        out << (value ? "true"s : "false"s);
    }

    void PrintValue(const std::string value, std::ostream& out, [[maybe_unused]] int i) {
        std::string str = value;
        ReplaceAll(str, "\\"sv, "\\\\"sv);
        ReplaceAll(str, "\""sv, "\\\""sv);
        ReplaceAll(str, "\r"sv, "\\r"sv);
        ReplaceAll(str, "\n"sv, "\\n"sv);
        out << "\"" << str << "\"";
    }

    void PrintValue(const std::map<std::string, Node>& value, std::ostream& out, [[maybe_unused]] int i) {
        std::string tab;
        tab.append(i, '\t');
        out << "{"<< std::endl;
        size_t size = value.size();
        for (const auto& [key, val] : value) {
            out << tab;
            out << "\"" << key << "\": ";
            PrintNode(val, out, i + 1);
            if (size > 1) out << "," << std::endl;
            size--;
        }
        tab.pop_back();
        out << std::endl << tab << "}";
    }

    void PrintValue(const std::vector<Node>& value, std::ostream& out, [[maybe_unused]] int i) {
        std::string tab;
        tab.append(i, '\t');
        out << "[" << std::endl;
        size_t size = value.size();
        for (const Node& elem : value) {
            out << tab;
            PrintNode(elem, out, i + 1);
            if (size > 1) out << ", ";
            out << std::endl;
            size--;
        }
        tab.pop_back();
        out << tab << "]";
    }

    void PrintNode(const Node& node, std::ostream& out, int i) {
        std::visit(
                [&out, &i](const auto& value){ PrintValue(value, out, i); },
                node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        int i = 1;
        PrintNode(doc.GetRoot(), output, i);
    }

}