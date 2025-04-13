#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(domain::Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    size_t len = points_.size();
    size_t i = 1;
    for(const domain::Point& point : points_){
        out << point.x << "," << point.y;
        if(len != i){
            out  << " ";
        }
        ++i;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(domain::Point center)  {
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
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

std::string Text::ScreeningText(std::string_view text) {
    std::string result;
    for (char ch : text){
        if (ch == '"'){
            result += "&quot;";
        }
        else if (ch == '<'){
           result += "&lt;";
        }
        else if (ch == '>'){
            result += "&gt;";
        }
        else if (ch == '&'){
            result += "&amp;";
        }
        else if (ch == '\''){
            result += "&apos;";
        }
        else{
            result += ch;
        }
    }
    return result;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv<< pos_.x <<"\""sv << " y=\""sv<< pos_.y <<"\""sv;
    out << " dx=\""sv<< offset_.x <<"\""sv << " dy=\""sv<< offset_.y <<"\""sv;

    out << " font-size=\""sv<< size_ <<"\""sv ;
    if(font_family_ != ""s){
        out  << " font-family=\""sv<< font_family_ <<"\""sv;
    }
    if(font_weight_ != ""s){
        out << " font-weight=\""sv<< font_weight_ <<"\"";
    }
    out << ">"sv << data_ <<"</text>"sv;
}

   Text& Text::SetPosition(domain::Point pos){
       pos_ = pos;
       return *this;
   }

   // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
   Text& Text::SetOffset(domain::Point offset){
       offset_ = offset;
       return *this;
   }

   // Задаёт размеры шрифта (атрибут font-size)
   Text& Text::SetFontSize(uint32_t size){
       size_ = size;
       return *this;
   }

   // Задаёт название шрифта (атрибут font-family)
   Text& Text::SetFontFamily(std::string font_family){
       font_family_ = font_family;
       return *this;
   }

   // Задаёт толщину шрифта (атрибут font-weight)
   Text& Text::SetFontWeight(std::string font_weight){
       font_weight_ = font_weight;
       return *this;
   }

   // Задаёт текстовое содержимое объекта (отображается внутри тега text)
   Text& Text::SetData(std::string data){
       std::string_view data_sv(data);
       data_ = ScreeningText(data_sv);
       return *this;
   }



// ---------- Document ------------------
void Document::Render(std::ostream &out) const{

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl; // @suppress("Function cannot be resolved") // @suppress("Invalid overload")
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv  << std::endl; // @suppress("Function cannot be resolved") // @suppress("Invalid overload")

    RenderContext context(out,1,2);
    for(auto& obj : objects_){
        obj->Render(context);
    }
    out << "</svg>";


}

void Document::AddPtr(std::unique_ptr<Object> &&obj){
    objects_.push_back(std::move(obj));
}

}  // namespace svg


