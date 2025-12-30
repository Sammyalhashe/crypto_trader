set(DATABASES_H market_data_db.h market_events_db.h bind_types.h)
set(DATABASES_SRC market_data_db.cpp market_events_db.cpp)

# Generate queries.h
include(../cmake/generate_queries.cmake)
add_custom_target(generate_queries ALL
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/generate_queries.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating queries.h from SQL files"
    VERBATIM
)

add_library(databases STATIC ${DATABASES_H} ${DATABASES_SRC} ${CMAKE_CURRENT_BINARY_DIR}/generated/queries.h)
set_target_properties(databases PROPERTIES
    PRIVATE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/generated/queries.h)

target_include_directories(databases PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

add_dependencies(databases generate_queries)

target_link_libraries(databases
                      PRIVATE
                      common
                      SQLiteCpp
                      sqlite3
                      Boost::system
                      spdlog::spdlog)
