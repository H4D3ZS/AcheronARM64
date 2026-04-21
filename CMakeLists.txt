cmake_minimum_required(VERSION 3.16)
project(AcheronARM64 
    VERSION 1.0.0
    DESCRIPTION "ARM64 System Register Emulation Library"
    LANGUAGES CXX
)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(ACHERONARM64_BUILD_TESTS "Build unit tests" OFF)
option(ACHERONARM64_BUILD_EXAMPLES "Build examples" OFF)
option(ACHERONARM64_HEADER_ONLY "Header-only distribution" OFF)
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# Include directories
include(GNUInstallDirs)

# Library target
if(ACHERONARM64_HEADER_ONLY)
    # Header-only mode
    add_library(AcheronARM64 INTERFACE)
    target_include_directories(AcheronARM64 INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
else()
    # Static/shared library mode
    add_library(AcheronARM64
        src/ARM64SystemRegisters.cpp
        src/ARM64InstructionDecoder.cpp
    )
    target_include_directories(AcheronARM64 PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endif()

# Compile options
target_compile_options(AcheronARM64 PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

# Install targets
install(TARGETS AcheronARM64
    EXPORT AcheronARM64Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/AcheronARM64
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Export targets
install(EXPORT AcheronARM64Targets
    FILE AcheronARM64Targets.cmake
    NAMESPACE AcheronARM64::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AcheronARM64
)

# Config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AcheronARM64Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AcheronARM64Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AcheronARM64
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AcheronARM64ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AcheronARM64Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AcheronARM64ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AcheronARM64
)

# Tests
if(ACHERONARM64_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(ACHERONARM64_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Package metadata
set(CPACK_PACKAGE_NAME "AcheronARM64")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ARM64 System Register Emulation Library")
set(CPACK_PACKAGE_VENDOR "AcheronARM64 Contributors")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/acheron-emulator/AcheronARM64")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

include(CPack)
