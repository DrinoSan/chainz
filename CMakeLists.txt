# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Define the project name
project(TzehChain LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenSSL package (required for SHA-256 hashing)
find_package(OpenSSL REQUIRED)

# Include directories for OpenSSL and nlohmann/json
include_directories(${OPENSSL_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/json)

# Add the executable target
add_executable(blockchain main.cpp Data.cpp Blockchain.cpp)

# Link OpenSSL libraries to the executable
target_link_libraries(blockchain PRIVATE OpenSSL::SSL OpenSSL::Crypto)

