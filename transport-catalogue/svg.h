#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <vector>
#include <optional>
#include <variant>
#include <sstream>

namespace svg {

    struct Point {
        Point() = default;
        Point(double x, double y)
                : x(x)
                , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        explicit RenderContext(std::ostream& out)
                : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
                : out(out)
                , indent_step(indent_step)
                , indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;
    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap elem);
    std::ostream& operator<<(std::ostream& out, StrokeLineJoin elem);

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
                : red(r), green(g), blue(b) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        std::string to_string() const {
            std::stringstream str;
            str << "rgb(" << static_cast<int>(red) << "," << static_cast<int>(green) << "," << static_cast<int>(blue) << ")";
            return str.str();
        }
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
                : red(r), green(g), blue(b), opacity(op) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
        std::string to_string() const {
            std::stringstream str;
            str << "rgba(" << static_cast<int>(red) << "," << static_cast<int>(green) << "," << static_cast<int>(blue) << "," << opacity << ")";
            return str.str();
        }
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const std::string NoneColor{"none"};

    struct ColorPrinter {
        std::string operator()(std::monostate) {
            return NoneColor;
        }
        std::string operator()(std::string value) {
            return value;
        }
        std::string operator()(Rgb rgb) {
            return rgb.to_string();
        }
        std::string operator()(Rgba rgba) {
            return rgba.to_string();
        }
    };

    std::ostream& operator<<(std::ostream& out, Color elem);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        };
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        };
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        };
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        };
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        };
    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            if (fill_color_) out << " fill=\""sv << *fill_color_ << "\""sv;
            if (stroke_color_) out << " stroke=\""sv << *stroke_color_ << "\""sv;
            if (stroke_width_) out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            if (line_cap_) out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            if (line_join_) out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const override;
        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext &context) const override;
        std::deque<Point> points_;
    };

    class Text final : public Object, public PathProps<Text> {
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);
    private:
        void RenderObject(const RenderContext &context) const override;
        std::string EscapingSpecialCharacters(std::string& str);
        void ReplaceAll(std::string& inout, std::string_view what, std::string_view with);
        Point position_ = {0, 0};
        Point offset_ = {0, 0};
        uint32_t font_size_ = 1;
        std::string font_family_ = "";
        std::string font_weight_ = "";
        std::string data_ = "";
    };

    class ObjectContainer {
    public:
        template<typename Obj>
        void Add(Obj obj) {
            objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    protected:
        ~ObjectContainer() = default;
        std::vector<std::unique_ptr<Object>>& GetObjects();
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Document : public ObjectContainer{
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out);
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };
}