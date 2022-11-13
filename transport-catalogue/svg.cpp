#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << std::endl;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap elem) {
        out << (elem == StrokeLineCap::SQUARE ? "square" :
                elem == StrokeLineCap::ROUND ? "round" : "butt");
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin elem) {
        out << (elem == StrokeLineJoin::ROUND ? "round" :
                elem == StrokeLineJoin::ARCS ? "arcs" :
                elem == StrokeLineJoin::BEVEL ? "bevel" :
                elem == StrokeLineJoin::MITER ? "miter" : "miter-clip");
        return out;
    }

    std::ostream& operator<<(std::ostream& out, Color elem) {
        out << std::visit(ColorPrinter{}, elem);
        return out;
    }

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        if (points_.empty()) {
            out << "\"/>";
        } else {
            size_t size = points_.size();
            for (auto point : points_) {
                out << point.x << ","sv << point.y;
                size--;
                if (size > 0) out << " "sv;
            }
            out << "\"";
            RenderAttrs(context.out);
            out << "/>";
        }
    }

    void Text::ReplaceAll(std::string& inout, std::string_view what, std::string_view with) {
        for (std::string::size_type pos{};
             inout.npos != (pos = inout.find(what.data(), pos, what.length()));
             pos += with.length()) {
            inout.replace(pos, what.length(), with.data(), with.length());
        }
    }

    std::string Text::EscapingSpecialCharacters(std::string& str) {
        if (str.find_first_of("\"<>'&") == std::string::npos) return str;
        ReplaceAll(str, "&", "&amp;");
        ReplaceAll(str, "'", "&apos;");
        ReplaceAll(str, "\"", "&quot;");
        ReplaceAll(str, "<", "&lt;");
        ReplaceAll(str, ">", "&gt;");
        return str;
    }

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(EscapingSpecialCharacters(data));
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y;
        out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
        out << "\" font-size=\""sv << font_size_;
        if (!font_family_.empty()) out << "\" font-family=\""sv << font_family_;
        if (!font_weight_.empty()) out << "\" font-weight=\""sv << font_weight_;
        out << "\"";
        out << ">"sv << data_ << "</text>"sv;
    }

    std::vector<std::unique_ptr<Object>> &ObjectContainer::GetObjects() {
        return objects_;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        GetObjects().emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
        for (auto& elem : GetObjects()) {
            elem->Render({out, 2, 2});
        }
        out << "</svg>" << std::endl;
    }
}