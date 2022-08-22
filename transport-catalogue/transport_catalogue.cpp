#include "transport_catalogue.h"

namespace transport_catalogue {
    void TransportCatalogue::AddStop(Stop stop) {
        stops_.push_back(std::move(stop));
        stop_indexes_.insert({std::string_view(stops_.back().name_), &stops_.back()});
    }

    void TransportCatalogue::AddBus(RawBus raw_bus) {
        std::vector<const Stop*> stops_set;
        std::unordered_set<std::string_view> unique_stops;
        for (const std::string_view str : raw_bus.stops_) {
            stops_set.push_back(FindStop(str));
            unique_stops.insert(str);
        }
        buses_.push_back({std::move(raw_bus.name_), std::move(stops_set), unique_stops.size(), raw_bus.type_});
        buses_indexes_.insert({std::string_view(buses_.back().name_), &buses_.back()});
        for (const std::string_view str : unique_stops) {
            buses_through_the_stop_indexes_[FindStop(str)].insert(buses_.back().name_);
        }
    }

    void TransportCatalogue::AddStopsDistances(const std::pair<std::string, std::unordered_map<std::string, int>>& distances) {
        for (auto& [key , value] : distances.second) {
            distances_between_stops_.insert({{FindStop(distances.first), FindStop(key)}, value});
        }
    }

    const Bus* TransportCatalogue::FindBus(std::string_view name) const {
        if (buses_indexes_.count(name) == 0) return nullptr;
        return buses_indexes_.at(name);
    }

    const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        if (stop_indexes_.count(name) == 0) return nullptr;
        return stop_indexes_.at(name);
    }

    BusInfo TransportCatalogue::GetBusInfo(const std::string& name) const {
        BusInfo info;
        const Bus* bus = FindBus(name);
        if (bus == nullptr) return {name, 0,0,0, 0.0};
        info.name_ = bus->name_;
        info.unique_stops_ = static_cast<int>(bus->unique_stops_);
        if (bus->type_ == BusType::REVERSE) {
            info.stops_on_route_ = static_cast<int>(bus->stops_.size()) * 2 - 1;
        } else if (bus->type_ == BusType::CIRCULAR) {
            info.stops_on_route_ = static_cast<int>(bus->stops_.size()) + 1;
        }
        info.route_length_road_ = ComputeRoadDistance(*bus);
        info.curvature_ =  info.route_length_road_ / ComputeGeographicalDistance(*bus);
        return info;
    }

    StopInfo TransportCatalogue::GetStopInfo(const std::string &name) const {
        const Stop* stop = FindStop(name);
        if (stop == nullptr) return {name, empty_, true};
        if (buses_through_the_stop_indexes_.count(stop) == 0) return {name, empty_};
        StopInfo info{name, buses_through_the_stop_indexes_.at(stop)};
        return info;
    }

    double TransportCatalogue::ComputeGeographicalDistance(const Bus& bus) const {
        std::vector<geo::Coordinates> coordinates;
        std::vector<double> distances;
        for (const Stop* stop: bus.stops_) {
            coordinates.push_back({stop->latitude_, stop->longitude_});
        }
        for (int i = 0; i < coordinates.size() - 1; i++) {
            distances.push_back(geo::ComputeDistance(coordinates[i], coordinates[i + 1]));
        }
        if (bus.type_ == BusType::REVERSE) {
            return std::accumulate(distances.begin(), distances.end(), 0.0) * 2;
        } else {
            distances.push_back(geo::ComputeDistance(coordinates.back(), coordinates[0]));
            return std::accumulate(distances.begin(), distances.end(), 0.0);
        }
    }

    int TransportCatalogue::CountDistanceOnSegmentForward(const Bus& bus, size_t finish) const {
        int distance = 0;
        for (size_t i = 0; i < finish; i++) {
            std::pair<const Stop*, const Stop*> pair_of_stops(bus.stops_[i], bus.stops_[i + 1]);
            std::pair<const Stop*, const Stop*> pair_of_stops_reverse(bus.stops_[i + 1], bus.stops_[i]);
            if (distances_between_stops_.count(pair_of_stops) > 0) {
                distance += distances_between_stops_.at(pair_of_stops);
            } else if (distances_between_stops_.count(pair_of_stops_reverse) > 0) {
                distance += distances_between_stops_.at(pair_of_stops_reverse);
            }
        }
        return distance;
    }

    int TransportCatalogue::CountDistanceOnSegmentBackward(const Bus& bus, size_t start) const {
        int distance = 0;
        for (size_t i = start; i > 0; i--) {
            std::pair<const Stop*, const Stop*> pair_of_stops(bus.stops_[i], bus.stops_[i - 1]);
            std::pair<const Stop*, const Stop*> pair_of_stops_reverse(bus.stops_[i - 1], bus.stops_[i]);
            if (distances_between_stops_.count(pair_of_stops) > 0) {
                distance += distances_between_stops_.at(pair_of_stops);
            } else if (distances_between_stops_.count(pair_of_stops_reverse) > 0) {
                distance += distances_between_stops_.at(pair_of_stops_reverse);
            }
        }
        return distance;
    }

    int TransportCatalogue::ComputeRoadDistance(const Bus& bus) const {
        int distance = 0;
        if (bus.type_ == BusType::CIRCULAR) {
            distance += CountDistanceOnSegmentForward(bus, bus.stops_.size() - 1);
            std::pair<const Stop*, const Stop*> pair_of_stops(bus.stops_.back(), bus.stops_[0]);
            std::pair<const Stop*, const Stop*> pair_of_stops_reverse(bus.stops_[0], bus.stops_.back());
            if (distances_between_stops_.count(pair_of_stops) > 0) {
                distance += distances_between_stops_.at(pair_of_stops);
            } else if (distances_between_stops_.count(pair_of_stops_reverse) > 0) {
                distance += distances_between_stops_.at(pair_of_stops_reverse);
            }
        } else {
            distance += CountDistanceOnSegmentForward(bus, bus.stops_.size() - 1);
            distance += CountDistanceOnSegmentBackward(bus,  bus.stops_.size() - 1);
        }
        return distance;
    }
}
