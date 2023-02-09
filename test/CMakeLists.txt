cmake_minimum_required(VERSION 3.14)

project(ok-serializerTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)

# ---- Tests ----

add_executable(ok-serializer_test source/ok-serializer_test.cpp)
target_link_libraries(ok-serializer_test PRIVATE ok-serializer::ok-serializer Catch2::Catch2WithMain)
target_compile_features(ok-serializer_test PRIVATE cxx_std_23)

# ---- End-of-file commands ----

add_folders(Test)
