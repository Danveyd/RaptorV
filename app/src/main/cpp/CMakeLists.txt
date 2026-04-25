cmake_minimum_required(VERSION 3.22.1)

project("raptorv")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(third_party)

add_library(raptorv SHARED
        core/RaptorEngine.cpp
        jni/bridge.cpp)

find_library(log-lib log)

target_link_libraries(raptorv ${log-lib})