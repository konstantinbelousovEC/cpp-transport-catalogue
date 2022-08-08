#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <vector>

namespace transport_catalogue {

    namespace output {

        void ReadAndShowOutputQueries(TransportCatalogue& tc, std::istream& input);

        namespace detail {

            void ShowBusInfo(const BusInfo& info);
            void ShowStopInfo(const StopInfo& info);
        }
    }
}