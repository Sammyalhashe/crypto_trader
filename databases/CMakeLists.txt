set(DATABASES_H market_data_db.h market_events_db.h)
set(DATABASES_SRC market_data_db.cpp market_events_db.cpp)

add_library(databases STATIC ${DATABASES_H} ${DATABASES_SRC})

target_precompile_headers(databases
                          PRIVATE
                          <SQLiteCpp/SQLiteCpp.h>
                          <boost/lockfree/spsc_queue.hpp>
                          <spdlog/spdlog.h>)

target_link_libraries(databases
                      PRIVATE
                      common
                      SQLiteCpp
                      sqlite3
                      Boost::system
                      spdlog::spdlog)
