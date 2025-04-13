#include "transport_router.h"

namespace transport_router {
    TransportRouter::TransportRouter(RoutingSettings settings, std::unique_ptr<transport_catalogue::TransportCatalogue> &&transport_catalogue)
            : routing_settings_(settings),
              transport_catalogue_(std::move(transport_catalogue)),
              graph_(std::make_unique<Graph>(transport_catalogue_->GetAmountOfUsedStops() * 2)),
              router_(nullptr)
    {
        FillGraph();
        router_ = std::make_unique<Router>(*graph_);
    }

    const RoutingSettings& TransportRouter::GetRoutingSettings() const & {
        return routing_settings_;
    }

    std::unique_ptr<Graph>& TransportRouter::GetGraph() & {
        return graph_;
    }

    const std::unique_ptr<Router>& TransportRouter::GetRouter() const & {
        return router_;
    }
    EdgeDescriptions& TransportRouter::GetEdgeDescription() & {
        return edges_descriptions_;
    }
    const std::unordered_map<std::string_view, std::pair<size_t, size_t>>& TransportRouter::GetPairsOfVertices() const & {
        return pairs_of_vertices_for_each_stop_;
    }
    const EdgeDescriptions &TransportRouter::GetEdgeDescriptions() const &{
        return edges_descriptions_;
    }

    std::optional<EdgeDescriptions> TransportRouter::BuildRoute(std::string_view stop_from, std::string_view stop_to) const {
        EdgeDescriptions result;

        if (stop_from == stop_to) return result;
        if (pairs_of_vertices_for_each_stop_.count(stop_from) == 0
            || pairs_of_vertices_for_each_stop_.count(stop_to) == 0) return std::nullopt;

        graph::VertexId from_id = pairs_of_vertices_for_each_stop_.at(stop_from).first;
        graph::VertexId to = pairs_of_vertices_for_each_stop_.at(stop_to).first;
        std::optional<Router::RouteInfo> route = router_->BuildRoute(from_id, to);

        if (!route.has_value()) return std::nullopt;

        const auto& ed = GetEdgeDescriptions();

        for (graph::VertexId id : route.value().edges) {
            result.push_back(ed[id]);
        }
        return result;
    }

void TransportRouter::FillGraph()
{
    AddWaitEdgesToGraph();
    for (const auto [name, bus_ptr] : transport_catalogue_->GetBusIndexes())
    {
        AddBusEdgesToGraph(bus_ptr->stop.begin(), bus_ptr->stop.end(), name);
        if (!bus_ptr->ring_route)
        {
            AddBusEdgesToGraph(bus_ptr->stop.crbegin(),bus_ptr->stop.crend(), name);
        }
    }
}


    void TransportRouter::AddWaitEdgesToGraph() {
        graph::VertexId from_id = 0;
        graph::VertexId to_id = 1;
        for (std::string_view name: transport_catalogue_->GetUsedStopNames()) {
            graph_->AddEdge({from_id, to_id, routing_settings_.bus_wait_time_});
            pairs_of_vertices_for_each_stop_.insert({name, {from_id, to_id}});
           // std::cerr <<  " name " << name << " form_id " << from_id << " to_id " << to_id << std::endl;
            GetEdgeDescription().push_back({
                EdgeType::WAIT,
                name,
                routing_settings_.bus_wait_time_,
                std::nullopt
            });
            from_id += 2;
            to_id += 2;
        }
    }
}
