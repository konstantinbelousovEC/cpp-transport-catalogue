#pragma once

#include "geo.h"
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <vector>
#include <deque>
#include <string>
#include <string_view>
#include <numeric>
#include <utility>
#include <functional>
#include <iostream>
#include <iomanip>

namespace transport_catalogue {

    enum BusType {
        REVERSE,
        CIRCULAR
    };

    struct Stop {
        Stop(std::string name, double lat, double lon)
                : name_(std::move(name)), latitude_(lat), longitude_(lon) {}

        std::string name_;
        double latitude_;
        double longitude_;
    };

    struct Bus {
        Bus(std::string name, std::vector<const Stop*> stops, size_t unique_stops, BusType type)
                : name_(std::move(name)), stops_(std::move(stops)), unique_stops_(unique_stops), type_(type) {}

        std::string name_;
        std::vector<const Stop*> stops_;
        size_t unique_stops_;
        BusType type_;
    };

    struct RawBus {
        std::string name_;
        std::vector<std::string> stops_;
        BusType type_;
    };

    struct BusInfo {
        std::string_view name_;
        int stops_on_route_;
        int unique_stops_;
        int route_length_road_;
        double curvature_;
    };

    struct StopInfo {
        std::string_view name_;
        const std::set<std::string_view>& buses_;
        bool not_exists = false;
    };

    class TransportCatalogue {
    public:
        void AddStop(Stop stop);
        void AddBus(RawBus raw_bus);
        void AddStopsDistances(const std::pair<std::string, std::unordered_map<std::string, int>>& distances);
        const Bus* FindBus(std::string_view name) const;
        const Stop* FindStop(std::string_view name) const;
        BusInfo GetBusInfo(std::string& name) const;
        StopInfo GetStopInfo(std::string& name) const;

    private:
        double ComputeGeographicalDistance(const Bus* bus) const;
        int ComputeRoadDistance(const Bus* bus) const;
        int CountDistanceOnSegmentForward(const Bus* bus, size_t finish) const;
        int CountDistanceOnSegmentBackward(const Bus* bus, size_t start) const;

        class Hasher {
        private:
            std::hash<const Stop*> hasher_;
        public:
            size_t operator()(const std::pair<const Stop*, const Stop*>& pointers) const {
                return static_cast<size_t>(hasher_(pointers.first) * 37 * 37 + hasher_(pointers.second));
            }
        };

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, const Stop*> stop_indexes_;
        std::unordered_map<std::string_view, const Bus*> buses_indexes_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distances_between_stops_;
        std::unordered_map<const Stop*, std::set<std::string_view>> buses_through_the_stop_indexes_;
        std::set<std::string_view> empty_ = {};
    };
}