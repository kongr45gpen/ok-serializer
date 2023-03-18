cmake_minimum_required(VERSION 3.14)

project(ok-serializerTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)

# ---- Tests ----

add_executable(ok-serializer_test source/serializer_test.cpp source/outputs_test.cpp source/deserializer_test.cpp source/static_test.cpp)
target_link_libraries(ok-serializer_test PRIVATE ok-serializer::ok-serializer Catch2::Catch2WithMain)
target_compile_features(ok-serializer_test PRIVATE cxx_std_23)

# ---- Assembly code tests ----
add_library(ok-serializer_perf_o0 STATIC source/optimizer_test.cpp)
target_link_libraries(ok-serializer_perf_o0 PRIVATE ok-serializer::ok-serializer)
target_compile_options(ok-serializer_perf_o0 PUBLIC -fno-rtti -fno-exceptions -O0)

add_library(ok-serializer_perf_o1 STATIC source/optimizer_test.cpp)
target_link_libraries(ok-serializer_perf_o1 PRIVATE ok-serializer::ok-serializer)
target_compile_options(ok-serializer_perf_o1 PUBLIC -fno-rtti -fno-exceptions -O1)

# ---- End-of-file commands ----

add_folders(Test)
