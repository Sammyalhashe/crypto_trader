set(DATABASES_H market_data_db.h)
set(DATABASES_SRC market_data_db.cpp)

add_library(databases STATIC ${DATABASES_H} ${DATABASES_SRC})

target_link_libraries(databases
                      PRIVATE
                      common
                      protocols
                      Boost::system
                      spdlog::spdlog)
