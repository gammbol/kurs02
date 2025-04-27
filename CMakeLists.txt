cmake_minimum_required(VERSION 3.10)
project(kurs)

set (CMAKE_CXX_STANDARD 11)

find_package(wxWidgets REQUIRED net core base)
if(wxWidgets_USE_FILE) # not defined in CONFIG mode
    include(${wxWidgets_USE_FILE})
endif()

add_executable(kurs kurs.cpp)

target_link_libraries(kurs ${wxWidgets_LIBRARIES})
