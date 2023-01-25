#pragma once

#include "ranges.h"
#include "graph.pb.h"

#include <cstdlib>
#include <vector>

namespace graph {

    using VertexId = size_t;
    using EdgeId = size_t;

    template <typename Weight>
    struct Edge {
        VertexId from;
        VertexId to;
        Weight weight;
    };

    template <typename Weight>
    class DirectedWeightedGraph {
    private:
        using IncidenceList = std::vector<EdgeId>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

    public:
        DirectedWeightedGraph() = default;
        explicit DirectedWeightedGraph(size_t vertex_count);
        DirectedWeightedGraph(std::vector<Edge<Weight>>&& edges, std::vector<IncidenceList>&& incidence_lists);
        EdgeId AddEdge(const Edge<Weight>& edge);

        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        const Edge<Weight>& GetEdge(EdgeId edge_id) const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    transport_catalogue_serialize::Graph GetSerializedGraph() const;

    private:
        std::vector<Edge<Weight>> edges_;
        std::vector<IncidenceList> incidence_lists_;
    };

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
            : incidence_lists_(vertex_count) {
    }

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(std::vector<Edge<Weight>>&& edges, std::vector<IncidenceList>&& incidence_lists)
            : edges_(edges), incidence_lists_(incidence_lists) {
    }

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_.at(edge.from).push_back(id);
        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_.at(edge_id);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
    DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }

    template<typename Weight>
    transport_catalogue_serialize::Graph DirectedWeightedGraph<Weight>::GetSerializedGraph() const {
        transport_catalogue_serialize::Graph graph_serialized;
        for (auto& edge : edges_) {
            transport_catalogue_serialize::Edge edge_serialized;
            edge_serialized.set_from_id(edge.from);
            edge_serialized.set_to_id(edge.to);
            edge_serialized.set_weight(edge.weight);
            *graph_serialized.add_edges() = std::move(edge_serialized);
        }

        for (auto& incidence_list : incidence_lists_ ) {
            transport_catalogue_serialize::IncidenceList incidence_list_serialized;
            for (auto elem : incidence_list) {
                incidence_list_serialized.add_edge_ids(elem);
            }
            *graph_serialized.add_incidence_lists() = std::move(incidence_list_serialized);
        }

        return graph_serialized;
    }
}  // namespace graph