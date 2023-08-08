#include "strategies/simple.h"
#include "test/test.h"

#include <cstdlib>
#include <cstring>
#include <stdio.h>

#include <fmt/printf.h>

int main() {
    // TODO: Remove test use of dep
    fmt::printf("test from fmt %d\n", 42);

    using namespace crypto_trader;
    strategies::simpleStrategy();

    return 0;
}
