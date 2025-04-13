#include "map_renderer.h"
namespace map_render {


	void MapRender::RenderSvg(const domain::StopCoordinatesListPointer& stop_list_pointer, std::ostringstream &out) {

		svg::Document doc;
		const std::unique_ptr<map_render::SphereProjector> sp = SettingSphereProjector(stop_list_pointer);

		const auto &sp_link = *sp;
		auto &stop_list_pointer_p = *stop_list_pointer;
		for(auto& [key, value]: stop_list_pointer_p.bus_info){
			for(const auto &geo_coord: value.coor){
				value.coor_xy.push_back(sp_link(geo_coord)); // @suppress("Invalid arguments")
			}
		}

		for(auto& [key, value]: stop_list_pointer_p.stop_info){
			value.coor_xy = (sp_link(value.coor)); // @suppress("Invalid arguments")
		}

		stop_list_pointer_p.bus_info.size();

		OutputLineLayer(stop_list_pointer, doc);
		OutputRouteNamesreferences(stop_list_pointer, doc);
		OutputStopSymbol(stop_list_pointer, doc);
		OutputStopName(stop_list_pointer, doc);
		doc.Render(out);

	}

	void MapRender::OutputLineLayer(const CoordinatesListPointer &list_pointer, svg::Document &doc) const{
			using namespace svg;
			size_t num_color = 0; size_t size_color = render_settings_.color_palette.size() - 1;
			 auto &stop_list_pointer_p = *list_pointer;
			for(const auto& [key, value]: stop_list_pointer_p.bus_info){
				  Polyline poli = Polyline();
				  for(const auto &point: value.coor_xy){
					  poli.AddPoint(point);
				  }
				  doc.Add(poli.SetStrokeLineJoin(StrokeLineJoin::ROUND)
								  .SetStrokeLineCap(StrokeLineCap::ROUND)
								  .SetFillColor("none"s)
								  .SetStrokeWidth(render_settings_.line_width)
								  .SetStrokeColor(render_settings_.color_palette[num_color]));
					if(num_color < size_color) ++num_color; else num_color = 0;
		   }
	}
//
	void MapRender::OutputStopName(const CoordinatesListPointer &list_pointer, svg::Document &doc) const{
			using namespace svg;
			auto &stop_list_pointer_p = *list_pointer;
			for(const auto& [key, value]: stop_list_pointer_p.stop_info){
					  doc.Add(Text()
							  .SetFontFamily("Verdana"s)
							  .SetFontSize(render_settings_.stop_label_font_size)
							  .SetPosition(value.coor_xy)
							  .SetOffset({render_settings_.stop_label_offset.x, render_settings_.stop_label_offset.y})
							  .SetData(std::string(key))
							  .SetFillColor(render_settings_.underlayer_color)
							  .SetStrokeColor(render_settings_.underlayer_color)
							  .SetStrokeLineJoin(StrokeLineJoin::ROUND)
							  .SetStrokeWidth(render_settings_.underlayer_width)
							  .SetStrokeLineCap(StrokeLineCap::ROUND));
					  doc.Add(Text()
							  .SetFontFamily("Verdana"s)
							  .SetFontSize(render_settings_.stop_label_font_size)
							  .SetPosition(value.coor_xy)
							  .SetOffset({render_settings_.stop_label_offset.x, render_settings_.stop_label_offset.y})
							  .SetFillColor("black"s)
							  .SetData(std::string(key)));
		   }
	}
//
	void MapRender::OutputRouteNamesreferences(const CoordinatesListPointer &list_pointer, svg::Document &doc) const{
			using namespace svg;
			size_t num_color = 0; size_t size_color = render_settings_.color_palette.size() - 1;
			auto &stop_list_pointer_p = *list_pointer;
			for(const auto& [key, value]: stop_list_pointer_p.bus_info){
				PrintSingleStopName(value.coor_xy[0], key, doc, num_color);
				auto end_stop_two = value.coor_xy[value.coor.size() / 2];
				if (!value.ring_route && end_stop_two != value.coor_xy[0]){
					   PrintSingleStopName(end_stop_two, key, doc, num_color);
				}
			   if(num_color < size_color) ++num_color; else num_color = 0;
		   }
	}

	void MapRender::PrintSingleStopName(const domain::Point& coor,
										const std::string_view name,
										svg::Document &doc,
										const size_t &num_color) const {
		using namespace svg;

		doc.Add(Text()
				.SetFillColor(render_settings_.underlayer_color)
				.SetStrokeColor(render_settings_.underlayer_color)
				.SetStrokeWidth(render_settings_.underlayer_width)
				.SetFontFamily("Verdana"s)
				.SetFontSize(render_settings_.bus_label_font_size)
				.SetPosition(coor)
				.SetOffset({render_settings_.bus_label_offset.x, render_settings_.bus_label_offset.y})
				.SetData(std::string(name))
				.SetFontWeight("bold"s)
				.SetStrokeLineJoin(StrokeLineJoin::ROUND)
				.SetStrokeLineCap(StrokeLineCap::ROUND));

		doc.Add(Text()
				 .SetFillColor(render_settings_.color_palette[num_color])
				 .SetFontFamily("Verdana"s)
				 .SetFontSize(render_settings_.bus_label_font_size)
				 .SetPosition(coor)
				 .SetOffset({render_settings_.bus_label_offset.x, render_settings_.bus_label_offset.y})
				 .SetData(std::string(name))
				 .SetFontWeight("bold"s));

	}
//
	void MapRender::OutputStopSymbol(const CoordinatesListPointer &list_pointer, svg::Document &doc) const{
			using namespace svg;
			using namespace std::literals;
			auto &stop_list_pointer_p = *list_pointer;
			for(const auto& [key, value]: stop_list_pointer_p.stop_info){
				doc.Add(Circle().SetCenter(value.coor_xy).SetRadius(render_settings_.stop_radius).SetFillColor("white"s));
			}
	}

	std::unique_ptr<SphereProjector> MapRender::SettingSphereProjector(const domain::StopCoordinatesListPointer& stop_list_pointer){
		std::vector<geo::Coordinates> geo_coords;
		auto &stop_list_pointer_p = *stop_list_pointer;
		for(const auto& [key, value]: stop_list_pointer_p.stop_info){
				geo_coords.push_back(value.coor);
		}
		map_render::SphereProjector sp(
					 geo_coords.begin(),
					 geo_coords.end(),
					 render_settings_.width,
					 render_settings_.height,
					 render_settings_.padding);
		return std::make_unique<map_render::SphereProjector>(sp);
	}

}


