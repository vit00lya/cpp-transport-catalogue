#pragma once
#include "svg.h"
#include "geo.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include "domain.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace map_render {


    using CoordinatesListPointer = std::unique_ptr<domain::RouteInfo>;
    using ScreenCoordUnique = std::unique_ptr<std::vector<domain::Point>>;

    struct RenderSettings{
            double width,height = 0.0;
            double padding,line_width = 0.0;
            double stop_radius = 0.0;
            int bus_label_font_size = 0;
            domain::Point bus_label_offset = {0.0,0.0};
            int stop_label_font_size = 0;
            domain::Point stop_label_offset = {0.0,0.0};
            svg::Color underlayer_color = "none"s; // @suppress("Invalid arguments")
            double underlayer_width = 0.0;
            std::vector<svg::Color>color_palette;
        };


    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng; // @suppress("Field cannot be resolved")
            const double max_lon = right_it->lng; // @suppress("Field cannot be resolved")

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat; // @suppress("Field cannot be resolved")
            max_lat_ = top_it->lat; // @suppress("Field cannot be resolved")

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        domain::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRender{
        public:
            MapRender(RenderSettings& render_settings):render_settings_(render_settings){};
            void RenderSvg(const domain::StopCoordinatesListPointer& stop_list_pointer, std::ostringstream &out);
        private:
            void OutputLineLayer(const CoordinatesListPointer &list_pointer, svg::Document &doc) const;
            void OutputRouteNamesreferences(const CoordinatesListPointer &list_pointer, svg::Document &doc) const;
            void OutputStopSymbol(const CoordinatesListPointer &list_pointer, svg::Document &doc) const;
            void PrintSingleStopName(const domain::Point& coor, const std::string_view name, svg::Document &doc, const size_t &num_color) const;
            void OutputStopName(const CoordinatesListPointer &list_pointer, svg::Document &doc) const;
            std::unique_ptr<SphereProjector> SettingSphereProjector(const domain::StopCoordinatesListPointer& stop_list_pointer); // @suppress("Type cannot be resolved")
            RenderSettings render_settings_;
    };

}
