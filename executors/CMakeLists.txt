set(EXECUTORS_H paper_trading_executor.h real_trading_executor.h)
set(EXECUTORS_SRC paper_trading_executor.cpp real_trading_executor.cpp)

add_library(executors STATIC ${EXECUTORS_H} ${EXECUTORS_SRC})

target_link_libraries(executors
                      PRIVATE
                      common
                      protocols
                      nlohmann_json::nlohmann_json
                      spdlog::spdlog
                      )
