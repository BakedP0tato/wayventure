cmake_minimum_required(VERSION 3.19)
set(PROJECT_NAME wayventure)
project(
    ${PROJECT_NAME}
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory("src")

target_link_libraries(${PROJECT_NAME} -static-libstdc++ -static-libgcc -ltcod -lSDL2 -lpthread -lz -ldl)