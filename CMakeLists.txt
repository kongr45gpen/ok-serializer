cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
        ok-serializer
        VERSION 0.1.0
        DESCRIPTION "A modern binary data (de)serializer that's quite OK"
        HOMEPAGE_URL "https://github.com/kongr45gpen/ok-serializer"
        LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(ok-serializer_ok-serializer INTERFACE)
add_library(ok-serializer::ok-serializer ALIAS ok-serializer_ok-serializer)

set_property(
    TARGET ok-serializer_ok-serializer PROPERTY
    EXPORT_NAME ok-serializer
)

target_include_directories(
    ok-serializer_ok-serializer ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(ok-serializer_ok-serializer INTERFACE cxx_std_23)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

add_subdirectory(example)

include(cmake/dev-mode.cmake)
