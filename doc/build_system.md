# Build System Architecture

This document details the architectural choices and configurations of the CMake-based build system used in `crypto_trader`.

## Core Philosophy

The build system is designed for **iteration speed** and **reproducibility**. We prioritize fast incremental builds and linking times, leveraging modern C++ toolchain features where possible.

## Key Features

### 1. High-Performance Linking (Mold & LLD)
We utilize CMake 3.29+'s `CMAKE_LINKER_TYPE` feature to automatically select the fastest available linker:
-   **Linux:** Defaults to [mold](https://github.com/rui314/mold), a modern high-speed linker.
-   **macOS:** Defaults to [lld](https://lld.llvm.org/), the LLVM linker, which significantly outperforms the default Apple `ld`.

### 2. Unity Builds (Jumbo Builds)
**Enabled via:** `set(CMAKE_UNITY_BUILD ON)`

We use Unity Builds instead of Precompiled Headers (PCH).
-   **How it works:** CMake merges multiple `.cpp` source files into a single compilation unit (e.g., `unity_0.cxx`) before invoking the compiler.
-   **Why:**
    -   **Speed:** Drastically reduces the overhead of parsing header files (like `<boost/asio.hpp>` or `<nlohmann/json.hpp>`) multiple times.
    -   **Stability:** Avoids brittle configuration mismatches often seen with PCH when using compiler wrappers (e.g., Nix hardening flags vs. CMake dependency scanners).
    -   **Linking:** Reduces the total number of object files, speeding up the link step.

### 3. C++20 Modules Support
The project supports C++20 Modules (`.cppm` files).
-   **Infrastructure:** Custom CMake logic (see `cmake/CxxModules.cmake`) handles dynamic include path discovery for module scanners.
-   **Generator Requirement:** Due to module support, the build system requires the **Ninja** generator (`-G Ninja`), as `Unix Makefiles` does not fully support C++20 module dependency scanning.

### 4. Interprocedural Optimization (IPO/LTO)
**Disabled via:** `set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)`

IPO is explicitly disabled for development builds to prioritize compile/link speed over runtime micro-optimizations.

## Environment: Nix
The project is designed to run within a Nix shell (`nix develop`), which provides a hermetic toolchain (Clang, CMake, Ninja, Boost, etc.).
-   **Note:** The build system is robust against Nix compiler wrappers, which is a primary reason for preferring Unity Builds over PCH (which can "bake in" conflicting wrapper flags).
