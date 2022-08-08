#pragma once

#include "transport_catalogue.h"
#include <functional>
#include <string>
#include <string_view>
#include <iostream>
#include <unordered_set>

namespace transport_catalogue {

    namespace input {

        struct SortedQueries {
            std::unordered_set<std::string> stops_queries;
            std::unordered_set<std::string> buses_queries;
        };

        void ReadInputQueries(TransportCatalogue& tc, std::istream& input);
        void ProcessInputQueries(TransportCatalogue& tc, SortedQueries& queries);

        namespace detail {

            Stop ParseStopQueryForBaseInfo(const std::string& query);
            std::pair<std::string, std::unordered_map<std::string, int>> ParseStopQueryForDistance(const std::string& query);
            RawBus ParseBusQuery(const std::string& query);
            std::vector<std::string> ParseSequenceOnSegments(const std::string& sequence, const std::string& separator);
            std::pair<std::string, int> ParseDistanceCharacteristic(const std::string& query);
        }
    }
}

