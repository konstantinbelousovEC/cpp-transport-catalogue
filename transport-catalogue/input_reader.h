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
    }
}
