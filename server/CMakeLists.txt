cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

project(server)


# 获取mysql_config的输出
execute_process(
  COMMAND mysql_config --cflags
  OUTPUT_VARIABLE MYSQL_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND mysql_config --libs
  OUTPUT_VARIABLE MYSQL_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 设置编译选项和链接选项
include_directories(${MYSQL_CFLAGS})
link_libraries(${MYSQL_LIBS})



aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)
file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.ini)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES})
# 确保正确查找并链接 Asio
list(APPEND CMAKE_PREFIX_PATH "/opt/vcpkg/vcpkg/installed/x64-linux/share")
set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE)
find_package(asio REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    pthread 
    asio
)

