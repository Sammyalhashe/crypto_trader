module;
#include <iostream>

export module hello;

export void say_hello() {
    std::cout << "Hello from C++20 Modules!" << std::endl;
}
