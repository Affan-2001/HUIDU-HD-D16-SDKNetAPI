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

# Add TinyXML2 source files
set(TINYXML2_SOURCES 
    ${CMAKE_SOURCE_DIR}/src/HCatNet/Tool/tinyxml2.cpp
)
set(TINYXML2_HEADERS 
    ${CMAKE_SOURCE_DIR}/src/HCatNet/Tool/tinyxml2.h
)

# Create a library target for TinyXML2
add_library(tinyxml2 STATIC ${TINYXML2_SOURCES} ${TINYXML2_HEADERS})

# Set include directory for TinyXML2
target_include_directories(tinyxml2 PUBLIC 
    ${CMAKE_SOURCE_DIR}/src/HCatNet/Tool
)

# Executable
add_executable(SDKNetApp ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Include directories
target_include_directories(SDKNetApp PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/ArduinoJson
    ${CMAKE_SOURCE_DIR}/src/HCatNet/Tool  # For TinyXML2
)

# Link libraries
target_link_libraries(SDKNetApp 
    HCatNet
    tinyxml2  # Link our TinyXML2 library
    ws2_32
)

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