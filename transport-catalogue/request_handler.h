#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"

class RequestHandler {
public:
    explicit RequestHandler(const transport_catalogue::TransportCatalogue& db, renderer::RenderSettings& settings)
    : db_(db), renderer_(settings, std::move(db.GetValidCoordinates()), db.GetSortedBuses(), db.GetSortedStops()) {}

    [[nodiscard]] const domain::BusInfo GetBusStat(const std::string_view& bus_name) const;
    [[nodiscard]] const domain::StopInfo GetBusesByStop(const std::string_view& stop_name) const;
    void Render(std::ostream& out) const { renderer_.Render(out); }

private:
    const transport_catalogue::TransportCatalogue& db_;
    renderer::MapRenderer renderer_;
};

json::Node MakeErrorResponse(const json::Dict* query);
json::Array MakeBusesArray(domain::StopInfo& stop_info);
json::Node MakeJSONStopResponse(domain::StopInfo& stop_info, const json::Dict* query);
json::Node MakeJSONBusResponse(domain::BusInfo& bus_info, const json::Dict* query);
json::Node MakeJSONMapResponse(const std::string& str, const json::Dict* query);