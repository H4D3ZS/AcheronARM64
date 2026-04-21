# examples/CMakeLists.txt - Example programs

# Basic usage example (cross-platform)
add_executable(basic_usage
    basic_usage.cpp
)
target_link_libraries(basic_usage PRIVATE arm64emu)
target_compile_features(basic_usage PRIVATE cxx_std_17)

# macOS HVF example
if(APPLE)
    add_executable(hvf_integration
        hvf_integration.cpp
    )
    target_link_libraries(hvf_integration PRIVATE arm64emu "-framework Hypervisor")
    target_compile_features(hvf_integration PRIVATE cxx_std_17)
endif()

# Windows WHP example
if(WIN32)
    add_executable(whp_integration
        whp_integration.cpp
    )
    target_link_libraries(whp_integration PRIVATE arm64emu WinHvPlatform)
    target_compile_features(whp_integration PRIVATE cxx_std_17)
endif()

# Linux KVM example
if(LINUX)
    add_executable(kvm_integration
        kvm_integration.cpp
    )
    target_link_libraries(kvm_integration PRIVATE arm64emu)
    target_compile_features(kvm_integration PRIVATE cxx_std_17)
endif()
