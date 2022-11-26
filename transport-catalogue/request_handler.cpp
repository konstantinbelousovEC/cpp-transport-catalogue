#include "request_handler.h"
#include "json_builder.h"

const std::optional<domain::BusInfo> RequestHandler::GetBusStat(const std::string_view &bus_name) const {
    return transport_catalogue_.GetBusInfo(std::string (bus_name));
}

const std::optional<domain::StopInfo> RequestHandler::GetBusesByStop(const std::string_view &stop_name) const {
    return transport_catalogue_.GetStopInfo(std::string (stop_name));
}

json::Array MakeBusesArray(domain::StopInfo& stop_info) {
    if (stop_info.buses_ == nullptr) return {};
    json::Array buses;
    buses.reserve(stop_info.buses_->size());
    for (auto bus : *stop_info.buses_) {
        buses.emplace_back(std::string(bus));
    }
    return buses;
}

json::Node MakeErrorResponse(const json::Dict* query) {
    return json::Builder{}.StartDict()
                              .Key("request_id").Value(query->at("id").GetValue())
                              .Key("error_message").Value("not found")
                          .EndDict()
                      .Build();
}

json::Node MakeJSONStopResponse(domain::StopInfo& stop_info, const json::Dict* query) {
    json::Array buses = MakeBusesArray(stop_info);
    return json::Builder{}.StartDict()
                                .Key("buses").Value(buses)
                                .Key("request_id").Value(query->at("id").GetValue())
                           .EndDict()
                       .Build();
}

json::Node MakeJSONBusResponse(domain::BusInfo& bus_info, const json::Dict* query) {
    return json::Builder{}.StartDict()
                                .Key("curvature").Value(bus_info.curvature_)
                                .Key("request_id").Value(query->at("id").GetValue())
                                .Key("route_length").Value(bus_info.route_length_road_)
                                .Key("stop_count").Value(bus_info.stops_on_route_)
                                .Key("unique_stop_count").Value(bus_info.unique_stops_)
                           .EndDict()
                       .Build();
}

json::Node MakeJSONMapResponse(const std::string& str, const json::Dict* query) {
    json::Dict res;
    res.insert({"map", json::Node(str)});
    res.insert({"request_id", query->at("id")});
    return json::Builder{}.StartDict()
                                .Key("map").Value(str)
                                .Key("request_id").Value(query->at("id").GetValue())
                          .EndDict()
                      .Build();
}