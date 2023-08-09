set(TEST_H test.h)
set(TEST_SRC test.cpp)

add_library(testlib STATIC ${TEST_H} ${TEST_SRC})
