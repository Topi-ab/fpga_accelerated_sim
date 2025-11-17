cmake_minimum_required(VERSION 3.16)
project(FPGA_Interface_Project LANGUAGES CXX)

# use modern C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# Create a header-only library target (fpga_iface)
# This exposes the include/ directory to users
# -------------------------------------------------------------------
add_library(fpga_iface INTERFACE)

target_include_directories(fpga_iface
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -------------------------------------------------------------------
# Application
# -------------------------------------------------------------------
add_executable(fpga_app
    src/main.cpp
)

# Link the header-only library to the app
target_link_libraries(fpga_app PRIVATE fpga_iface)
