#include "request_handler.h"

const std::optional<domain::BusInfo> RequestHandler::GetBusStat(const std::string_view &bus_name) const {
    return transport_catalogue_.GetBusInfo(std::string (bus_name));
}

const std::optional<domain::StopInfo> RequestHandler::GetBusesByStop(const std::string_view &stop_name) const {
    return transport_catalogue_.GetStopInfo(std::string (stop_name));
}

json::Node MakeErrorResponse(const json::Dict* query) {
    json::Dict res;
    std::string s = "not found";
    res.insert({"request_id", query->at("id")});
    res.insert({"error_message", json::Node(s)});
    return {res};
}

json::Array MakeBusesArray(domain::StopInfo& stop_info) {
    json::Array buses;
    if (stop_info.buses_ == nullptr) return buses;
    buses.reserve(stop_info.buses_->size());
    for (auto bus : *stop_info.buses_) {
        buses.emplace_back(std::string(bus));
    }
    return buses;
}

json::Node MakeJSONStopResponse(domain::StopInfo& stop_info, const json::Dict* query) {
    json::Array buses = MakeBusesArray(stop_info);
    json::Dict res;
    res.insert({"buses", buses});
    res.insert({"request_id", query->at("id")});
    return {res};
}

json::Node MakeJSONBusResponse(domain::BusInfo& bus_info, const json::Dict* query) {
    json::Dict res;
    res.insert({"curvature", json::Node(bus_info.curvature_)});
    res.insert({"request_id", query->at("id")});
    res.insert({"route_length", json::Node(bus_info.route_length_road_)});
    res.insert({"stop_count", json::Node(bus_info.stops_on_route_)});
    res.insert({"unique_stop_count", json::Node(bus_info.unique_stops_)});
    return {res};
}

json::Node MakeJSONMapResponse(const std::string& str, const json::Dict* query) {
    json::Dict res;
    res.insert({"map", json::Node(str)});
    res.insert({"request_id", query->at("id")});
    return res;
}