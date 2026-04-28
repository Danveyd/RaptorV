cmake_minimum_required(VERSION 3.22.1)
project(JoltPhysics)

file(GLOB_RECURSE JOLT_SOURCES "*.cpp" "*.h" "*.inl")

add_library(Jolt STATIC ${JOLT_SOURCES})

set_target_properties(Jolt PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
)

target_include_directories(Jolt PUBLIC ..)

target_compile_definitions(Jolt PUBLIC
        JPH_OPEN_SOURCE
        JPH_OBJECT_STREAM
        JPH_CROSS_PLATFORM_DETERMINISTIC
        JPH_DEBUG_RENDERER
)

target_compile_options(Jolt PRIVATE -O3 -fno-exceptions -fno-rtti)
target_compile_options(Jolt PRIVATE -w)

if(ANDROID_ABI STREQUAL "arm64-v8a")
    target_compile_definitions(Jolt PUBLIC JPH_USE_NEON)
elseif(ANDROID_ABI STREQUAL "x86_64")
    target_compile_definitions(Jolt PUBLIC
            JPH_USE_SSE4_1
            JPH_USE_SSE4_2
            JPH_USE_LZCNT
            JPH_USE_TZCNT
            JPH_USE_F16C
            JPH_USE_PCLMUL
    )
    target_compile_options(Jolt PUBLIC -msse4.2 -mpopcnt -mlzcnt -mf16c)
endif()
