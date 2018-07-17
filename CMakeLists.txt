# Copyright (c) 2014-2016, Ruslan Baratov
# All rights reserved.

cmake_minimum_required(VERSION 3.0)

### Hunter snapshot that will be used ###
include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.22.31.tar.gz"
  SHA1 "80b45f6182151d235bded7c3d3a818e84851521f"
  LOCAL
)

project(HunterSimple)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  # minimize the size or the resulting EXE
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

add_compile_options(-std=c++14)

### Download dependencies ###
hunter_add_package(GTest)
hunter_add_package(OpenSSL)
hunter_add_package(CURL)
hunter_add_package(Boost)
hunter_add_package(ICU)

### Find dependencies ###
find_package(GTest CONFIG REQUIRED) # GTest::main
find_package(OpenSSL REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)
find_package(ICU CONFIG REQUIRED)
include_directories("${OPENSSL_INCLUDE_DIR}")
include_directories("di/include")

#CMake doesn't generate compile_commands.json on Windows when the compiler is from VS
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Targets ###
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(simple ${SRC_FILES})
target_link_libraries(simple PUBLIC GTest::main ${OPENSSL_LIBRARIES} CURL::libcurl Boost::boost ICU::uc)

if(MSVC)
  # Suppress link warnings LNK4099
  set_target_properties(simple PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

### Testing ###
enable_testing()

if(IOS)
  # Extra cmake modules needed for iOS testing:
  # * https://github.com/ruslo/sugar/tree/master/cmake/core#sugar_add_ios_gtest
  add_test(
      NAME SimpleTest COMMAND "${CMAKE_COMMAND}" -E echo "iOS testing ignored"
  )
elseif(ANDROID)
  # Building .apk, testing: https://github.com/forexample/android-cmake
  add_test(
      NAME SimpleTest
      COMMAND "${CMAKE_COMMAND}" -E echo "Android testing ignored"
  )
else()
  add_test(NAME SimpleTest COMMAND simple)
endif()
