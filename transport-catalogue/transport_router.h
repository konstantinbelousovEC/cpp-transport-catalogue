#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include <memory>

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
        TransportRouter(RoutingSettings settings, const transport_catalogue::TransportCatalogue& transport_catalogue);

        TransportRouter(RoutingSettings routing_settings,
                        const transport_catalogue::TransportCatalogue& transport_catalogue,
                        std::unique_ptr<Graph>&& graph,
                        std::unique_ptr<Router>&& router,
                        std::unordered_map<std::string_view, std::pair<size_t, size_t>>&& pairs_of_vertices_for_each_stop,
                        EdgeDescriptions&& edges_descriptions);

        const RoutingSettings& GetRoutingSettings() const &;
        const transport_catalogue::TransportCatalogue& GetTransportCatalogue() const &;
        std::unique_ptr<Graph>& GetGraph() &;
        const std::unique_ptr<Graph>& GetGraph() const &;
        const std::unique_ptr<Router>& GetRouter() const &;
        EdgeDescriptions& GetEdgeDescription() &;
        const std::unordered_map<std::string_view, std::pair<size_t, size_t>>& GetPairsOfVertices() const &;
        const EdgeDescriptions& GetEdgeDescriptions() const &;

        std::optional<EdgeDescriptions> BuildRoute(std::string_view stop_from, std::string_view stop_to) const;

    private:
        RoutingSettings routing_settings_;
        const transport_catalogue::TransportCatalogue& transport_catalogue_;
        std::unique_ptr<Graph> graph_;
        std::unique_ptr<Router> router_;
        std::unordered_map<std::string_view, std::pair<size_t, size_t>> pairs_of_vertices_for_each_stop_;
        EdgeDescriptions edges_descriptions_;

        void FillGraph();
        void AddWaitEdgesToGraph();
    };
}
