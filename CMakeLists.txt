###cmake_minimum_required(VERSION 3.16)
###project(FPGA_Interface_Project LANGUAGES CXX)
###
#### use modern C++
###set(CMAKE_CXX_STANDARD 23)
###set(CMAKE_CXX_STANDARD_REQUIRED ON)
###
#### -------------------------------------------------------------------
#### Create a header-only library target (fpga_iface)
#### This exposes the include/ directory to users
#### -------------------------------------------------------------------
###add_library(fpga_iface INTERFACE)
###
###target_include_directories(fpga_iface
###    INTERFACE
###        ${CMAKE_CURRENT_SOURCE_DIR}/include
###)
###
#### -------------------------------------------------------------------
#### Application
#### -------------------------------------------------------------------
###add_executable(fpga_app
###    src/main.cpp
###)
###
#### Link the header-only library to the app
###target_link_libraries(fpga_app PRIVATE fpga_iface)
###

cmake_minimum_required(VERSION 3.16)
project(FPGA_Interface_Project LANGUAGES CXX)

# ------------------------------------------------------------
# Modern C++
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------
# ARM64 SPEED OPTIMIZED BUILD
# (applies only when building for aarch64 or cross compiling)
# ------------------------------------------------------------
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_CROSSCOMPILING)
    message(STATUS "Configuring ARM64 speed-optimized build")

    add_compile_options(
        -O3
        -finline-functions
        -finline-functions-called-once
        -finline-limit=100000
        -funroll-loops
        -fno-plt
        -DNDEBUG
    )

    # Uncomment and tune for specific ARM CPU (recommended!)
    # add_compile_options(-mcpu=neoverse-n1)
    # add_compile_options(-mcpu=cortex-a72)
    # add_compile_options(-mcpu=apple-m1)
endif()

# ------------------------------------------------------------
# Header-only library target (fpga_iface)
# ------------------------------------------------------------
add_library(fpga_iface INTERFACE)

target_include_directories(fpga_iface
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------
# Application executable
# ------------------------------------------------------------
add_executable(fpga_app
    src/main.cpp
)

target_link_libraries(fpga_app PRIVATE fpga_iface)
