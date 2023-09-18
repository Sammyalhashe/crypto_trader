set(COMMON_H types.h serialization.h)
set(COMMON_SRC types.cpp)

add_library(common
            STATIC
            ${COMMON_H}
            ${COMMON_SRC})

target_link_libraries(common
                      PUBLIC
                      Boost::serialization
                      )
