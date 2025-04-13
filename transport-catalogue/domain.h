#pragma once

#include <string_view>
#include <string>
#include "geo.h"
#include "vector"
#include "memory"
#include "map"
#include "set"

namespace domain{

struct Point {
    Point() = default;
    Point(double x, double y) :
            x(x), y(y) {
    }
    double x = 0;
    double y = 0;

    bool operator!=(const Point &other) {
           return (other.x != x || other.y != y);
       }
};

struct BusInfoMap{
    std::vector<geo::Coordinates> coor;
    std::vector<Point> coor_xy;
    bool ring_route;
};

struct StopInfoMap{
    geo::Coordinates coor;
    Point coor_xy;
};

struct RouteInfo{
    std::map<std::string_view, BusInfoMap> bus_info;
    std::map<std::string_view, StopInfoMap> stop_info;
};

using StopCoordinatesListPointer = std::unique_ptr<RouteInfo>;

struct Stop{

        Stop() = default;

        Stop(const std::string& stop_name, const geo::Coordinates& coordinates)
                :name(stop_name), coordinates(coordinates){}

        std::string name;
        geo::Coordinates coordinates;
    };

    struct Bus{
        std::string name;
        std::vector<const Stop*> stop;
        bool ring_route = false; // Признак колцевого маршрута
        size_t unique_stops_count = 0;
    };

    struct BusInfo{
        size_t unique_stops_count = 0;
        size_t no_unique_stops_count = 0;
        double route_length = 0;
        bool found = false;
        double curvature;
    };
}
