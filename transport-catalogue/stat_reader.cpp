#include "stat_reader.h"

namespace transport_catalogue {

    namespace output {

        void ShowBusInfo(const BusInfo& info);
        void ShowStopInfo(const StopInfo& info);

        void ReadAndShowOutputQueries(const TransportCatalogue& tc, std::istream& input) {
            std::string line;
            getline(input, line);
            int queries_count = std::stoi(line);
            for (int i = 0; i < queries_count; i++) {
                getline(input, line);
                if (line.substr(0, 3) == "Bus") {
                    std::string name = line.substr(4);
                    ShowBusInfo(tc.GetBusInfo(name));
                } else if (line.substr(0, 4) == "Stop") {
                    std::string name = line.substr(5);
                    ShowStopInfo(tc.GetStopInfo(name));
                }
            }
        }

        void ShowStopInfo(const StopInfo& info) {
            if (info.not_exists) {
                std::cout << "Stop "
                          << info.name_
                          << ": not found"
                          << std::endl;
                return;
            }
            if (info.buses_.empty()) {
                std::cout << "Stop "
                          << info.name_
                          << ": no buses"
                          << std::endl;
                return;
            }
            std::cout << "Stop "
                      << info.name_
                      << ": buses ";
            bool first = true;
            for (const std::string_view bus: info.buses_) {
                if (first) {
                    std::cout << bus;
                    first = false;
                    continue;
                }
                std::cout << " " << bus;
            }
            std::cout << std::endl;
        }

        void ShowBusInfo(const BusInfo& info) {
            if (info.stops_on_route_ == 0) {
                std::cout << "Bus "
                          << info.name_
                          << ": not found"
                          << std::endl;
                return;
            }
            std::cout << "Bus "
                      << info.name_
                      << ": "
                      << info.stops_on_route_
                      << " stops on route, "
                      << info.unique_stops_
                      << " unique stops, "
                      << info.route_length_road_
                      << " route length, "
                      << info.curvature_
                      << " curvature"
                      << std::endl;
        }
    }
}
