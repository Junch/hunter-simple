# Copyright (c) 2014-2016, Ruslan Baratov
# All rights reserved.

cmake_minimum_required(VERSION 3.0)

### Hunter snapshot that will be used ###
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.18.27.tar.gz"
    SHA1 "dcc5416d24b536856c780edc1ba7cc3087487eeb"
    LOCAL
)

project(HunterSimple)

add_compile_options(-std=c++11)

### Download dependencies ###
hunter_add_package(GTest)
hunter_add_package(OpenSSL)
hunter_add_package(CURL)

### Find dependencies ###
find_package(GTest CONFIG REQUIRED) # GTest::main
find_package(OpenSSL REQUIRED)
find_package(CURL CONFIG REQUIRED)
include_directories("${OPENSSL_INCLUDE_DIR}")

### Targets ###
add_executable(simple simple.cpp curl_test.cpp string_format.cpp)
target_link_libraries(simple PUBLIC GTest::main ${OPENSSL_LIBRARIES} CURL::libcurl)

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
