cmake_minimum_required(VERSION 3.5.0)
project(oat_db VERSION 0.1.0 LANGUAGES C CXX)

configure_file(version.h.in ../include/version.h)

include_directories(include)
# add_library(${PROJECT_NAME}-lib 
#             include/component.h
#             include/controller.h
#             include/dto.h)

# Specify the directory containing your source files
file(GLOB SOURCES "src/*.cpp")

find_package(oatpp  1.3.0 REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" )

target_link_libraries(${PROJECT_NAME} PUBLIC oatpp::oatpp ssl crypto) #PUBLIC oatpp::oatpp-test

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra -Wpedantic") # TODO: append -s -Q --help=warning