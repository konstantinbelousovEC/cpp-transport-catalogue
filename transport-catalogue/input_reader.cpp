#include "input_reader.h"

namespace transport_catalogue {

    namespace input {

        void ReadInputQueries(TransportCatalogue& tc, std::istream& input) {
            std::string line;
            getline(input, line);
            int queries_count = std::stoi(line);
            SortedQueries queries;
            for (int i = 0; i < queries_count; i++) {
                getline(input, line);
                if (line.substr(0, 4) == "Stop") {
                    queries.stops_queries.insert(line);
                } else if (line.substr(0,3) == "Bus") {
                    queries.buses_queries.insert(line);
                }
            }
            ProcessInputQueries(tc, queries);
        }

        void ProcessInputQueries(TransportCatalogue& tc, SortedQueries& queries) {
            for (const std::string& query : queries.stops_queries) {
                tc.AddStop(detail::ParseStopQueryForBaseInfo(query));
            }
            for (const std::string& query : queries.stops_queries) {
                if (std::count(query.begin(), query.end(), ',') == 1) continue;
                tc.AddStopsDistances(detail::ParseStopQueryForDistance(query));
            }
            for (const std::string& query : queries.buses_queries) {
                tc.AddBus(detail::ParseBusQuery(query));
            }
        }

        namespace detail {

            Stop ParseStopQueryForBaseInfo(const std::string& query) {
                return {query.substr(5, query.find(':') - 5),
                        std::stod(query.substr(query.find(':') + 1, query.find(',') - query.find(':') + 1)),
                        std::stod((query.substr(query.find(',') + 1)).substr(0, query.find(',')))};
            }

            std::pair<std::string, std::unordered_map<std::string, int>> ParseStopQueryForDistance(const std::string& query) {
                std::string name = query.substr(5, query.find(':') - 5);
                std::string tmp = query.substr(query.find(',') + 1);
                std::string sequence = tmp.substr(tmp.find(',') + 2);
                std::vector<std::string> partials = ParseSequenceOnSegments(sequence, ", ");
                std::pair<std::string, std::unordered_map<std::string, int>> result;
                result.first = std::move(name);
                for (std::string& elem : partials) {
                    result.second.insert(ParseDistanceCharacteristic(elem));
                }
                return result;
            }

            RawBus ParseBusQuery(const std::string& query) {
                std::string bus_name = query.substr(4, query.find(':') - 4);
                const std::string separator = query.find('-') == std::string::npos ? " > " : " - ";
                std::string sequence = query.substr(query.find(':') + 2);
                std::vector<std::string> stops = ParseSequenceOnSegments(sequence, separator);
                BusType type = separator == " - "  ? BusType::REVERSE : BusType::CIRCULAR;
                if (type == BusType::CIRCULAR) stops.pop_back();
                return {bus_name, stops, type};
            }

            std::vector<std::string> ParseSequenceOnSegments(const std::string& sequence, const std::string& separator) {
                std::vector<std::string> result;
                bool last = false;
                for (size_t i = 0, j = sequence.find(separator) - i; !last; ) {
                    if (j > sequence.size()) last = true;
                    std::string str = (sequence.substr(i, j));
                    result.push_back(std::move(str));
                    i = i + j + separator.size();
                    j = sequence.find(separator, i) - i;
                }
                return result;
            }

            std::pair<std::string, int> ParseDistanceCharacteristic(const std::string& query) {
                std::pair<std::string, int> result;
                result.first = query.substr(query.find("to ") + 3);
                result.second = std::stoi(query.substr(0,query.find('m')));
                return result;
            }
        }
    }
}