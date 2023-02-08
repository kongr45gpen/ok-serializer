if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/ok-serializer-${PROJECT_VERSION}"
      CACHE PATH ""
  )
endif()

# Project is configured with no languages, so tell GNUInstallDirs the lib dir
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package ok-serializer)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT ok-serializer_Development
)

install(
    TARGETS ok-serializer_ok-serializer
    EXPORT ok-serializerTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    ok-serializer_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(ok-serializer_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${ok-serializer_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT ok-serializer_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${ok-serializer_INSTALL_CMAKEDIR}"
    COMPONENT ok-serializer_Development
)

install(
    EXPORT ok-serializerTargets
    NAMESPACE ok-serializer::
    DESTINATION "${ok-serializer_INSTALL_CMAKEDIR}"
    COMPONENT ok-serializer_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
