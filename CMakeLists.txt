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
find_package(oatpp-openssl 1.3.0 REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(include)
include_directories(../mms_kfunctions)
include_directories(${CMAKE_SYSROOT}/usr/include/kfunctions)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" )

target_link_libraries(${PROJECT_NAME} PUBLIC oatpp::oatpp oatpp::oatpp-openssl ssl crypto kfunctions) #PUBLIC oatpp::oatpp-test

#################################################################
## define certificates path
if(BUILDROOT)
  add_definitions(-DBUILDROOT)
  add_definitions(
    -DCERT_PEM_PATH="/usr/share/container_api/test_key.pem"
    -DCERT_CRT_PATH="/usr/share/container_api/test_cert.crt"
)
else()
  add_definitions(
    -DCERT_PEM_PATH="${CMAKE_CURRENT_LIST_DIR}/cert/test_key.pem"
    -DCERT_CRT_PATH="${CMAKE_CURRENT_LIST_DIR}/cert/test_cert.crt"
)
endif()
#################################################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra -Wpedantic") # TODO: append -s -Q --help=warning




set(CMAKE_INSTALL_PREFIX "/")
install(TARGETS oat_db DESTINATION fs0/prg)
