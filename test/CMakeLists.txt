cmake_minimum_required(VERSION 3.14)

project(ok-serializerTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(ok-serializer REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(ok-serializer_test source/ok-serializer_test.cpp)
target_link_libraries(ok-serializer_test PRIVATE ok-serializer::ok-serializer)
target_compile_features(ok-serializer_test PRIVATE cxx_std_20)

add_test(NAME ok-serializer_test COMMAND ok-serializer_test)

# ---- End-of-file commands ----

add_folders(Test)
