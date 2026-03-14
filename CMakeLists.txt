cmake_minimum_required(VERSION 3.10)

# 1. Project Name
project(HFTEngine)

# Export compile commands so editors pick up the C++17 flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2. Force C++17 Standard (Fixes the std::variant error)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 3. Add Include Directories (So you can #include "types.hpp")
include_directories(include)

# 4. Define the Executable and its source files
add_executable(hft_engine
    src/main.cpp
    src/orderbook.cpp
    src/engine.cpp
)

# 5. Enable Threading (Required for Step 4 & 5)
find_package(Threads REQUIRED)
target_link_libraries(hft_engine PRIVATE Threads::Threads)

# 6. Add Compiler Flags
target_compile_options(hft_engine PRIVATE -O3 -march=native)