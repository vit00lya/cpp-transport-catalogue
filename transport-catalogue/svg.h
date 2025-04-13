#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <iostream>
#include <variant>
#include <sstream>
#include <iomanip>
#include "domain.h"

using namespace std::literals;

namespace svg {

struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

inline std::string doubleToString(double value) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << value; // Устанавливаем фиксированное представление и точность
    return ss.str(); // Возвращаем строку
}

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline const std::monostate NoneColor ;


enum class StrokeLineCap {
    BUTT, ROUND, SQUARE,
};

enum class StrokeLineJoin {
    ARCS, BEVEL, MITER, MITER_CLIP, ROUND,
};


inline std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& obj) {
   // Здесь напишите код для вывода объекта obj
   switch (obj) {
       case svg::StrokeLineJoin::ARCS:
           out << "arcs";
           break;
       case svg::StrokeLineJoin::BEVEL:
           out << "bevel";
           break;
       case svg::StrokeLineJoin::MITER:
           out << "miter";
           break;
       case svg::StrokeLineJoin::MITER_CLIP:
           out << "miter-clip";
           break;
       case svg::StrokeLineJoin::ROUND:
           out << "round";
           break;
   }
   return out;
}

inline std::ostream& operator<<(std::ostream& out, const StrokeLineCap& obj) {
   // Здесь напишите код для вывода объекта obj
   switch (obj) {
       case svg::StrokeLineCap::BUTT:
           out << "butt";
           break;
       case svg::StrokeLineCap::ROUND:
           out << "round";
           break;
       case svg::StrokeLineCap::SQUARE:
           out << "square";
           break;
   }
   return out;
}

inline std::ostream& operator<<(std::ostream& out, const Color& v) {
    if (std::holds_alternative<std::monostate>(v)) {
        out << "none"s;
    } else if (std::holds_alternative<std::string>(v)) {
        out << std::get<std::string>(v);
    } else if (std::holds_alternative<Rgb>(v)) {
        auto& rgb = std::get<Rgb>(v);
        out << "rgb("s << std::to_string(rgb.red) << ","s + std::to_string(rgb.green) << ","s << std::to_string(rgb.blue) << ")"s;
    } else {
        auto& rgba = std::get<Rgba>(v);
        out << "rgba("s << std::to_string(rgba.red) << ","s << std::to_string(rgba.green) << ","s << std::to_string(rgba.blue)
                            <<  ","s << rgba.opacity << ")"s;
    }
    return out;
}

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream &out) :
            out(out) {
    }

    RenderContext(std::ostream &out, int indent_step, int indent = 0) :
            out(out), indent_step(indent_step), indent(indent) {
    }

    RenderContext Indented() const {
        return
        {   out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream &out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    virtual void Render(const RenderContext &context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext &context) const = 0;
};

template<typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color){
       fill_color_ = std::move(color);
       return AsOwner();
    }
    Owner& SetStrokeColor(Color color){
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width){
        width_ =  width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ =  line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ =  line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;



    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream &out) const
    {
        using namespace std::literals;


        if (fill_color_)
        {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }

        if (stroke_color_)
        {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (width_)
        {
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (line_cap_)
        {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_)
        {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:

    Circle() = default;
    Circle& SetCenter(domain::Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext &context) const override;

    domain::Point center_ = { 0.0, 0.0 };
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(domain::Point point);

private:
    void RenderObject(const RenderContext &context) const override;
    std::vector<domain::Point> points_;

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:

    Text() = default;
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(domain::Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(domain::Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>

private:

    std::string ScreeningText(std::string_view text);
    void RenderObject(const RenderContext &context) const override;
    domain::Point pos_ = { 0.0, 0.0 };
    domain::Point offset_ = { 0.0, 0.0 };
    uint32_t size_ = 1;
    std::string font_family_ = "";
    std::string font_weight_ = "";
    std::string data_ = "";

};

class ObjectContainer {

public:
    template<typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

    virtual ~ObjectContainer() {
    }
    ;

protected:
    std::vector<std::unique_ptr<Object>> objects_;

};

class Drawable {
public:

    virtual void Draw(ObjectContainer &container) const = 0;

    virtual ~Drawable() {
    }
    ;
};

class Document: public ObjectContainer {
public:

// Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object> &&obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream &out) const;

};
}  // namespace svg

