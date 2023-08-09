#include "simple.h"

#include <iostream>

#include "../testlib/test.h"

namespace crypto_trader {
namespace strategies {

int simpleStrategy() {
    std::cout << "simple strategy run!" << '\n';
    test();
    return 0;
}

} // strategies
} // crypto_trader

