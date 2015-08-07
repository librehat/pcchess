project(pcchess C CXX)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

find_package(MPI REQUIRED)
find_package(Boost 1.57 REQUIRED COMPONENTS mpi serialization)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${MPI_CXX_INCLUDE_PATH})

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

file(GLOB CORESRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/core/*.h ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)
file(GLOB PLAYERSRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/player/utils/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/player/*.h ${CMAKE_CURRENT_SOURCE_DIR}/player/*.cpp)

add_library(pcc_core SHARED ${CORESRC})
add_library(pcc_player SHARED ${PLAYERSRC})

add_executable(pcc_serial pcc_serial.cpp)
add_executable(pcc_mpi pcc_mpi.cpp)

target_link_libraries(pcc_serial pcc_core pcc_player ${MPI_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_link_libraries(pcc_mpi pcc_core pcc_player ${MPI_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

find_library(gperf profiler)
if (gperf)
    message("-- Found Google performance tools")
    target_link_libraries(pcc_serial debug profiler)
    target_link_libraries(pcc_mpi debug profiler)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -Wall -Wextra -Wshadow")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(AUTHOR_WARNING "You're using Clang, atomic library may be needed.")
    target_link_libraries(pcc_serial atomic)
    target_link_libraries(pcc_mpi atomic)
endif()
