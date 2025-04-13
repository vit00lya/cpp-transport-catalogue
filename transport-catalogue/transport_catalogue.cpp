#include "transport_catalogue.h"

using namespace std::literals;

namespace transport_catalogue {

    // Если остановки такой не было словаре тогда добавляем
    void TransportCatalogue::AddStop(const domain::Stop &stop) {

        // Если имя остановки не заполнено тогда не нужно добавлять в список.
        if (stop.name == ""s) {
            return;
        }

        auto it = stopname_to_stop_.find(stop.name);
        if (it == stopname_to_stop_.end()) {
            stops_.push_back(std::move(stop));
            stopname_to_stop_[stops_.back().name] = &stops_.back();
        }

    }

    const domain::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {

        if (stop_name == ""s) {
            return nullptr;
        }

        auto it = stopname_to_stop_.find(std::string{stop_name}); // @suppress("Symbol is not resolved")
        if (it != stopname_to_stop_.end()) {
            return it->second;
        }

        return nullptr;
    }

    void TransportCatalogue::AddBus(std::string bus_name,
                                    const std::vector<std::string_view> stops,
                                    bool ring_route) {

        if(bus_name == ""s){
           return;
        }

        // Есть маршрут с таким именем пропускаем
        auto bus_it = busname_to_bus_.find(bus_name);
         if (bus_it != busname_to_bus_.end()) {
            return;
        }

        domain::Bus bus;
        bus.name = bus_name;
        const domain::Stop* stp;

        std::unordered_map<std::string_view, const domain::Stop*, HasherStopBus> unique_stops; // @suppress("Invalid template argument")

        for(const std::string_view& stop_str: stops){
            stp = FindStop(stop_str);
            if(stp == nullptr) {
                return; // Остановка не найдена прекращаем выполнение процедуры
            }

            bus.stop.push_back(stp);

            unique_stops[stop_str] = stp;
            // Добавляем в словарь остановок информацию о автобусе
            auto& v_str = buses_stop_at_stops_[stp->name];
            auto it = v_str.find(bus_name);
            if (it == v_str.end()){
               v_str.insert(bus.name);
            }
        }

        bus.unique_stops_count = unique_stops.size(); // Количество уникальных остновок
        bus.ring_route = ring_route; // Колцевой маршрут
        UpdateDistanceBetweenStops(bus);
        buses_.push_back(std::move(bus));
        busname_to_bus_[buses_.back().name] = &buses_.back();

    }

    domain::BusInfo TransportCatalogue::GetBusInfo(const std::string_view& bus_name) const{
        domain::BusInfo bus_info;
        if (bus_name == ""s) {
              return bus_info;
        }

        auto it = busname_to_bus_.find(bus_name);
        if (it == busname_to_bus_.end()) {
              return bus_info;
        }

        bus_info.found = true;
        bus_info.unique_stops_count = it->second->unique_stops_count;
        bus_info.no_unique_stops_count = it->second->stop.size();
        bus_info.route_length = GetRouteLengthForBus(it->second);
        bus_info.curvature = bus_info.route_length / GetRouteLengthGeographicalCoordinatesForBus(it->second);
        return bus_info;
    }

    const std::set<std::string> TransportCatalogue::GetStopInfo(const std::string_view& stop_name) const{
        auto it = buses_stop_at_stops_.find(stop_name);
        if (it == buses_stop_at_stops_.end()) {
              std::set<std::string> tmp;
              return tmp;
        }
        return it->second;
    }

    const domain::Bus* TransportCatalogue::FindBus(std::string bus_name) const {

        if (bus_name == ""s) {
            return nullptr;
        }

        auto it = busname_to_bus_.find(bus_name);
        if (it != busname_to_bus_.end()) {
            return it->second;
        }

        return nullptr;
    }

    size_t TransportCatalogue::NumberOfStops(){
        return stopname_to_stop_.size();
    }

    size_t TransportCatalogue::NumberOfRoutes(){
        return busname_to_bus_.size();
    }

    void TransportCatalogue::UpdateDistanceBetweenStops(const domain::Bus &bus){
        auto it = bus.stop.begin();
        const auto &end = bus.stop.end();
        std::pair<const domain::Stop*,const domain::Stop*>dist;
        while(it != end){
            if(std::next(it) != end){ // @suppress("Invalid arguments")
               dist = {*it,*std::next(it)}; // @suppress("Invalid arguments")
               auto find_dist = distance_between_stops_geographical_coordinates_.find(dist);
               if(find_dist == distance_between_stops_geographical_coordinates_.end()){
                   distance_between_stops_geographical_coordinates_[dist] = ComputeDistance(dist.first->coordinates,dist.second->coordinates);
               }
            }
            ++it;
        }
    }

    void TransportCatalogue::SetDistanceBetweenStop(std::string_view stp_to_sv, const domain::Stop* stp_from, uint32_t distance){

        const auto stp_to = FindStop(stp_to_sv);
      //  std::cerr << "to " << stp_to->name << " from " << stp_from->name << " dist " << distance << std::endl;
        distance_between_stops_from_route_[{stp_from, stp_to}] = distance;
        distance_between_stops_[{stp_to, stp_from}] = distance;

    }


    uint32_t TransportCatalogue::GetDistanceBetweenStops (
            std::pair<const domain::Stop*, const domain::Stop*> &dist) const{
        auto find_dist = distance_between_stops_.find(dist);
        if (find_dist != distance_between_stops_.end()) {
            auto dist = *find_dist;
            return dist.second;
        }
        return 0;
    }

    uint32_t TransportCatalogue::GetDistanceBetweenStops(const domain::Stop* stop_1, const domain::Stop* stop_2) const {
         if (distance_between_stops_from_route_.count({stop_1, stop_2}) > 0) {
             return distance_between_stops_from_route_.at({stop_1, stop_2});
         } else if (distance_between_stops_from_route_.count({stop_2, stop_1})) {
             return distance_between_stops_from_route_.at({stop_2, stop_1});
         } else {
             return 0;
         }
     }

    double TransportCatalogue::GetRouteLengthForBus(const domain::Bus *bus) const {
        uint32_t result = 0;

        auto it = bus->stop.begin();
        const auto &end = bus->stop.end();
        std::pair<const domain::Stop*, const domain::Stop*> dist;
        try {
            while (it != end) {
                if (std::next(it) != end) { // @suppress("Invalid arguments")
                    dist = { *std::next(it), *it }; // @suppress("Invalid arguments")
                    uint32_t tmp = GetDistanceBetweenStops(dist);
                    if (tmp == 0) {
                        dist = { *it, *std::next(it) }; // @suppress("Invalid arguments")
                        result += GetDistanceBetweenStops(dist);
                    } else
                        result += tmp;
                }
                ++it;
            }
        } catch (...) {
            return 0;
        }

        return result;
    }

    double TransportCatalogue::GetRouteLengthGeographicalCoordinatesForBus(const domain::Bus* bus) const {
        double result = 0;
        auto it = bus->stop.begin();
        const auto &end = bus->stop.end();
        std::pair<const domain::Stop*, const domain::Stop*>dist;
        try{
            while(it != end){
                if(std::next(it) != end){ // @suppress("Invalid arguments")
                   dist = {*it,*std::next(it)}; // @suppress("Invalid arguments")
                   auto find_dist = distance_between_stops_geographical_coordinates_.find(dist);
                   if(find_dist != distance_between_stops_geographical_coordinates_.end()){
                       auto dist = *find_dist;
                       result += dist.second; // @suppress("Field cannot be resolved")
                   }
                }
                ++it;
            }
        } catch (...){
            return 0.0;
        }
        return result;
    }

    bool TransportCatalogue::StopExists(std::string_view stp_name) const{
        auto it = stopname_to_stop_.find(stp_name);
        return !(it == stopname_to_stop_.end());
    }

    BusesListPointer TransportCatalogue::GetBuses() const {
        std::set<std::string_view> result;
        for(const domain::Bus& bus: buses_){
            result.insert(bus.name);
        }
       return std::make_unique<std::set<std::string_view>>(result);
    }

    domain::StopCoordinatesListPointer TransportCatalogue::GetCoordinatesStopBuses(std::set<std::string_view> buses_names) const { // @suppress("Member declaration not found")
        domain::RouteInfo result;
        for(const auto &bus_name :buses_names){
            domain::BusInfoMap bus_info_item;
            const domain::Bus* bus = FindBus(std::string{bus_name}); // @suppress("Symbol is not resolved")
            auto it = bus->stop.begin();
            const auto &end = bus->stop.end();
                std::vector<geo::Coordinates> coord;
                while (it != end) {
                    auto res = *it;
                    domain::StopInfoMap stp;
                    stp.coor = res->coordinates;

                    bus_info_item.coor.push_back(res->coordinates); // @suppress("Field cannot be resolved") // @suppress("Invalid arguments")
                    result.stop_info.insert({res->name, stp});
                    ++it;
                }
                bus_info_item.ring_route = bus->ring_route;
                result.bus_info.insert({bus_name,std::move(bus_info_item)});
        }
       return std::make_unique<domain::RouteInfo>(result);
    }

    size_t TransportCatalogue::GetAmountOfUsedStops() const {
        return buses_stop_at_stops_.size();
    }

    // Получить используемые автобусные остановки
    std::vector<std::string_view> TransportCatalogue::GetUsedStopNames() const {
        std::vector<std::string_view> used_stops_cash;
        for (auto& buses_through_stop : buses_stop_at_stops_) {
            used_stops_cash.push_back(buses_through_stop.first);
        }
        return used_stops_cash;
    }

    const std::unordered_map<std::string_view, domain::Bus*, HasherStopBus>& TransportCatalogue::GetBusIndexes() const {
        return busname_to_bus_;
    }

}
