#include "json_reader.h"
#include <stdexcept>

namespace reader {
    void ParseBaseRequests(const json::Node& data, SortedJSONQueries& queries) {
        auto& requests = data.AsArray();
        for (auto& node : requests) {
            auto& req_json = node.AsMap();
            if (req_json.at("type").AsString() == "Stop") {
                queries.stops_queries_.insert(&req_json);
            } else {
                queries.buses_queries_.insert(&req_json);
            }
        }
    }

    void ParseStatRequests(const json::Node& data, SortedJSONQueries& queries) {
        auto& requests = data.AsArray();
        for (auto& node : requests) {
            queries.stat_requests_.push_back(&node.AsMap());
        }
    }

    svg::Point MakeOffset(const json::Array& values) {
        if (values.size() != 2) throw std::logic_error("Incorrect format of points");
        return {values[0].AsDouble(), values[1].AsDouble()};
    }

    svg::Color MakeColorForSVG(const json::Node& node) {
        if (node.IsString()) {
            return {node.AsString()};
        } else if (node.IsArray() && node.AsArray().size() == 3) {
            const auto& arr = node.AsArray();
            return svg::Rgb{static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()), static_cast<uint8_t>(arr[2].AsInt())};
        } else if (node.IsArray() && node.AsArray().size() == 4) {
            const auto& arr = node.AsArray();
            return svg::Rgba{static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()), static_cast<uint8_t>(arr[2].AsInt()), arr[3].AsDouble()};
        } else {
            return {std::monostate()};
        }
    }

    std::vector<svg::Color> MakeArrayOfColors(const json::Array& arr) {
        std::vector<svg::Color> res;
        for (auto& elem : arr) {
            res.push_back(MakeColorForSVG(elem));
        }
        return res;
    }

    void ParseRenderSettings(const json::Node& data, SortedJSONQueries& queries) {
        auto& settings = data.AsMap();
        renderer::RenderSettings set;
        set.width_ = settings.at("width").AsDouble();
        set.height_ = settings.at("height").AsDouble();
        set.padding_ = settings.at("padding").AsDouble();
        set.line_width_ = settings.at("line_width").AsDouble();
        set.stop_radius_ = settings.at("stop_radius").AsDouble();
        set.bus_label_font_size_ = settings.at("bus_label_font_size").AsInt();
        set.bus_label_offset_ = MakeOffset(settings.at("bus_label_offset").AsArray());
        set.stop_label_font_size_ = settings.at("stop_label_font_size").AsInt();
        set.stop_label_offset_ = MakeOffset(settings.at("stop_label_offset").AsArray());
        set.underlayer_color_ = MakeColorForSVG(settings.at("underlayer_color"));
        set.underlayer_width_ = settings.at("underlayer_width").AsDouble();
        set.color_palette_ = MakeArrayOfColors(settings.at("color_palette").AsArray());
        queries.settings_ = std::move(set);
    }

    json::Document ReadJSON(std::istream& input) {
        return json::Load(input);
    }

    SortedJSONQueries ParseJSON(json::Document& doc) {
        SortedJSONQueries queries;
        for (auto& [query, data] : doc.GetRoot().AsMap()) {
            if (query == "base_requests"s) {
                ParseBaseRequests(data, queries);
            } else if (query == "stat_requests"s) {
                ParseStatRequests(data, queries);
            } else if (query == "render_settings") {
                ParseRenderSettings(data, queries);
            }
        }
        return queries;
    }

    void AddStopsFromJSON(transport_catalogue::TransportCatalogue& tc, std::unordered_set<const json::Dict*>& queries) {
        for (const json::Dict* query : queries) {
            const std::string stop_name = query->at("name").AsString();
            tc.AddStop({
                               stop_name,
                               query->at("latitude").AsDouble(),
                               query->at("longitude").AsDouble(),
                       });
        }
    }

    void AddStopsDistancesFromJSON(transport_catalogue::TransportCatalogue& tc, std::unordered_set<const json::Dict*>& queries) {
        for (const json::Dict* query : queries) {
            std::unordered_map<std::string, int> distances;
            for (auto& [key, value] : query->at("road_distances").AsMap()) {
                distances.insert({key, value.AsInt()});
            }
            tc.AddStopsDistances({query->at("name").AsString(), distances});
        }
    }

    void AddBusesFromJSON(transport_catalogue::TransportCatalogue& tc, std::unordered_set<const json::Dict*>& queries) {
        for (const json::Dict* query : queries) {
            std::vector<std::string> stops;
            stops.reserve(query->at("stops").AsArray().size());
            for (auto& stop_node : query->at("stops").AsArray()) {
                stops.push_back(stop_node.AsString());
            }
            tc.AddBus({
                              query->at("name").AsString(),
                              stops,
                              query->at("is_roundtrip").AsBool() ? domain::BusType::CIRCULAR : domain::BusType::REVERSE
                      });
        }
    }

    void FillTC(transport_catalogue::TransportCatalogue& tc,
                std::unordered_set<const json::Dict*>& stop_q,
                std::unordered_set<const json::Dict*>& bus_q)
    {
        AddStopsFromJSON(tc, stop_q);
        AddStopsDistancesFromJSON(tc, stop_q);
        AddBusesFromJSON(tc, bus_q);
    }

    json::Node ProcessStopQuery(RequestHandler& db, const json::Dict* query) {
        auto stop_info = db.GetBusesByStop(query->at("name").AsString());
        if (stop_info.not_exists) {
            return MakeErrorResponse(query);
        } else {
            return MakeJSONStopResponse(stop_info, query);
        }
    }

    json::Node ProcessBusQuery(RequestHandler& db, const json::Dict* query) {
        auto bus_info = db.GetBusStat(query->at("name").AsString());
        if (bus_info.stops_on_route_ == 0) {
            return MakeErrorResponse(query);
        } else {
            return MakeJSONBusResponse(bus_info, query);
        }
    }

    json::Node ProcessMapQuery(RequestHandler& db, const json::Dict* query) {
        std::ostringstream str;
        db.Render(str);
        return MakeJSONMapResponse(str.str(), query);
    }

    json::Document ProcessStatRequests(RequestHandler& db, std::vector<const json::Dict*>& stat_q) {
        json::Array arr;
        for (auto query : stat_q) {
            if (query->at("type").AsString() == "Stop") {
                arr.push_back(ProcessStopQuery(db, query));
            } else if (query->at("type").AsString() == "Bus") {
                arr.push_back(ProcessBusQuery(db, query));
            } else if (query->at("type").AsString() == "Map") {
                arr.push_back(ProcessMapQuery(db, query));
            }
        }
        json::Document doc(arr);
        return doc;
    }
}