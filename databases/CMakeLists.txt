set(DATABASES_H market_data_db.h market_events_db.h)
set(DATABASES_SRC market_data_db.cpp market_events_db.cpp)

add_library(databases STATIC ${DATABASES_H} ${DATABASES_SRC})

target_link_libraries(databases
                      PRIVATE
                      common
                      SQLiteCpp
                      sqlite3
                      Boost::system
                      spdlog::spdlog)
