cmake_minimum_required(VERSION 3.25)
project(xsdmerge VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    rapidxml
    URL "${CMAKE_CURRENT_SOURCE_DIR}/rapidxml-1.13.1.tar.xz"
    URL_HASH MD5=1798691bd735e92c2c4563c601f8c098
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(rapidxml)
message(STATUS "${rapidxml_SOURCE_DIR}")
add_library(RapidXML INTERFACE)
target_include_directories(RapidXML INTERFACE ${rapidxml_SOURCE_DIR})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG REQUIRED COMPONENTS program_options)
message(STATUS "Boost version ${Boost_VERSION}")

add_executable(xsdmerge "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(xsdmerge PRIVATE RapidXML Boost::program_options)