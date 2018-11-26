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
set (CMAKE_CXX_STANDARD 14)

### Download dependencies ###
hunter_add_package(GTest)
hunter_add_package(OpenSSL)
hunter_add_package(CURL)
hunter_add_package(Boost)
hunter_add_package(ICU)
hunter_add_package(OpenCV)
hunter_add_package(glog)
hunter_add_package(nlohmann_json)
find_library(FOLLY_LIBRARY folly)

### Find dependencies ###
find_package(GTest CONFIG REQUIRED) # GTest::main
find_package(OpenSSL REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)
find_package(ICU CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
include_directories("${OPENSSL_INCLUDE_DIR}")
include_directories("di/include")
include_directories("RxCpp/Rx/v2/src")
add_subdirectory(yaml-cpp)

#CMake doesn't generate compile_commands.json on Windows when the compiler is from VS
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Targets ###
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

if (NOT FOLLY_LIBRARY)
  list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/folly.cpp)
endif()

if(MSVC)
  list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/openldap.cpp)
endif()

add_executable(simple ${SRC_FILES})
target_link_libraries(simple PUBLIC GTest::main ${OPENSSL_LIBRARIES} CURL::libcurl Boost::boost ICU::uc ${OpenCV_LIBS} glog::glog nlohmann_json yaml-cpp)
if (FOLLY_LIBRARY)
  target_link_libraries(simple PUBLIC ${FOLLY_LIBRARY})
endif()

if(CMAKE_HOST_WIN32)
  add_custom_command(TARGET simple POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_CURRENT_SOURCE_DIR}/data
                        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/data
                    )

else()
  add_custom_command(TARGET simple POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E create_symlink
                        ${CMAKE_CURRENT_SOURCE_DIR}/data
                        ${CMAKE_CURRENT_BINARY_DIR}/data
                    )
endif()

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
