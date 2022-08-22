#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <string>
#include <vector>

namespace transport_catalogue {

    namespace output {

        void ReadAndShowOutputQueries(const TransportCatalogue& tc, std::istream& input);
    }
}
