#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "svg.pb.h"
#include "graph.pb.h"
#include "transport_router.pb.h"

#include <iostream>

namespace serialization {
    struct SerializationSettings {
        std::string file_name_;
    };

    struct EntitiesForSerialization {
        const transport_catalogue::TransportCatalogue& transport_catalogue_;
        const renderer::RenderSettings& render_settings_;
        const transport_router::TransportRouter& transport_router_;
    };

    struct DataBase {
        transport_catalogue::TransportCatalogue transport_catalogue_;
        renderer::RenderSettings render_settings_;
        transport_router::TransportRouter transport_router_;
    };

    void SerializeTransportDataBase(EntitiesForSerialization entities, std::ostream& out);
    DataBase DeserializeTransportDataBase(std::istream& in);
}