#include "json_reader.h"
#include "request_handler.h"
#include <iostream>

int main() {
    transport_catalogue::TransportCatalogue tc;
    auto doc{reader::ReadJSON(std::cin)};
    auto queries{reader::ParseJSON(doc)};
    reader::FillTC(tc, queries.stops_queries_, queries.buses_queries_);
    RequestHandler db{tc, queries.settings_};
    json::Document response{reader::ProcessStatRequests(db, queries.stat_requests_)};
    json::Print(response, std::cout);
}
