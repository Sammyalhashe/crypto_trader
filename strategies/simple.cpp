#include "simple.h"

#include <iostream>

#include "../test/test.h"

namespace crypto_trader {
namespace strategies {

void simpleStrategy() {
    std::cout << "simple strategy run!" << '\n';
    test();
}

} // strategies
} // crypto_trader

