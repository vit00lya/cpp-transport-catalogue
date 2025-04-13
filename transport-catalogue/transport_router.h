#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include <iostream>
#include <memory>
#include "algorithm"

namespace transport_router {
    constexpr static double METERS_PER_KM = 1000.0;
    constexpr static double MIN_PER_HOUR = 60.0;

    struct RoutingSettings {
        double bus_wait_time_ = 0.0;
        double bus_velocity_ = 0.0;
    };

    enum class EdgeType {
        WAIT,
        BUS
    };

    struct EdgeDescription {
        EdgeType type_;
        std::string_view edge_name_;
        double time_ = 0.0;
        std::optional<int> span_count_ = 0;
    };

    using Router = graph::Router<double>;
    using Graph = graph::DirectedWeightedGraph<double>;
    using EdgeDescriptions = std::vector<EdgeDescription>;

   class TransportRouter {
    public:
       TransportRouter() = default;
       TransportRouter(RoutingSettings settings, std::unique_ptr<transport_catalogue::TransportCatalogue> &&transport_catalogue);

       const RoutingSettings& GetRoutingSettings() const &;
       std::optional<EdgeDescriptions> BuildRoute(std::string_view stop_from, std::string_view stop_to) const;

        template<typename InputIterator>
        void AddBusEdgesToGraph(InputIterator first, InputIterator last, std::string_view bus_name) {
            for (; std::distance(first, last) != 1; first++) {
                graph::VertexId from_id = GetPairsOfVertices().at((*first)->name).second;
                const domain::Stop* from_stop = *first;
                double time = 0.0;
                InputIterator next_after_first = first;
                for (std::advance(next_after_first, 1); next_after_first != last; next_after_first++) {
                    graph::VertexId to_id = GetPairsOfVertices().at((*next_after_first)->name).first;
                    time += transport_catalogue_->GetDistanceBetweenStops(from_stop,*next_after_first) /
                            METERS_PER_KM / GetRoutingSettings().bus_velocity_ * MIN_PER_HOUR;
                    //std::cerr  << "bus " << bus_name << " from stop " << from_stop->name_ << " to stop " << (*next_after_first)->name_ << " from_id " << from_id << " to_id " << to_id << std::endl;
                    GetGraph()->AddEdge({from_id, to_id, time});
                    from_stop = *next_after_first;
                    GetEdgeDescription().push_back({
                                                          EdgeType::BUS,
                                                          bus_name,
                                                          time,
                                                          std::distance(first, next_after_first)
                                                  });
                }
            }
        }

    private:

        std::unique_ptr<Graph>& GetGraph() &;
        const std::unique_ptr<Graph>& GetGraph() const &;
        const std::unique_ptr<Router>& GetRouter() const &;
        EdgeDescriptions& GetEdgeDescription() &;
        const std::unordered_map<std::string_view, std::pair<size_t, size_t>>& GetPairsOfVertices() const &;
        const EdgeDescriptions& GetEdgeDescriptions() const &;

        RoutingSettings routing_settings_;
        std::unique_ptr<transport_catalogue::TransportCatalogue> transport_catalogue_;
        std::unique_ptr<Graph> graph_;
        std::unique_ptr<Router> router_;
        std::unordered_map<std::string_view, std::pair<size_t, size_t>> pairs_of_vertices_for_each_stop_;
        EdgeDescriptions edges_descriptions_;

        void FillGraph();
        void AddWaitEdgesToGraph();
    };
}
