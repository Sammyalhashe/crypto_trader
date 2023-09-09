set(COMMON_H types.h)
set(COMMON_SRC types.cpp)

add_library(common
            STATIC
            ${COMMON_H}
            ${COMMON_SRC})

target_link_libraries(common
                      PRIVATE
                      )
