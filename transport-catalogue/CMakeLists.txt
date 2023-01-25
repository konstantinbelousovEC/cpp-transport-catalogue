cmake_minimum_required(VERSION 3.12)

project(sprint_fourteen_final_project)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto transport_router.proto)

set(TRANSPORT_CATALOGUE_FILES transport_catalogue.h transport_catalogue.cpp domain.h domain.cpp transport_catalogue.proto)
set(JSON_FILES json.h json.cpp json_builder.h json_builder.cpp json_reader.h json_reader.cpp)
set(SVG_FILES svg.h svg.cpp svg.proto)
set(ROUTER_FILES router.h graph.h transport_router.h transport_router.cpp)
set(REQUEST_HANDLER_FILES request_handler.h request_handler.cpp)
set(MAP_RENDER_FILES map_renderer.h map_renderer.cpp map_renderer.proto)
set(UTILITY_FILES geo.h geo.cpp ranges.h)
set(SERIALIZE_FILES serialization.h serialization.cpp)

add_executable(transport_catalogue main.cpp ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES}
               ${JSON_FILES} ${SVG_FILES} ${ROUTER_FILES} ${REQUEST_HANDLER_FILES} ${MAP_RENDER_FILES}
               ${UTILITY_FILES} ${SERIALIZE_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
#string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

string(REPLACE "libprotobuf.a" "libprotobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

#-DCMAKE_PREFIX_PATH=/Users/konstantinbelousov/usr_lib/protobuf/package