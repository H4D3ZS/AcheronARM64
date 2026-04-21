# tests/CMakeLists.txt - Test suite configuration

enable_testing()

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

# Test executable
add_executable(acheronarm64_tests
    test_main.cpp
    test_decoder.cpp
    test_emulator.cpp
)

target_link_libraries(acheronarm64_tests
    PRIVATE
        arm64emu
        GTest::gtest_main
)

target_include_directories(acheronarm64_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Set C++17
target_compile_features(acheronarm64_tests PRIVATE cxx_std_17)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(acheronarm64_tests)
