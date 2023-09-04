#include "simple.h"

#include <spdlog/spdlog.h>

#include "../testlib/test.h"

namespace crypto_trader {
namespace strategies {

int simpleStrategy() {
    spdlog::info("simple strategy run!");
    test();
    return 0;
}

} // strategies
} // crypto_trader

