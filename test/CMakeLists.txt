cmake_minimum_required(VERSION 3.14)

project(ok-serializerTests LANGUAGES CXX)

Include(FetchContent)
include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

# ---- Dependencies ----


FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.3.2
)

SET(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(Catch2)

# ---- Tests ----

add_executable(ok-serializer_test source/serializer_test.cpp source/outputs_test.cpp source/deserializer_test.cpp source/static_test.cpp source/idempotence_test.cpp)
target_link_libraries(ok-serializer_test PRIVATE ok-serializer::ok-serializer Catch2::Catch2WithMain)
target_compile_features(ok-serializer_test PRIVATE cxx_std_23)

add_executable(ok-serializer_bench source/benchmarks.cpp)
target_link_libraries(ok-serializer_bench PRIVATE ok-serializer::ok-serializer Catch2::Catch2WithMain)
target_compile_features(ok-serializer_bench PRIVATE cxx_std_23)
target_compile_options(ok-serializer_bench PUBLIC -fno-rtti -fno-exceptions -O1)

# ---- Assembly code tests ----
add_library(ok-serializer_perf_o0 STATIC source/optimizer_test.cpp)
target_link_libraries(ok-serializer_perf_o0 PRIVATE ok-serializer::ok-serializer)
target_compile_options(ok-serializer_perf_o0 PUBLIC -fno-rtti -fno-exceptions -O0)

add_library(ok-serializer_perf_o1 STATIC source/optimizer_test.cpp)
target_link_libraries(ok-serializer_perf_o1 PRIVATE ok-serializer::ok-serializer)
target_compile_options(ok-serializer_perf_o1 PUBLIC -fno-rtti -fno-exceptions -O1)

add_library(ok-serializer_perf_o3 STATIC source/optimizer_test.cpp)
target_link_libraries(ok-serializer_perf_o3 PRIVATE ok-serializer::ok-serializer)
target_compile_options(ok-serializer_perf_o3 PUBLIC -fno-rtti -fno-exceptions -O3)

# ---- End-of-file commands ----

add_folders(Test)
