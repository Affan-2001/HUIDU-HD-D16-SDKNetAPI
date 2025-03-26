cmake_minimum_required(VERSION 3.12)

# Use g++ from MSYS2 if found
set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/g++.exe")

project(SDKNetApi C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Debug)

link_directories(${CMAKE_SOURCE_DIR}/lib64)
# Executable
add_executable(SDKNetApp ${CMAKE_SOURCE_DIR}/src/main.cpp)

   
target_link_libraries(SDKNetApp HCatNet)


# Include ArduinoJson
target_include_directories(SDKNetApp PRIVATE ${CMAKE_SOURCE_DIR}/include)
# Include the external library's root directly
target_include_directories(SDKNetApp PRIVATE ${CMAKE_SOURCE_DIR}/include/ArduinoJson)

# Link libraries
target_link_libraries(SDKNetApp ws2_32)

# Definitions
target_compile_definitions(SDKNetApp PRIVATE 
    _DEBUG 
    UNICODE 
    _UNICODE
)

# Compiler flags
target_compile_options(SDKNetApp PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic 
    -g
)
