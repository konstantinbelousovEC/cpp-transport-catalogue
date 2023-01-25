#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"
#include <vector>
#include <unordered_set>
#include <utility>

namespace reader {
    using namespace std::string_literals;

    struct SortedJSONQueries {
        std::unordered_set<const json::Dict*> stops_queries_;
        std::unordered_set<const json::Dict*> buses_queries_;
        transport_router::RoutingSettings routing_settings_;
        std::vector<const json::Dict*> stat_requests_;
        renderer::RenderSettings render_settings_;
    };

    // draft
    struct MakeBaseRequests {
        std::unordered_set<const json::Dict*> stops_queries_;
        std::unordered_set<const json::Dict*> buses_queries_;
        transport_router::RoutingSettings routing_settings_;
        renderer::RenderSettings render_settings_;
        serialization::SerializationSettings serialization_settings_;
    };
    struct ProcessRequests {
        std::vector<const json::Dict*> stat_requests_;
        serialization::SerializationSettings serialization_settings_;
    };
    //

    json::Document ReadJSON(std::istream& input);

    void ParseBaseRequests(const json::Node& data, MakeBaseRequests& queries);
    void ParseStatRequests(const json::Node& data, ProcessRequests& queries);
    void ParseRenderSettings(const json::Node& data, MakeBaseRequests& queries);
    void ParseRoutingSettings(const json::Node& data, MakeBaseRequests& queries);

    MakeBaseRequests ParseMakeBaseJSON(json::Document& doc);
    ProcessRequests ParseProcessRequestsJSON(json::Document& doc);

    void AddStopsFromJSON(transport_catalogue::TransportCatalogue& transport_catalogue, std::unordered_set<const json::Dict*>& queries);
    void AddStopsDistancesFromJSON(transport_catalogue::TransportCatalogue& transport_catalogue, std::unordered_set<const json::Dict*>& queries);
    void AddBusesFromJSON(transport_catalogue::TransportCatalogue& transport_catalogue, std::unordered_set<const json::Dict*>& queries);

    svg::Point MakeOffset(const json::Array& values);
    svg::Color MakeColorForSVG(const json::Node& node);
    std::vector<svg::Color> MakeArrayOfColors(const json::Array& array);

    void FillTransportCatalogue(transport_catalogue::TransportCatalogue& transport_catalogue,
                                std::unordered_set<const json::Dict*>& stop_queries,
                                std::unordered_set<const json::Dict*>& bus_queries);

    json::Node ProcessStopQuery(RequestHandler& request_handler, const json::Dict* query);
    json::Node ProcessBusQuery(RequestHandler& request_handler, const json::Dict* query);
    json::Node ProcessRouteQuery(RequestHandler& request_handler, const json::Dict* query);
    json::Document ProcessStatRequests(RequestHandler& request_handler, std::vector<const json::Dict*>& stat_queries);
}