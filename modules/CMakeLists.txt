add_library(hello_module)
target_sources(hello_module
  PUBLIC
    FILE_SET CXX_MODULES FILES
      hello.cppm
)
