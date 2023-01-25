#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"
#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        transport_catalogue::TransportCatalogue transport_catalogue;
        auto doc{reader::ReadJSON(std::cin)};
        auto queries{reader::ParseMakeBaseJSON(doc)};
        reader::FillTransportCatalogue(transport_catalogue, queries.stops_queries_, queries.buses_queries_);
        transport_router::TransportRouter transport_router{queries.routing_settings_, transport_catalogue};

        std::ofstream out_file(queries.serialization_settings_.file_name_, std::ios::binary);
        serialization::EntitiesForSerialization entities{transport_catalogue, queries.render_settings_, transport_router};
        serialization::SerializeTransportDataBase(entities, out_file);

    } else if (mode == "process_requests"sv) {

        auto doc{reader::ReadJSON(std::cin)};
        auto queries{reader::ParseProcessRequestsJSON(doc)};

        std::ifstream in_file(queries.serialization_settings_.file_name_, std::ios::binary);
        serialization::DataBase db = serialization::DeserializeTransportDataBase(in_file);

        RequestHandler request_handler{db.transport_catalogue_, db.render_settings_, db.transport_router_};

        json::Document response{reader::ProcessStatRequests(request_handler, queries.stat_requests_)};
        json::Print(response, std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}