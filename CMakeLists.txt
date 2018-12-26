# Copyright (c) 2014-2016, Ruslan Baratov
# All rights reserved.

cmake_minimum_required(VERSION 3.12)

### Hunter snapshot that will be used ###
include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.23.58.tar.gz"
  SHA1 "ee80dc91f91f5eb6892f80a0a92c83b4de8e980a"
  LOCAL
)

include(FetchContent)
include(cmake/fmt.cmake)

project(HunterSimple)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
  # minimize the size or the resulting EXE
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

# https://github.com/boostorg/system/issues/26
add_compile_options(-DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY)
set(CMAKE_CXX_STANDARD 14)
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

add_subdirectory(yaml-cpp)
add_subdirectory(cpprestsdk)

#CMake doesn't generate compile_commands.json on Windows when the compiler is from VS
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Targets ###
if (MSVC)
  file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp
                      ${PROJECT_SOURCE_DIR}/src/windows/winsock.cpp)
else()
  file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp
                      ${PROJECT_SOURCE_DIR}/src/rxcpp/*.cpp)
endif()

### Don't link to folly temporarily
### The brew installed folly will conflict with gflags. The error is as below
### ERROR: flag 'flagfile' was defined more than once (in files '/tmp/gflags-20170916-46768-ftgkio/gflags-2.2.1/src/gflags.cc' 
### and '/Users/juchen3/.hunter/_Base/ee80dc9/814ec5f/72fe860/Build/gflags/Source/src/gflags.cc').
list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/folly.cpp)

if(MSVC)
  list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/openldap.cpp)
  list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/cpprest_test.cpp)
  list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/http_client_test.cpp)
endif()

add_executable(simple ${SRC_FILES})
target_link_libraries(simple PUBLIC GTest::gtest ${OPENSSL_LIBRARIES} CURL::libcurl Boost::boost ICU::uc ${OpenCV_LIBS} nlohmann_json yaml-cpp)
target_link_libraries(simple PUBLIC double-conversion::double-conversion Libevent::event_core Libevent::event_extra gflags glog::glog fmt::fmt)
target_link_libraries(simple PUBLIC Boost::system Boost::filesystem Boost::date_time Boost::context Boost::program_options Boost::regex Boost::random Boost::atomic)
target_link_libraries(simple PUBLIC cpprest)

target_include_directories(simple PUBLIC ${fmt_SOURCE_DIR}/include)
target_include_directories(simple PUBLIC ${sigslot_SOURCE_DIR}/include)
target_include_directories(simple PUBLIC "rang/include")
target_include_directories(simple PUBLIC "RxCpp/Rx/v2/src")
target_include_directories(simple PUBLIC "di/include")
target_include_directories(simple PUBLIC "di/include")
target_include_directories(simple PUBLIC "cpprestsdk/Release/include")

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
