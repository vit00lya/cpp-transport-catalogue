#pragma once

#include <algorithm>
#include <deque>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include "domain.h"
#include <set>
#include <map>
#include <iostream>
#include "memory"

namespace transport_catalogue{

    using BusesListPointer = std::unique_ptr<std::set<std::string_view>>;

    struct HasherStopBus {
    public:
        size_t operator()(const std::string_view& name) const {
            return hasher_(name);
        }

    private:
        std::hash<std::string_view> hasher_;
    };

    struct HasherDistanceBetweenStops {
        public:
            size_t operator()(std::pair<const domain::Stop*, const domain::Stop*> key_bus) const {
                return hasher_(key_bus.second->name + key_bus.first->name);
            }

        private:
            std::hash<std::string_view> hasher_;
        };

    class TransportCatalogue {

    public:

        TransportCatalogue() = default;

        void AddStop(const domain::Stop& stop);
        const domain::Stop* FindStop(std::string_view stop_name) const;
        void AddBus(std::string bus_name, const std::vector<std::string_view> stop, bool ring_route = false);
        const domain::Bus* FindBus(std::string bus_name) const;
        domain::BusInfo GetBusInfo(const std::string_view& bus_name) const;
        const std::set<std::string> GetStopInfo(const std::string_view& stop_name) const;
        void SetDistanceBetweenStop(std::string_view stp_to, const domain::Stop* stp_from, uint32_t distance);
        bool StopExists(std::string_view stp_name) const;
        BusesListPointer GetBuses() const;
        domain::StopCoordinatesListPointer GetCoordinatesStopBuses(std::set<std::string_view> buses_names) const;

        const std::unordered_map<std::string_view, domain::Bus*, HasherStopBus>& GetBusIndexes() const;
        size_t GetAmountOfUsedStops() const;
        std::vector<std::string_view> GetUsedStopNames() const;
        uint32_t GetDistanceBetweenStops(const domain::Stop* stop_1, const domain::Stop* stop_2) const;

        // -- Методы используются для самописных юнит-тестов
        size_t NumberOfStops();
        size_t NumberOfRoutes();
        // --

    private:

        uint32_t GetDistanceBetweenStops(std::pair<const domain::Stop*, const domain::Stop*> &dist) const;
        void UpdateDistanceBetweenStops(const domain::Bus &bus);
        double GetRouteLengthForBus(const domain::Bus* bus) const;
        double GetRouteLengthGeographicalCoordinatesForBus(const domain::Bus*) const;

        std::deque<domain::Bus> buses_; // Хранилище аттрибутов всех автобусов
        std::deque<domain::Stop> stops_; // Хранилище аттрибутов всех остановок
        std::unordered_map<std::string_view, domain::Bus*, HasherStopBus> busname_to_bus_; // Список - имя автобуса : аттрибуты автобуса
        std::unordered_map<std::string_view, domain::Stop*, HasherStopBus> stopname_to_stop_; // Список - имя остановки : аттрибуты остановки
        std::unordered_map<std::string_view, std::set<std::string>, HasherStopBus> buses_stop_at_stops_; // Список - имя остановки : список имен автобусов проходящих через эту остановку
        std::unordered_map<std::pair<const domain::Stop*,const domain::Stop*>, uint32_t, HasherDistanceBetweenStops> distance_between_stops_;
        std::unordered_map<std::pair<const domain::Stop*,const domain::Stop*>, uint32_t, HasherDistanceBetweenStops> distance_between_stops_from_route_;
        std::unordered_map<std::pair<const domain::Stop*,const domain::Stop*>, double, HasherDistanceBetweenStops> distance_between_stops_geographical_coordinates_;

    };
}
