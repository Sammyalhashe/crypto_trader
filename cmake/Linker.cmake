
# Linker configurations for mold
if(UNIX AND NOT APPLE)
    set(CMAKE_LINKER_TYPE MOLD)
elseif(APPLE)
    set(CMAKE_LINKER_TYPE LLD)
endif()
