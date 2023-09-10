#include "simple.h"

#include <spdlog/spdlog.h>

namespace crypto_trader {
namespace strategies {

int simpleStrategy() {
    spdlog::info("simple strategy run!");
    return 0;
}

} // strategies
} // crypto_trader

