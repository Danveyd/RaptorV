cmake_minimum_required(VERSION 3.22.1)
project("raptorv")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(third_party)
include_directories(third_party/filament/include)
include_directories(third_party/filament/include/gmath)

add_subdirectory(third_party/Jolt)

file(GLOB FILAMENT_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/filament/lib/${ANDROID_ABI}/*.a")

message(STATUS "Filament libs found: ${FILAMENT_LIBS}")

add_library(raptorv SHARED
        core/RaptorEngine.cpp
        render/Renderer.cpp
        scene/Scene.cpp
        physics/PhysicsWorld.cpp
        jni/bridge.cpp)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(raptorv
        -Wl,--start-group
        ${FILAMENT_LIBS}
        Jolt
        -Wl,--end-group

        ${log-lib}
        ${android-lib}
        EGL GLESv3 z atomic m
)
