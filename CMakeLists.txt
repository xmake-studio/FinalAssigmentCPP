cmake_minimum_required(VERSION 3.15)
project(schedule_db CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(schedule_db
    src/main.cpp
    src/Menu.cpp
    src/ScheduleDB.cpp
    src/ScheduleRecord.cpp
)

target_include_directories(schedule_db PRIVATE src)
