# generate_queries.cmake

# Get the list of SQL files
file(GLOB SQL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sql/*.sql")

set(HEADER_CONTENT "
#ifndef INCLUDED_CRYPTO_TRADER_DATABASES_QUERIES
#define INCLUDED_CRYPTO_TRADER_DATABASES_QUERIES

namespace crypto_trader {
namespace databases {
namespace SQL {

")

foreach(SQL_FILE ${SQL_FILES})
    # Get the filename without extension
    get_filename_component(VAR_NAME ${SQL_FILE} NAME_WE)
    
    # Read the content of the SQL file
    file(READ ${SQL_FILE} SQL_CONTENT)

    # Append to the header content
    string(APPEND HEADER_CONTENT "
inline constexpr const char* ${VAR_NAME} = R\"SQL(\n${SQL_CONTENT}
)SQL\";
")
endforeach()

string(APPEND HEADER_CONTENT "

} // namespace SQL
} // namespace databases
} // namespace crypto_trader

#endif // INCLUDED_CRYPTO_TRADER_DATABASES_QUERIES
")

# Write the header file
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/queries.h" "${HEADER_CONTENT}")
