#include "transport_router.h"

namespace transport_router {
    TransportRouter::TransportRouter(RoutingSettings settings, const transport_catalogue::TransportCatalogue &transport_catalogue)
            : routing_settings_(settings),
              transport_catalogue_(transport_catalogue),
              graph_(std::make_unique<Graph>(transport_catalogue_.GetAmountOfUsedStops() * 2)),
              router_(nullptr)
    {
        FillGraph();
        router_ = std::make_unique<Router>(*graph_);
    }

    TransportRouter::TransportRouter(RoutingSettings routing_settings,
                                     const transport_catalogue::TransportCatalogue& transport_catalogue,
                                     std::unique_ptr<Graph>&& graph,
                                     std::unique_ptr<Router>&& router,
                                     std::unordered_map<std::string_view, std::pair<size_t, size_t>>&& pairs_of_vertices_for_each_stop,
                                     EdgeDescriptions&& edges_descriptions)
            : routing_settings_(routing_settings),
              transport_catalogue_(transport_catalogue),
              graph_(std::move(graph)),
              router_(std::move(router)),
              pairs_of_vertices_for_each_stop_(std::move(pairs_of_vertices_for_each_stop)),
              edges_descriptions_(std::move(edges_descriptions)) {}

    template<typename InputIterator>
    void AddBusEdgesToGraph(TransportRouter& transport_router, InputIterator first, InputIterator last, std::string_view bus_name) {
        for (; std::distance(first, last) != 1; first++) {
            graph::VertexId from_id = transport_router.GetPairsOfVertices().at((*first)->name_).second;
            const domain::Stop* from_stop = *first;
            double time = 0.0;
            InputIterator next_after_first = first;
            for (std::advance(next_after_first, 1); next_after_first != last; next_after_first++) {
                graph::VertexId to_id = transport_router.GetPairsOfVertices().at((*next_after_first)->name_).first;
                time += transport_router.GetTransportCatalogue().GetDistancesBetweenStops(from_stop,*next_after_first) /
                        METERS_PER_KM / transport_router.GetRoutingSettings().bus_velocity_ * MIN_PER_HOUR;

                transport_router.GetGraph()->AddEdge({from_id, to_id, time});
                from_stop = *next_after_first;
                transport_router.GetEdgeDescription().push_back({
                                                      EdgeType::BUS,
                                                      bus_name,
                                                      time,
                                                      std::distance(first, next_after_first)
                                              });
            }
        }
    }

    const RoutingSettings& TransportRouter::GetRoutingSettings() const & {
        return routing_settings_;
    }
    const transport_catalogue::TransportCatalogue& TransportRouter::GetTransportCatalogue() const & {
        return transport_catalogue_;
    }
    std::unique_ptr<Graph>& TransportRouter::GetGraph() & {
        return graph_;
    }
    const std::unique_ptr<Graph> &TransportRouter::GetGraph() const & {
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

        for (graph::VertexId id : route.value().edges) {
            result.push_back(edges_descriptions_[id]);
        }
        return result;
    }

    void TransportRouter::FillGraph() {
        AddWaitEdgesToGraph();
        for (auto [name, bus_ptr] : transport_catalogue_.GetBusIndexes()) {
            AddBusEdgesToGraph(*this, bus_ptr->stops_.begin(), bus_ptr->stops_.end(), name);
            if (bus_ptr->type_ == domain::BusType::REVERSE) {
                AddBusEdgesToGraph(*this, bus_ptr->stops_.crbegin(), bus_ptr->stops_.crend(), name);
            }
        }
    }

    void TransportRouter::AddWaitEdgesToGraph() {
        graph::VertexId from_id = 0;
        graph::VertexId to_id = 1;
        for (std::string_view name: transport_catalogue_.GetUsedStopNames()) {
            graph_->AddEdge({from_id, to_id, routing_settings_.bus_wait_time_});
            pairs_of_vertices_for_each_stop_.insert({name, {from_id, to_id}});
            edges_descriptions_.push_back({
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