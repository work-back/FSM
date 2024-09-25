cmake_minimum_required(VERSION 3.22.1)
project(fsm)

set(CMAKE_CXX_STANDARD 20)

add_executable(fsm main.cpp
        fsm.cpp
        sys.cpp
        runfsm.cpp
    )
