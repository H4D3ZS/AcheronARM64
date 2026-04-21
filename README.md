# AcheronARM64

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-cross--platform-blue)](https://github.com/H4D3ZS/AcheronARM64)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-green.svg)](https://en.cppreference.com/w/cpp/17)

**A cross-platform C++ library for ARM64 system register and MMU emulation.**

---

## Overview

AcheronARM64 provides production-ready ARM64 system register and memory management emulation for hypervisor developers, emulator authors, and OS researchers.

### Key Features

- ✅ **Complete EL1 System Registers** — SCTLR, TCR, TTBR0/1, ESR, FAR, ELR, VBAR, MPIDR, MIDR
- ✅ **MMU Emulation** — 4-level page table walking, TLB caching, ASID support
- ✅ **System Instruction Decoder** — MSR, MRS, ISB, DSB, DMB, WFI, WFE, BRK
- ✅ **Cross-Platform** — Works with any hypervisor backend (HVF, WHP, KVM, JIT)
- ✅ **Zero Dependencies** — Pure C++17, works anywhere
- ✅ **Well-Tested** — 55+ unit tests with GoogleTest
- ✅ **Documented** — Full API reference and integration examples

### Use Cases

1. **Hypervisor Development** — Build ARM64 hypervisors on x86_64 or ARM hosts
2. **OS Development** — Test ARM64 OS kernels without physical hardware
3. **Security Research** — Analyze ARM64 software in sandboxed environments
4. **Education** — Teach ARM64 architecture and virtualization concepts
5. **Cross-Platform Testing** — Run ARM64 builds on x86_64 CI infrastructure
6. **Emulator Projects** — Add ARM64 CPU emulation to existing emulators

---

## Quick Start

### Installation

```bash
git clone https://github.com/H4D3ZS/AcheronARM64.git
cd AcheronARM64
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Basic Usage

```cpp
#include <acheronarm64/ARM64SystemRegisters.hpp>
#include <acheronarm64/MMUEmulator.hpp>

using namespace acheronarm64;

int main() {
    // System register emulation
    SystemRegisterEmulator sysreg;
    sysreg.getState().reset();
    
    // MMU emulation
    MMUEmulator mmu;
    mmu.set_ttbr0(0x40000000);
    mmu.set_tcr(0x00000000);
    
    // Translate virtual address
    MemoryRegion region;
    auto pa = mmu.translate_with_attributes(0xFFFFFFF007004000, region);
    
    return 0;
}
```

### Integration Example

```cpp
// In your hypervisor exit handler
void handle_trap(uint64_t pc, uint32_t esr) {
    uint32_t ec = (esr >> 26) & 0x3F;
    
    if (ec == ExceptionClass::SYSTEM_REGISTER_TRAP) {
        uint32_t instr = read_instruction(pc);
        auto decoded = decodeSystemInstruction(instr);
        
        emulator.emulateMSR(decoded, getReg, setReg);
        advance_pc(pc + 4);
    }
}
```

---

## Components

### System Register Emulation

| Register | Purpose | Status |
|----------|---------|--------|
| SCTLR_EL1 | System Control | ✅ Full |
| TCR_EL1 | Translation Control | ✅ Full |
| TTBR0_EL1 | Translation Table Base 0 | ✅ Full |
| TTBR1_EL1 | Translation Table Base 1 | ✅ Full |
| ESR_EL1 | Exception Syndrome | ✅ Read/Write |
| FAR_EL1 | Fault Address | ✅ Read/Write |
| ELR_EL1 | Exception Link | ✅ Read/Write |
| VBAR_EL1 | Vector Base Address | ✅ Full |
| MPIDR_EL1 | Multiprocessor Affinity | ✅ Read-only |
| MIDR_EL1 | Main ID Register | ✅ Read-only |

### MMU Emulation

| Feature | Description | Status |
|---------|-------------|--------|
| Page Table Walk | 4-level ARM64 page tables | ✅ Complete |
| TLB Cache | Software TLB with ASID tagging | ✅ Complete |
| Block Mappings | 1GB, 2MB blocks | ✅ Complete |
| Page Mappings | 4KB pages | ✅ Complete |
| Memory Attributes | Type, permissions, XN bits | ✅ Complete |
| Invalidate Ops | VA, ASID, full flush | ✅ Complete |

### Instruction Support

- ✅ `MSR <sysreg>, #imm` — System register write (immediate)
- ✅ `MSR <sysreg>, <Xn>` — System register write (register)
- ✅ `MRS <Xn>, <sysreg>` — System register read
- ✅ `ISB` — Instruction synchronization barrier
- ✅ `DSB` — Data synchronization barrier
- ✅ `DMB` — Data memory barrier
- ✅ `WFI` — Wait for interrupt
- ✅ `WFE` — Wait for event
- ✅ `BRK #imm` — Software breakpoint

---

## Platform Support

| Platform | Hypervisor | Backend | Status |
|----------|------------|---------|--------|
| **macOS** | Hypervisor.framework | `hvf_integration.cpp` | ✅ Tested |
| **Windows** | Windows Hypervisor Platform | `whp_integration.cpp` | ✅ Tested |
| **Linux** | KVM | `kvm_integration.cpp` | ✅ Tested |
| **Cross-platform** | Pure JIT | `basic_usage.cpp` | ✅ Tested |

---

## Building

### Requirements

- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16+
- (Optional) GoogleTest for tests

### Build Options

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DACHERONARM64_BUILD_TESTS=ON \
    -DACHERONARM64_BUILD_EXAMPLES=ON
cmake --build build
```

### CMake Integration

```cmake
find_package(AcheronARM64 REQUIRED)
target_link_libraries(your_target PRIVATE acheronarm64::arm64emu)
```

Or use FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    AcheronARM64
    GIT_REPOSITORY https://github.com/H4D3ZS/AcheronARM64.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(AcheronARM64)
```

---

## Testing

```bash
cd build
ctest --output-on-failure
```

### Test Coverage

- ✅ System instruction decoding (all variants)
- ✅ System register read/write
- ✅ MMU translation (all levels)
- ✅ TLB operations
- ✅ Exception handling
- ✅ Memory attribute decoding

---

## Documentation

| Document | Description |
|----------|-------------|
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Library design and internals |
| [RISK_ASSESSMENT.md](docs/RISK_ASSESSMENT.md) | Legal and security considerations |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Contribution guidelines |

---

## Performance

### Instruction Throughput

| Scenario | Instructions/sec | Notes |
|----------|-----------------|-------|
| Native (no trap) | ~10M+ | Direct execution |
| With emulation | ~50K | System register traps |
| Pure JIT | ~500K | Binary translation |

### Optimization Tips

1. **Cache Decoded Instructions** — Avoid re-decoding repeated traps
2. **Batch Exits** — Handle multiple exits before returning to guest
3. **TLB Tuning** — Adjust TLB size for your workload
4. **Header-Only Mode** — Eliminate function call overhead

---

## License

**MIT License** — Free for commercial and private use.

```
Copyright (c) 2024 AcheronARM64 Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.
```

### Commercial Use

✅ **Allowed** — Use in commercial products, proprietary software, and closed-source projects.

---

## Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Ways to Contribute

- 🐛 Report bugs
- 💡 Suggest features
- 📝 Improve documentation
- 🔧 Submit PRs
- 🧪 Write tests

---

## Acknowledgments

- ARM Holdings — ARM Architecture Reference Manual (ARMv8, ARMv9)
- QEMU Community — Reference implementations
- KVM Developers — ARM64 virtualization

---

## Contact

- **Repository:** https://github.com/H4D3ZS/AcheronARM64
- **Issues:** https://github.com/H4D3ZS/AcheronARM64/issues
- **Discussions:** https://github.com/H4D3ZS/AcheronARM64/discussions

---

## FAQ

### Q: Is this legal?
**A:** Yes. Implements standardized ARM architecture registers from public ARM documentation. No proprietary code included.

### Q: Does this work on x86_64?
**A:** Yes! Pure C++ with no platform-specific code. Pair with a JIT translator for x86_64 → ARM64 emulation.

### Q: Is this production-ready?
**A:** Yes. API is stable at v1.0. Used in production hypervisor deployments.

### Q: What about EL2/EL3 registers?
**A:** Future roadmap. Current focus is EL1 (guest OS) emulation.

### Q: Can I use this commercially?
**A:** Yes, MIT license allows commercial use. Attribution appreciated but not required.
