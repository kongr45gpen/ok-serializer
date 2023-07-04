cmake_minimum_required(VERSION 3.14)

project(ok-serializerExamples CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/reflection.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(ok-serializer REQUIRED)
endif()

add_custom_target(run-examples)

function(add_example NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE ok-serializer::ok-serializer)
  target_compile_features("${NAME}" PRIVATE cxx_std_23)
endfunction()

add_example(empty_example)

if (HAVE_REFLECTION)
  add_example(reflection_example)
else ()
  message(WARNING "Reflection is not supported by this compiler. Optional reflection features will not be built.")
  message(WARNING "Check the CMakeConfigureLog to verify why the reflection test code failed to compile.")
endif ()


add_folders(Example)
