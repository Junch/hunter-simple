# Copyright (c) 2014-2016, Ruslan Baratov
# All rights reserved.

cmake_minimum_required(VERSION 3.12)

### Hunter snapshot that will be used ###
include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.23.205.tar.gz"
  SHA1 "eba80d3028c856c6bedb5d8d9627b1b3bba19634"
)

include(FetchContent)
include(cmake/fmt.cmake)

project(HunterSimple)

if(MSVC)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING /D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  # minimize the size or the resulting EXE
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
  # VS2017 (C4996) std::tr1 namespace and TR1-only machinery deprecation
  # add_compile_options(/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

# https://github.com/boostorg/system/issues/26
add_compile_options(-DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

### Download dependencies ###
hunter_add_package(GTest)
hunter_add_package(OpenSSL)
hunter_add_package(CURL)
hunter_add_package(Boost COMPONENTS system filesystem context date_time program_options regex random atomic)
hunter_add_package(ICU)
hunter_add_package(OpenCV)
hunter_add_package(glog)
hunter_add_package(nlohmann_json)
hunter_add_package(double-conversion)
hunter_add_package(Libevent)
hunter_add_package(gflags)
hunter_add_package(sqlite3)

### Find dependencies ###
find_package(GTest CONFIG REQUIRED) # GTest::main
find_package(OpenSSL REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system filesystem context date_time program_options regex random atomic)
find_package(ICU CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(double-conversion CONFIG REQUIRED)
find_package(Libevent CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)

option(YAML_CPP_BUILD_TESTS "Enable testing" OFF)
add_subdirectory(yaml-cpp)

#CMake doesn't generate compile_commands.json on Windows when the compiler is from VS
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRC_FILES ${PROJECT_SOURCE_DIR}/src/avatar.cpp
  ${PROJECT_SOURCE_DIR}/src/avatar_legacy.cpp
  ${PROJECT_SOURCE_DIR}/src/boost_system.cpp
  ${PROJECT_SOURCE_DIR}/src/curl_multi_thread.cpp
  ${PROJECT_SOURCE_DIR}/src/curl_test.cpp
  ${PROJECT_SOURCE_DIR}/src/di_test.cpp
  ${PROJECT_SOURCE_DIR}/src/emplace.cpp
  ${PROJECT_SOURCE_DIR}/src/fmt_test.cpp
  ${PROJECT_SOURCE_DIR}/src/gflags_demo.cpp
  ${PROJECT_SOURCE_DIR}/src/glog_demo.cpp
  ${PROJECT_SOURCE_DIR}/src/gtest_demo.cpp
  ${PROJECT_SOURCE_DIR}/src/json_test.cpp
  ${PROJECT_SOURCE_DIR}/src/macro_test.cpp
  ${PROJECT_SOURCE_DIR}/src/main.cpp
  ${PROJECT_SOURCE_DIR}/src/memory.cpp
  ${PROJECT_SOURCE_DIR}/src/opencv.cpp
  ${PROJECT_SOURCE_DIR}/src/predicate.cpp
  ${PROJECT_SOURCE_DIR}/src/predicate_test.cpp
  ${PROJECT_SOURCE_DIR}/src/rvalue.cpp
  ${PROJECT_SOURCE_DIR}/src/sigslot_demo.cpp
  ${PROJECT_SOURCE_DIR}/src/simple.cpp
  ${PROJECT_SOURCE_DIR}/src/string_format.cpp
  ${PROJECT_SOURCE_DIR}/src/typeindex.cpp
  ${PROJECT_SOURCE_DIR}/src/unicode_test.cpp
  ${PROJECT_SOURCE_DIR}/src/yaml_test.cpp
  ${PROJECT_SOURCE_DIR}/src/sqlite_test.cpp
)

### Targets ###
if (MSVC)
  list(APPEND SRC_FILES src/windows/winsock.cpp
    src/windows/dns_query.cpp)
else()
  list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/openldap.cpp
    ${PROJECT_SOURCE_DIR}/src/rxcpp/post.cpp
    ${PROJECT_SOURCE_DIR}/src/rxcpp/rxcpp_test.cpp)
endif()

add_executable(simple ${SRC_FILES})
target_link_libraries(simple PUBLIC GTest::gtest ${OPENSSL_LIBRARIES} CURL::libcurl Boost::boost ICU::uc ${OpenCV_LIBS} nlohmann_json yaml-cpp)
target_link_libraries(simple PUBLIC double-conversion::double-conversion Libevent::event_core Libevent::event_extra gflags glog::glog fmt::fmt sqlite3::sqlite3)
target_link_libraries(simple PUBLIC Boost::system Boost::filesystem Boost::date_time Boost::context Boost::program_options Boost::regex Boost::random Boost::atomic)

target_include_directories(simple PUBLIC ${fmt_SOURCE_DIR}/include)
target_include_directories(simple PUBLIC ${sigslot_SOURCE_DIR}/include)
target_include_directories(simple PUBLIC "rang/include")
target_include_directories(simple PUBLIC "RxCpp/Rx/v2/src")
target_include_directories(simple PUBLIC "di/include")

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
