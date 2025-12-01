#include "simple.h"

#include <spdlog/spdlog.h>

namespace crypto_trader {
namespace strategies {

int simpleStrategy()
{
    SPDLOG_INFO("simple strategy run!");
    return 0;
}

} // namespace strategies
} // namespace crypto_trader
