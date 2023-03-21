#include "serialization.h"

namespace serialization {
    using Graph = graph::DirectedWeightedGraph<double>;
    using VertexId = size_t;
    using PairsOfVerticesMap = std::unordered_map<std::string_view, std::pair<VertexId, VertexId>>;

    transport_catalogue_serialize::TransportCatalogue SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue);
    transport_catalogue_serialize::RenderSettings SerializeRenderSettings(const renderer::RenderSettings& render_settings);
    transport_catalogue_serialize::TransportRouter SerializeTransportRouter(const transport_router::TransportRouter& transport_router);

    transport_catalogue::TransportCatalogue DeserializeTransportCatalogue(const transport_catalogue_serialize::TransportCatalogue& transport_catalogue_serialized);
    renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& render_settings_serialized);
    transport_router::RoutingSettings DeserializeRoutingSettings(const transport_catalogue_serialize::RoutingSettings& router_settings_serialized);
    std::unique_ptr<Graph> DeserializeGraph(const transport_catalogue_serialize::Graph& graph_serialized);
    std::unique_ptr<graph::Router<double>> DeserializeRouter(
            const transport_catalogue_serialize::Router& router_serialized,
            const Graph& graph
            );
    PairsOfVerticesMap DeserializePairsOfVertices(
            const transport_catalogue_serialize::TransportRouter& transport_router_serialized,
            const transport_catalogue::TransportCatalogue& transport_catalogue
            );
    std::vector<transport_router::EdgeDescription> DeserializeEdgeDescriptions(
            const transport_catalogue_serialize::TransportRouter& transport_router_serialized,
            const transport_catalogue::TransportCatalogue& transport_catalogue
            );

    transport_catalogue_serialize::Color ChangeColorFormatToProtoMessage(const svg::Color& color);
    svg::Color ChangeColorFormatToSVGColor(const transport_catalogue_serialize::Color& color_serialized);

    template<typename InputIterator>
    uint32_t CalcStopId(InputIterator first, InputIterator last, std::string_view name);

    void SerializeTransportDataBase(EntitiesForSerialization entities, std::ostream& out) {
        transport_catalogue_serialize::DataBase db_serialized;

        transport_catalogue_serialize::TransportCatalogue transport_catalogue_serialized =
                SerializeTransportCatalogue(entities.transport_catalogue_);
        transport_catalogue_serialize::RenderSettings render_settings_serialized =
                SerializeRenderSettings(entities.render_settings_);
        transport_catalogue_serialize::TransportRouter transport_router_serialized =
                SerializeTransportRouter(entities.transport_router_);

        *db_serialized.mutable_transport_catalogue() = std::move(transport_catalogue_serialized);
        *db_serialized.mutable_render_settings() = std::move(render_settings_serialized);
        *db_serialized.mutable_transport_router() = std::move(transport_router_serialized);

        db_serialized.SerializePartialToOstream(&out);
    }

    DataBase DeserializeTransportDataBase(std::istream& in) {
        transport_catalogue_serialize::DataBase db_serialized;
        if (!db_serialized.ParseFromIstream(&in)) {
            throw std::runtime_error("error of parsing serialized file from istream");
        }

        std::unique_ptr<Graph> graph = DeserializeGraph(db_serialized.transport_router().graph());
        std::unique_ptr<graph::Router<double>> router = DeserializeRouter(db_serialized.transport_router().router(), *graph);

        DataBase db{ DeserializeTransportCatalogue(db_serialized.transport_catalogue()),
                     DeserializeRenderSettings(db_serialized.render_settings()),
                    { DeserializeRoutingSettings(db_serialized.transport_router().routing_settings()),
                         db.transport_catalogue_,
                         std::move(graph),
                         std::move(router),
                         DeserializePairsOfVertices(db_serialized.transport_router(), db.transport_catalogue_),
                         DeserializeEdgeDescriptions(db_serialized.transport_router(), db.transport_catalogue_)
                     }};
        return db;
    }

    transport_catalogue_serialize::TransportCatalogue SerializeTransportCatalogue(const transport_catalogue::TransportCatalogue& transport_catalogue) {
        transport_catalogue_serialize::TransportCatalogue transport_catalogue_serialized;

        auto& stops = transport_catalogue.GetStops();
        for (int i = 0; i < stops.size(); i++) {
            transport_catalogue_serialize::Stop stop_serialized;
            stop_serialized.set_id(i);
            stop_serialized.set_name(stops[i].name_);
            stop_serialized.set_latitude(stops[i].latitude_);
            stop_serialized.set_longitude(stops[i].longitude_);
            *transport_catalogue_serialized.add_stops() = std::move(stop_serialized);
        }

        auto& buses = transport_catalogue.GetBuses();
        for (const auto & bus : buses) {
            transport_catalogue_serialize::Bus bus_serialized;
            bus_serialized.set_name(bus.name_);
            bus_serialized.set_unique_stops(bus.unique_stops_);
            bus_serialized.set_bus_type(bus.type_ == domain::BusType::REVERSE ?
                                        transport_catalogue_serialize::BusType::REVERSE :
                                        transport_catalogue_serialize::BusType::CIRCULAR);
            for (auto stop : bus.stops_) {
                uint32_t stop_id = CalcStopId(stops.cbegin(), stops.cend(), stop->name_);
                bus_serialized.add_stops(stop_id);
            }
            *transport_catalogue_serialized.add_buses() = std::move(bus_serialized);
        }

        auto& distances = transport_catalogue.GetDistancess();
        for (auto [stops_pair, distance_between] : distances) {
            transport_catalogue_serialize::Distance distance;
            uint32_t stop_id_first = CalcStopId(stops.cbegin(), stops.cend(), stops_pair.first->name_);
            uint32_t stop_id_second = CalcStopId(stops.cbegin(), stops.cend(), stops_pair.second->name_);
            distance.set_stop_id_1(stop_id_first);
            distance.set_stop_id_2(stop_id_second);
            distance.set_distance(distance_between);
            *transport_catalogue_serialized.add_distances() = std::move(distance);
        }

        return transport_catalogue_serialized;
    }

    transport_catalogue_serialize::RenderSettings SerializeRenderSettings(const renderer::RenderSettings& render_settings) {
        transport_catalogue_serialize::RenderSettings render_settings_serialized;
        render_settings_serialized.set_width(render_settings.width_);
        render_settings_serialized.set_height(render_settings.height_);
        render_settings_serialized.set_padding(render_settings.padding_);
        render_settings_serialized.set_line_width(render_settings.line_width_);
        render_settings_serialized.set_stop_radius(render_settings.stop_radius_);
        render_settings_serialized.set_bus_label_font_size(render_settings.bus_label_font_size_);

        transport_catalogue_serialize::Point bus_label_offset_serialized;
        bus_label_offset_serialized.set_x(render_settings.bus_label_offset_.x);
        bus_label_offset_serialized.set_y(render_settings.bus_label_offset_.y);
        *render_settings_serialized.mutable_bus_label_offset() = std::move(bus_label_offset_serialized);

        render_settings_serialized.set_stop_label_font_size(render_settings.stop_label_font_size_);

        transport_catalogue_serialize::Point stop_label_offset_serialized;
        stop_label_offset_serialized.set_x(render_settings.stop_label_offset_.x);
        stop_label_offset_serialized.set_y(render_settings.stop_label_offset_.y);
        *render_settings_serialized.mutable_stop_label_offset() = std::move(stop_label_offset_serialized);

        transport_catalogue_serialize::Color underlayer_color_serialized = ChangeColorFormatToProtoMessage(render_settings.underlayer_color_);
        *render_settings_serialized.mutable_underlayer_color() = std::move(underlayer_color_serialized);

        render_settings_serialized.set_underlayer_width(render_settings.underlayer_width_);

        for (auto& color : render_settings.color_palette_) {
            transport_catalogue_serialize::Color color_serialized = ChangeColorFormatToProtoMessage(color);
            *render_settings_serialized.add_color_palette() = std::move(color_serialized);
        }

        return render_settings_serialized;
    }

    transport_catalogue_serialize::TransportRouter SerializeTransportRouter(const transport_router::TransportRouter& transport_router) {
        transport_catalogue_serialize::TransportRouter transport_router_serialized;

        transport_catalogue_serialize::RoutingSettings router_settings_serialized;
        router_settings_serialized.set_bus_wait_time(transport_router.GetRoutingSettings().bus_wait_time_);
        router_settings_serialized.set_bus_velocity(transport_router.GetRoutingSettings().bus_velocity_);
        *transport_router_serialized.mutable_routing_settings() = std::move(router_settings_serialized);

        transport_catalogue_serialize::Graph graph_serialized = transport_router.GetGraph()->GetSerializedGraph();
        *transport_router_serialized.mutable_graph() = std::move(graph_serialized);
        transport_catalogue_serialize::Router router_serialized = transport_router.GetRouter()->GetSerializedRouter();
        *transport_router_serialized.mutable_router() = std::move(router_serialized);


        for (auto pair : transport_router.GetPairsOfVertices()) {
            transport_catalogue_serialize::PairOfVertices pair_of_vertices_serialized;
            pair_of_vertices_serialized.set_name(std::string (pair.first));
            pair_of_vertices_serialized.set_id_from(pair.second.first);
            pair_of_vertices_serialized.set_id_to(pair.second.second);
            *transport_router_serialized.add_pairs_of_vertices() = std::move(pair_of_vertices_serialized);
        }

        for (auto& edge_description : transport_router.GetEdgeDescriptions()) {
            transport_catalogue_serialize::EdgeDescription edge_description_serialized;
            edge_description_serialized.set_type(edge_description.type_ == transport_router::EdgeType::WAIT ?
                                                 transport_catalogue_serialize::EdgeType::WAIT :
                                                 transport_catalogue_serialize::EdgeType::BUS);
            edge_description_serialized.set_edge_name(std::string(edge_description.edge_name_));
            edge_description_serialized.set_time(edge_description.time_);
            if (edge_description.span_count_.has_value()) {
                transport_catalogue_serialize::SpanCount span_count_serialized;
                span_count_serialized.set_has_value(true);
                span_count_serialized.set_span_count(edge_description.span_count_.value());
                *edge_description_serialized.mutable_span_count() = std::move(span_count_serialized);
            } else {
                transport_catalogue_serialize::SpanCount span_count_serialized;
                span_count_serialized.set_has_value(false);
                *edge_description_serialized.mutable_span_count() = std::move(span_count_serialized);
            }
            *transport_router_serialized.add_edges_description() = std::move(edge_description_serialized);
        }

        return transport_router_serialized;
    }

    transport_catalogue::TransportCatalogue DeserializeTransportCatalogue(const transport_catalogue_serialize::TransportCatalogue& transport_catalogue_serialized) {
        transport_catalogue::TransportCatalogue transport_catalogue;

        for (auto& stop_serialized : transport_catalogue_serialized.stops()) {
            transport_catalogue.AddStop({stop_serialized.name(), stop_serialized.latitude(), stop_serialized.longitude()});
        }

        auto& stops = transport_catalogue.GetStops();
        for (auto& distance_serialized : transport_catalogue_serialized.distances()) {
            std::string_view stop_name_1 = stops[distance_serialized.stop_id_1()].name_;
            std::string_view stop_name_2 = stops[distance_serialized.stop_id_2()].name_;
            transport_catalogue.AddStopsDistancesByPair(stop_name_1, stop_name_2, distance_serialized.distance());
        }

        for (auto& bus_serialized : transport_catalogue_serialized.buses()) {
            std::string name = bus_serialized.name();
            std::vector<std::string> stops_of_bus;
            for (auto stop_id : bus_serialized.stops()) {
                std::string stop_name = stops[stop_id].name_;
                stops_of_bus.push_back(stop_name);
            }
            domain::BusType type = bus_serialized.bus_type() == transport_catalogue_serialize::BusType::REVERSE ?
                                                                domain::BusType::REVERSE :
                                                                domain::BusType::CIRCULAR;
            transport_catalogue.AddBus({std::move(name), std::move(stops_of_bus), type});
        }

        return transport_catalogue;
    }

    renderer::RenderSettings DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& render_settings_serialized) {
        renderer::RenderSettings render_settings;
        render_settings.width_ = render_settings_serialized.width();
        render_settings.height_ = render_settings_serialized.height();
        render_settings.padding_ = render_settings_serialized.padding();
        render_settings.line_width_ = render_settings_serialized.line_width();
        render_settings.stop_radius_ = render_settings_serialized.stop_radius();
        render_settings.bus_label_font_size_ = render_settings_serialized.bus_label_font_size();
        render_settings.bus_label_offset_.x = render_settings_serialized.bus_label_offset().x();
        render_settings.bus_label_offset_.y = render_settings_serialized.bus_label_offset().y();
        render_settings.stop_label_font_size_ = render_settings_serialized.stop_label_font_size();
        render_settings.stop_label_offset_.x = render_settings_serialized.stop_label_offset().x();
        render_settings.stop_label_offset_.y = render_settings_serialized.stop_label_offset().y();
        render_settings.underlayer_color_ = ChangeColorFormatToSVGColor(render_settings_serialized.underlayer_color());
        render_settings.underlayer_width_ = render_settings_serialized.underlayer_width();
        for (auto& color_serialized : render_settings_serialized.color_palette()) {
            render_settings.color_palette_.push_back(ChangeColorFormatToSVGColor(color_serialized));
        }
        return render_settings;
    }

    std::unique_ptr<Graph> DeserializeGraph(const transport_catalogue_serialize::Graph& graph_serialized) {
        std::vector<graph::Edge<double>> edges;
        edges.reserve(graph_serialized.edges_size());
        for (auto& edge_serialized : graph_serialized.edges()) {
            graph::Edge<double> edge{edge_serialized.from_id(), edge_serialized.to_id(), edge_serialized.weight()};
            edges.push_back(edge);
        }

        std::vector<std::vector<VertexId>> incidence_lists;
        incidence_lists.reserve(graph_serialized.incidence_lists_size());
        for (auto& incidence_list_serialized : graph_serialized.incidence_lists()) {
            std::vector<VertexId> incidence_list;
            incidence_list.reserve(incidence_list_serialized.edge_ids_size());
            for (auto id : incidence_list_serialized.edge_ids()) {
                incidence_list.push_back(id);
            }
            incidence_lists.push_back(std::move(incidence_list));
        }

        return std::make_unique<Graph>(std::move(edges), std::move(incidence_lists));
    }

    std::unique_ptr<graph::Router<double>> DeserializeRouter(const transport_catalogue_serialize::Router& router_serialized, const Graph& graph) {
        std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>> array_routes_internal_data;
        for (auto& routes_of_internal_data_serialized : router_serialized.array_of_routes_internal_data()) {
            std::vector<std::optional<graph::Router<double>::RouteInternalData>> routes_of_internal_data;
            routes_of_internal_data.reserve(routes_of_internal_data_serialized.routes_internal_data_size());
            for (auto& route_internal_data_serialized : routes_of_internal_data_serialized.routes_internal_data()) {
                std::optional<graph::Router<double>::RouteInternalData> route_internal_data;
                if (route_internal_data_serialized.has_value()) {
                    graph::Router<double>::RouteInternalData routeInternalData;
                    routeInternalData.weight = route_internal_data_serialized.weight();
                    if (route_internal_data_serialized.prev_edge().has_value()) {
                        routeInternalData.prev_edge = route_internal_data_serialized.prev_edge().prev_edge();
                    }
                    route_internal_data = routeInternalData;
                }
                routes_of_internal_data.push_back(route_internal_data);
            }
            array_routes_internal_data.push_back(std::move(routes_of_internal_data));
        }
        return std::make_unique<graph::Router<double>>(graph::Router<double>{graph, std::move(array_routes_internal_data)});
    }

    transport_router::RoutingSettings DeserializeRoutingSettings(const transport_catalogue_serialize::RoutingSettings& router_settings_serialized) {
        transport_router::RoutingSettings routing_settings;
        routing_settings.bus_wait_time_ = router_settings_serialized.bus_wait_time();
        routing_settings.bus_velocity_ = router_settings_serialized.bus_velocity();
        return routing_settings;
    }

    PairsOfVerticesMap DeserializePairsOfVertices(const transport_catalogue_serialize::TransportRouter& transport_router_serialized,
                                                  const transport_catalogue::TransportCatalogue& transport_catalogue) {
        PairsOfVerticesMap pairs_of_vertices_for_each_stop;
        const auto& stop_names = transport_catalogue.GetUsedStopNames();
        for (auto& pair : transport_router_serialized.pairs_of_vertices()) {
            auto stop_name_iter = std::find(stop_names.begin(), stop_names.end(), pair.name());
            std::pair<std::string_view, std::pair<VertexId, VertexId>> pair_of_vertices{*stop_name_iter, {pair.id_from(), pair.id_to()}};
            pairs_of_vertices_for_each_stop.insert(std::move(pair_of_vertices));
        }
        return pairs_of_vertices_for_each_stop;
    }

    std::vector<transport_router::EdgeDescription> DeserializeEdgeDescriptions(const transport_catalogue_serialize::TransportRouter& transport_router_serialized,
                                                                               const transport_catalogue::TransportCatalogue& transport_catalogue) {
        std::vector<transport_router::EdgeDescription> edges_description;
        edges_description.reserve(transport_router_serialized.edges_description_size());
        const auto& stop_names = transport_catalogue.GetUsedStopNames();
        const auto& bus_names = transport_catalogue.GetBuses();
        for (auto& edge_description_serialized : transport_router_serialized.edges_description()) {
            transport_router::EdgeDescription edge_description;
            edge_description.type_ = edge_description_serialized.type() == transport_catalogue_serialize::EdgeType::BUS ?
                                                                           transport_router::EdgeType::BUS :
                                                                           transport_router::EdgeType::WAIT;
            edge_description.time_ = edge_description_serialized.time();
            if (edge_description.type_ == transport_router::EdgeType::WAIT) {
                edge_description.edge_name_ = *(std::find(stop_names.begin(), stop_names.end(), edge_description_serialized.edge_name()));
            } else {
                std::string_view str_to_find = edge_description_serialized.edge_name();
                edge_description.edge_name_ = (std::find_if(bus_names.begin(), bus_names.end(), [str_to_find](const domain::Bus& bus){
                    return bus.name_ == str_to_find;
                }))->name_;
            }
            if (edge_description_serialized.span_count().has_value()) {
                edge_description.span_count_ = edge_description_serialized.span_count().span_count();
            } else {
                edge_description.span_count_ = std::nullopt;
            }
            edges_description.push_back(edge_description);
        }
        return edges_description;
    }

    template<typename InputIterator>
    uint32_t CalcStopId(InputIterator first, InputIterator last, std::string_view name) {
        auto stop_iterator = std::find_if(first, last, [&name](const domain::Stop stop_elem){
            return stop_elem.name_ == name;
        });
        return std::distance(first, stop_iterator);
    }

    transport_catalogue_serialize::Color ChangeColorFormatToProtoMessage(const svg::Color& color) {
        transport_catalogue_serialize::Color color_serialized;
        if (std::holds_alternative<std::monostate>(color)) {
            color_serialized.set_non_color(true);
        } else if (std::holds_alternative<svg::Rgb>(color)) {
            svg::Rgb rgb = std::get<svg::Rgb>(color);
            color_serialized.mutable_rgb()->set_red(rgb.red);
            color_serialized.mutable_rgb()->set_green(rgb.green);
            color_serialized.mutable_rgb()->set_blue(rgb.blue);
        } else if (std::holds_alternative<svg::Rgba>(color)) {
            svg::Rgba rgba = std::get<svg::Rgba>(color);
            color_serialized.mutable_rgba()->set_red(rgba.red);
            color_serialized.mutable_rgba()->set_green(rgba.green);
            color_serialized.mutable_rgba()->set_blue(rgba.blue);
            color_serialized.mutable_rgba()->set_alpha(rgba.opacity);
        } else if (std::holds_alternative<std::string>(color)) {
            color_serialized.set_str_color(std::get<std::string>(color));
        }
        return color_serialized;
    }

    svg::Color ChangeColorFormatToSVGColor(const transport_catalogue_serialize::Color& color_serialized) {
        svg::Color color;
        if (color_serialized.has_rgb()) {
            svg::Rgb rgb;
            rgb.red = color_serialized.rgb().red();
            rgb.green = color_serialized.rgb().green();
            rgb.blue = color_serialized.rgb().blue();
            color = rgb;
        } else if (color_serialized.has_rgba()) {
            svg::Rgba rgba;
            rgba.red = color_serialized.rgba().red();
            rgba.green = color_serialized.rgba().green();
            rgba.blue = color_serialized.rgba().blue();
            rgba.opacity = color_serialized.rgba().alpha();
            color = rgba;
        } else if (color_serialized.has_str_color()) {
            color = color_serialized.str_color();
        }
        return color;
    }
}