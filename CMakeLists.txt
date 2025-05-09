cmake_minimum_required(VERSION 3.10)
project(kurs)

set (CMAKE_CXX_STANDARD 11)
set (wxBUILD_SHARED OFF)

add_subdirectory(wxWidgets)


add_executable(kurs kurs.cpp)

target_link_libraries(kurs wx::net wx::core wx::base)
