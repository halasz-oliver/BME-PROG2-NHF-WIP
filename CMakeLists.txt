cmake_minimum_required(VERSION 3.30)
project(BME_PROG2_NHF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(MEMTRACE)

add_executable(BME_PROG2_NHF
        main.cpp
        alarm.cpp alarm.h
        component.h
        logicalgate.cpp logicalgate.h
        sensor.cpp sensor.h
        switch.cpp switch.h
        memtrace.cpp)

add_executable(BME_PROG2_TEST
        test.cpp
        alarm.cpp alarm.h
        component.h
        logicalgate.cpp logicalgate.h
        sensor.cpp sensor.h
        switch.cpp switch.h
        memtrace.cpp)


target_include_directories(BME_PROG2_TEST PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")