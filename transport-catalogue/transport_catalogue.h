#pragma once

#include "geo.h"
#include "domain.h"
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

    class TransportCatalogue {
    public:
        void AddStop(domain::Stop stop);
        void AddBus(domain::RawBus raw_bus);
        void AddStopsDistances(const std::pair<std::string, std::unordered_map<std::string, int>>& distances);
        const domain::Bus* FindBus(std::string_view name) const;
        const domain::Stop* FindStop(std::string_view name) const;
        domain::BusInfo GetBusInfo(const std::string& name) const;
        domain::StopInfo GetStopInfo(const std::string& name) const;
        std::vector<const domain::Bus*> GetSortedBuses() const;
        std::vector<const domain::Stop*> GetSortedStops() const;
        std::vector<geo::Coordinates> GetValidCoordinates() const;

    private:
        double ComputeGeographicalDistance(const domain::Bus& bus) const;
        int ComputeRoadDistance(const domain::Bus& bus) const;
        int CountDistanceOnSegmentForward(const domain::Bus& bus, size_t finish) const;
        int CountDistanceOnSegmentBackward(const domain::Bus& bus, size_t start) const;

        class Hasher {
        private:
            std::hash<const domain::Stop*> hasher_;
        public:
            size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& pointers) const {
                return static_cast<size_t>(hasher_(pointers.first) * 37 * 37 + hasher_(pointers.second));
            }
        };

        std::deque<domain::Stop> stops_;
        std::deque<domain::Bus> buses_;
        std::unordered_map<std::string_view, const domain::Stop*> stop_indexes_;
        std::unordered_map<std::string_view, const domain::Bus*> buses_indexes_;
        std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, Hasher> distances_between_stops_;
        std::unordered_map<const domain::Stop*, std::set<std::string_view>> buses_through_the_stop_indexes_;
        std::set<std::string_view> empty_ = {};
    };
}