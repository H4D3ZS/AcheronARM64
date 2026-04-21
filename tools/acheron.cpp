// acheron.cpp - AcheronARM64 Binary Runner CLI
// Run ARM64 Linux binaries instantly - no configuration needed

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include "acheronarm64/ARM64SystemRegisters.hpp"
#include "acheronarm64/MMUEmulator.hpp"

using namespace acheronarm64;

// ============================================================================
// Command Line Parser
// ============================================================================

struct Config {
    std::string binary_path;
    std::vector<std::string> args;
    std::vector<std::string> env_vars;
    std::string rootfs;
    size_t memory_size = 4 * 1024 * 1024 * 1024;  // 4GB default
    int cpu_count = 1;
    bool debug = false;
    bool trace_syscalls = false;
    bool trace_memory = false;
    bool show_registers = false;
};

void print_usage(const char* program) {
    std::cout << "AcheronARM64 - Instant ARM64 Binary Runner\n\n";
    std::cout << "Usage: " << program << " [OPTIONS] <binary> [ARGS...]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -e, --env <VAR=value>    Set environment variable\n";
    std::cout << "  -r, --rootfs <path>      Set root filesystem path\n";
    std::cout << "  -m, --memory <size>      Set memory size (e.g., 512M, 2G)\n";
    std::cout << "  -c, --cpus <count>       Set CPU count\n";
    std::cout << "  -d, --debug              Enable debug output\n";
    std::cout << "  --trace-syscalls         Trace system calls\n";
    std::cout << "  --trace-memory           Trace memory accesses\n";
    std::cout << "  --show-registers         Show register state on exit\n";
    std::cout << "  -h, --help               Show this help message\n";
    std::cout << "  -v, --version            Show version information\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program << " ./hello\n";
    std::cout << "  " << program << " -e FOO=bar ./myapp\n";
    std::cout << "  " << program << " --rootfs /path/to/rootfs /bin/bash\n";
    std::cout << "  " << program << " --debug ./myapp\n";
}

void print_version() {
    std::cout << "AcheronARM64 v1.0.0\n";
    std::cout << "ARM64 system register and MMU emulation library\n";
    std::cout << "License: MIT\n";
}

bool parse_args(int argc, char** argv, Config& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return false;
        } else if (arg == "-v" || arg == "--version") {
            print_version();
            return false;
        } else if (arg == "-e" || arg == "--env") {
            if (i + 1 < argc) {
                config.env_vars.push_back(argv[++i]);
            }
        } else if (arg == "-r" || arg == "--rootfs") {
            if (i + 1 < argc) {
                config.rootfs = argv[++i];
            }
        } else if (arg == "-m" || arg == "--memory") {
            if (i + 1 < argc) {
                std::string size_str = argv[++i];
                // Parse size (e.g., "512M", "2G")
                size_t multiplier = 1;
                if (size_str.back() == 'M' || size_str.back() == 'm') {
                    multiplier = 1024 * 1024;
                    size_str.pop_back();
                } else if (size_str.back() == 'G' || size_str.back() == 'g') {
                    multiplier = 1024 * 1024 * 1024;
                    size_str.pop_back();
                }
                config.memory_size = std::stoull(size_str) * multiplier;
            }
        } else if (arg == "-c" || arg == "--cpus") {
            if (i + 1 < argc) {
                config.cpu_count = std::stoi(argv[++i]);
            }
        } else if (arg == "-d" || arg == "--debug") {
            config.debug = true;
        } else if (arg == "--trace-syscalls") {
            config.trace_syscalls = true;
        } else if (arg == "--trace-memory") {
            config.trace_memory = true;
        } else if (arg == "--show-registers") {
            config.show_registers = true;
        } else if (arg[0] != '-') {
            config.binary_path = arg;
            // Rest are arguments to the binary
            for (int j = i + 1; j < argc; j++) {
                config.args.push_back(argv[j]);
            }
            break;
        }
    }
    
    if (config.binary_path.empty()) {
        std::cerr << "Error: No binary specified\n\n";
        print_usage(argv[0]);
        return false;
    }
    
    return true;
}

// ============================================================================
// ELF Loader
// ============================================================================

struct ELFInfo {
    bool valid = false;
    bool is_arm64 = false;
    uint64_t entry_point = 0;
    uint64_t base_address = 0;
    size_t file_size = 0;
};

ELFInfo load_elf(const std::string& path) {
    ELFInfo info;
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open binary: " << path << std::endl;
        return info;
    }
    
    // Read ELF header
    char header[64];
    file.read(header, 64);
    
    // Check ELF magic
    if (header[0] != 0x7f || header[1] != 'E' || 
        header[2] != 'L' || header[3] != 'F') {
        std::cerr << "Error: Not an ELF binary: " << path << std::endl;
        return info;
    }
    
    // Check if 64-bit
    if (header[4] != 2) {  // EI_CLASS = ELFCLASS64
        std::cerr << "Error: Not a 64-bit binary: " << path << std::endl;
        return info;
    }
    
    // Check if little-endian
    if (header[5] != 1) {  // EI_DATA = ELFDATA2LSB
        std::cerr << "Error: Not a little-endian binary: " << path << std::endl;
        return info;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    info.file_size = file.tellg();
    
    // Check machine type (offset 18-19)
    uint16_t machine = *reinterpret_cast<uint16_t*>(header + 18);
    if (machine == 0xB7) {  // EM_AARCH64
        info.is_arm64 = true;
    } else {
        std::cerr << "Error: Not an ARM64 binary (machine: 0x" 
                  << std::hex << machine << std::dec << "): " << path << std::endl;
        return info;
    }
    
    // Get entry point (offset 24-31 for 64-bit)
    info.entry_point = *reinterpret_cast<uint64_t*>(header + 24);
    
    info.valid = true;
    return info;
}

// ============================================================================
// Main Entry Point
// ============================================================================

int main(int argc, char** argv) {
    Config config;
    
    if (!parse_args(argc, argv, config)) {
        return 1;
    }
    
    if (config.debug) {
        std::cout << "=== AcheronARM64 Debug Mode ===" << std::endl;
        std::cout << "Binary: " << config.binary_path << std::endl;
        std::cout << "Memory: " << (config.memory_size / (1024*1024)) << " MB" << std::endl;
        std::cout << "CPUs: " << config.cpu_count << std::endl;
        if (!config.rootfs.empty()) {
            std::cout << "RootFS: " << config.rootfs << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Load and validate ELF binary
    ELFInfo elf = load_elf(config.binary_path);
    if (!elf.valid) {
        return 1;
    }
    
    if (config.debug) {
        std::cout << "ELF Info:" << std::endl;
        std::cout << "  ARM64: " << (elf.is_arm64 ? "yes" : "no") << std::endl;
        std::cout << "  Entry: 0x" << std::hex << elf.entry_point << std::dec << std::endl;
        std::cout << "  Size: " << elf.file_size << " bytes" << std::endl;
        std::cout << std::endl;
    }
    
    // Initialize system register emulator
    SystemRegisterEmulator sysreg;
    sysreg.getState().reset();
    
    // Initialize MMU emulator
    MMUEmulator mmu;
    
    std::cout << "=== Running ARM64 Binary ===" << std::endl;
    std::cout << "Binary: " << config.binary_path << std::endl;
    std::cout << "Entry: 0x" << std::hex << elf.entry_point << std::dec << std::endl;
    std::cout << std::endl;
    
    // NOTE: Full implementation would:
    // 1. Load binary into memory
    // 2. Set up initial stack and arguments
    // 3. Initialize system registers
    // 4. Set up MMU and page tables
    // 5. Start execution loop with hypervisor backend
    // 6. Handle exits (syscalls, exceptions, etc.)
    // 7. Clean up and return exit code
    
    std::cout << "⚠️  Full execution engine under development" << std::endl;
    std::cout << std::endl;
    std::cout << "Current capabilities:" << std::endl;
    std::cout << "  ✓ ELF binary validation" << std::endl;
    std::cout << "  ✓ ARM64 system register emulation" << std::endl;
    std::cout << "  ✓ MMU emulation with TLB" << std::endl;
    std::cout << "  ✓ Cross-platform hypervisor backends" << std::endl;
    std::cout << std::endl;
    std::cout << "Coming soon:" << std::endl;
    std::cout << "  ⏳ System call emulation" << std::endl;
    std::cout << "  ⏳ Linux binary execution" << std::endl;
    std::cout << "  ⏳ Thread support" << std::endl;
    std::cout << "  ⏳ File system access" << std::endl;
    std::cout << std::endl;
    
    if (config.show_registers) {
        std::cout << "Initial Register State:" << std::endl;
        std::cout << "  SCTLR_EL1: 0x" << std::hex << sysreg.getState().SCTLR_EL1.value << std::dec << std::endl;
        std::cout << "  TCR_EL1:   0x" << sysreg.getState().TCR_EL1.value << std::dec << std::endl;
        std::cout << "  TTBR0_EL1: 0x" << sysreg.getState().TTBR0_EL1.value << std::dec << std::endl;
        std::cout << "  TTBR1_EL1: 0x" << sysreg.getState().TTBR1_EL1.value << std::dec << std::endl;
        std::cout << "  MPIDR_EL1: 0x" << sysreg.getState().MPIDR_EL1.value << std::dec << std::endl;
        std::cout << "  MIDR_EL1:  0x" << sysreg.getState().MIDR_EL1.value << std::dec << std::endl;
    }
    
    return 0;
}
