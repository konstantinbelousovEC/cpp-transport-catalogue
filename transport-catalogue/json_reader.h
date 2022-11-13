#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <vector>
#include <unordered_set>
#include <utility>

#include <iostream>

namespace reader {
    using namespace std::string_literals;

    struct SortedJSONQueries {
        std::unordered_set<const json::Dict*> stops_queries_;
        std::unordered_set<const json::Dict*> buses_queries_;
        std::vector<const json::Dict*> stat_requests_;
        renderer::RenderSettings settings_;
    };

    json::Document ReadJSON(std::istream& input);

    void ParseBaseRequests(const json::Node& data, SortedJSONQueries& queries);
    void ParseStatRequests(const json::Node& data, SortedJSONQueries& queries);
    void ParseRenderSettings(const json::Node& data, SortedJSONQueries& queries);

    SortedJSONQueries ParseJSON(json::Document& doc);

    void AddStopsFromJSON(transport_catalogue::TransportCatalogue& tc, std::unordered_set<const json::Dict*>& queries);
    void AddStopsDistancesFromJSON(transport_catalogue::TransportCatalogue& tc, std::unordered_set<const json::Dict*>& queries);
    void AddBusesFromJSON(transport_catalogue::TransportCatalogue& tc, std::unordered_set<const json::Dict*>& queries);

    svg::Point MakeOffset(const json::Array& values);
    svg::Color MakeColorForSVG(const json::Node& node);
    std::vector<svg::Color> MakeArrayOfColors(const json::Array& arr);

    void FillTC(transport_catalogue::TransportCatalogue& tc, std::unordered_set<const json::Dict*>& stop_q, std::unordered_set<const json::Dict*>& bus_q);

    json::Node ProcessStopQuery(RequestHandler& db, const json::Dict* query);
    json::Node ProcessBusQuery(RequestHandler& db, const json::Dict* query);
    json::Document ProcessStatRequests(RequestHandler& db, std::vector<const json::Dict*>& stat_q);
}