// ARM64SystemRegisters.hpp
// Cross-platform ARM64 system register emulation
// Part of AcheronARM64 - ARM64 System Register Emulation Library
//
// This library emulates ARM64 system registers for hypervisor implementations
// where the host hypervisor (HVF/WHP/KVM) traps system register access.
//
// License: MIT (see LICENSE)
// Repository: https://github.com/acheron-emulator/AcheronARM64

#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <functional>
#include <optional>

namespace acheronarm64 {

// ============================================================================
// ARM64 System Register Definitions
// Reference: ARM Architecture Reference Manual (ARMv8, ARMv9)
// ============================================================================

/**
 * @brief SCTLR_EL1 - System Control Register (Exception Level 1)
 * 
 * Controls fundamental CPU behavior: MMU, caches, alignment checks, etc.
 * This is typically the FIRST register written during kernel boot.
 */
struct SCTLR_EL1 {
    union {
        uint64_t value;
        struct {
            uint64_t M      : 1;  // [0]  MMU enable
            uint64_t A      : 1;  // [1]  Alignment check enable
            uint64_t C      : 1;  // [2]  Data cache enable
            uint64_t SA     : 1;  // [3]  Stack alignment check
            uint64_t SA0    : 1;  // [4]  Stack alignment check EL0
            uint64_t _rsvd1 : 3;  // [5:7]
            uint64_t ITD    : 1;  // [8]  IT instruction disable
            uint64_t _rsvd2 : 3;  // [9:11]
            uint64_t I      : 1;  // [12] Instruction cache enable
            uint64_t DZE    : 1;  // [13] Division by zero trap
            uint64_t _rsvd3 : 2;  // [14:15]
            uint64_t UCT    : 1;  // [16] Trap undefined cache ops
            uint64_t nTWE   : 1;  // [17] Trap WFE
            uint64_t _rsvd4 : 1;  // [18]
            uint64_t nTWI   : 1;  // [19] Trap WFI
            uint64_t _rsvd5 : 4;  // [20:23]
            uint64_t UCI    : 1;  // [24] Trap cache maintenance EL0
            uint64_t _rsvd6 : 3;  // [25:27]
            uint64_t EE     : 1;  // [28] Endianness (data)
            uint64_t _rsvd7 : 1;  // [29]
            uint64_t E0E    : 1;  // [30] Endianness EL0
            uint64_t SPAN   : 1;  // [31] Set Privileged Access Never
            uint64_t _rsvd8 : 4;  // [32:35]
            uint64_t ENIA   : 1;  // [36] Enable pointer auth (instr)
            uint64_t ENIB   : 1;  // [37] Enable pointer auth (branch)
            uint64_t _rsvd9 : 2;  // [38:39]
            uint64_t BT0    : 1;  // [40] Branch target indent (EL0)
            uint64_t BT1    : 1;  // [41] Branch target indent (EL1)
            uint64_t _rsvd10: 22; // [42:63]
        };
    };
    
    SCTLR_EL1() : value(0) {}
    explicit SCTLR_EL1(uint64_t val) : value(val) {}
    
    // Default reset value (MMU/caches disabled, safe defaults)
    static constexpr uint64_t RESET_VALUE = 0x00C50838ULL;
};

/**
 * @brief TCR_EL1 - Translation Control Register (Exception Level 1)
 * 
 * Controls address translation when MMU is enabled.
 */
struct TCR_EL1 {
    union {
        uint64_t value;
        struct {
            uint64_t T0SZ   : 6;  // [0:5]  Size offset for TTBR0
            uint64_t _rsvd1 : 2;  // [6:7]
            uint64_t IRGN0  : 2;  // [8:9]  Inner cacheability TTBR0
            uint64_t ORGN0  : 2;  // [10:11] Outer cacheability TTBR0
            uint64_t SH0    : 2;  // [12:13] Shareability TTBR0
            uint64_t TG0    : 2;  // [14:15] Granule TTBR0
            uint64_t T1SZ   : 6;  // [16:21] Size offset for TTBR1
            uint64_t A1     : 1;  // [22]    ASID from TTBR1
            uint64_t _rsvd2 : 1;  // [23]
            uint64_t IRGN1  : 2;  // [24:25] Inner cacheability TTBR1
            uint64_t ORGN1  : 2;  // [26:27] Outer cacheability TTBR1
            uint64_t SH1    : 2;  // [28:29] Shareability TTBR1
            uint64_t TG1    : 2;  // [30:31] Granule TTBR1
            uint64_t _rsvd3 : 2;  // [32:33]
            uint64_t IPS    : 3;  // [34:36] Input address size
            uint64_t _rsvd4 : 1;  // [37]
            uint64_t AS     : 1;  // [38] Extended ASID
            uint64_t TBI0   : 1;  // [39] Top byte ignore TTBR0
            uint64_t TBI1   : 1;  // [40] Top byte ignore TTBR1
            uint64_t _rsvd5 : 23; // [41:63]
        };
    };
    
    TCR_EL1() : value(0) {}
    explicit TCR_EL1(uint64_t val) : value(val) {}
    
    static constexpr uint64_t RESET_VALUE = 0x00000000ULL;
};

/**
 * @brief TTBR0_EL1 - Translation Table Base Register 0
 * 
 * Base address of level 0 translation table for TTBR0.
 */
struct TTBR0_EL1 {
    union {
        uint64_t value;
        struct {
            uint64_t BADDR  : 48; // [0:47] Base address (4KB aligned)
            uint64_t ASID   : 16; // [48:63] ASID (if ASID=16bit)
        };
    };
    
    TTBR0_EL1() : value(0) {}
    explicit TTBR0_EL1(uint64_t val) : value(val) {}
    
    uint64_t getBaseAddress() const { return (value & 0xFFFFFFFFFFFF0000ULL); }
    void setBaseAddress(uint64_t addr) { value = (value & 0xFFFF000000000000ULL) | (addr & 0xFFFFFFFFFFFF0000ULL); }
};

/**
 * @brief TTBR1_EL1 - Translation Table Base Register 1
 */
struct TTBR1_EL1 {
    union {
        uint64_t value;
        struct {
            uint64_t BADDR  : 48;
            uint64_t ASID   : 16;
        };
    };
    
    TTBR1_EL1() : value(0) {}
    explicit TTBR1_EL1(uint64_t val) : value(val) {}
};

/**
 * @brief ESR_EL1 - Exception Syndrome Register
 */
struct ESR_EL1 {
    union {
        uint64_t value;
        struct {
            uint64_t ISS    : 25; // [0:24]  Instruction Specific Syndrome
            uint64_t _rsvd1 : 1;  // [25]
            uint64_t EC     : 6;  // [26:31] Exception Class
            uint64_t IL     : 1;  // [32]    Instruction Length
            uint64_t _rsvd2 : 31; // [33:63]
        };
    };
    
    ESR_EL1() : value(0) {}
    explicit ESR_EL1(uint64_t val) : value(val) {}
    
    uint8_t getExceptionClass() const { return (value >> 26) & 0x3F; }
    bool isInstruction32Bit() const { return (value >> 32) & 0x1; }
};

// Exception Class codes
namespace ExceptionClass {
    constexpr uint8_t UNKNOWN = 0x00;
    constexpr uint8_t WFI_WFE = 0x01;
    constexpr uint8_t UNDEFINED = 0x03;
    constexpr uint8_t SYSTEM_REGISTER_TRAP = 0x05;  // MRS/MSR
    constexpr uint8_t WFX_TRAP = 0x08;
    constexpr uint8_t SVC = 0x18;
    constexpr uint8_t HVC = 0x19;
    constexpr uint8_t SMC = 0x1A;
    constexpr uint8_t INSTR_ABORT_LOWER = 0x20;
    constexpr uint8_t INSTR_ABORT_SAME = 0x21;
    constexpr uint8_t DATA_ABORT_LOWER = 0x24;
    constexpr uint8_t DATA_ABORT_SAME = 0x25;
    constexpr uint8_t SOFTWARE_BREAKPOINT = 0x3C;
}

/**
 * @brief FAR_EL1 - Fault Address Register
 */
struct FAR_EL1 {
    uint64_t value;
    FAR_EL1() : value(0) {}
    explicit FAR_EL1(uint64_t val) : value(val) {}
};

/**
 * @brief ELR_EL1 - Exception Link Register
 */
struct ELR_EL1 {
    uint64_t value;
    ELR_EL1() : value(0) {}
    explicit ELR_EL1(uint64_t val) : value(val) {}
};

/**
 * @brief VBAR_EL1 - Vector Base Address Register
 */
struct VBAR_EL1 {
    uint64_t value;
    VBAR_EL1() : value(0) {}
    explicit VBAR_EL1(uint64_t val) : value(val) {}
};

/**
 * @brief MPIDR_EL1 - Multiprocessor Affinity Register
 */
struct MPIDR_EL1 {
    union {
        uint64_t value;
        struct {
            uint64_t Aff0   : 8;  // [0:7]   Affinity level 0
            uint64_t Aff1   : 8;  // [8:15]  Affinity level 1
            uint64_t Aff2   : 8;  // [16:23] Affinity level 2
            uint64_t Aff3   : 8;  // [24:31] Affinity level 3
            uint64_t _rsvd1 : 7;  // [32:38]
            uint64_t U      : 1;  // [39]    Uniprocessor non-affine
            uint64_t _rsvd2 : 24; // [40:63]
        };
    };
    
    MPIDR_EL1() : value(0) {}
    explicit MPIDR_EL1(uint64_t val) : value(val) {}
    
    // Default: single core, Aff0=0
    static constexpr uint64_t RESET_VALUE = 0x80000000ULL;
};

/**
 * @brief MIDR_EL1 - Main ID Register
 */
struct MIDR_EL1 {
    uint64_t value;
    MIDR_EL1() : value(0) {}
    explicit MIDR_EL1(uint64_t val) : value(val) {}
    
    // Example: Apple M1-like
    static constexpr uint64_t APPLE_M1 = 0x610F0220ULL;
};

// ============================================================================
// ARM64 System Register State Container
// ============================================================================

/**
 * @brief Complete ARM64 system register state for EL1
 * 
 * This structure holds all emulated system registers for a single vCPU.
 * Used by hypervisors to maintain guest CPU state.
 */
struct ARM64SystemRegisterState {
    // System Control
    SCTLR_EL1 SCTLR_EL1;
    TCR_EL1   TCR_EL1;
    TTBR0_EL1 TTBR0_EL1;
    TTBR1_EL1 TTBR1_EL1;
    
    // Exception handling
    ESR_EL1  ESR_EL1;
    FAR_EL1  FAR_EL1;
    ELR_EL1  ELR_EL1;
    VBAR_EL1 VBAR_EL1;
    
    // CPU identification
    MPIDR_EL1 MPIDR_EL1;
    MIDR_EL1  MIDR_EL1;
    
    // Reset all registers to default values
    void reset() {
        SCTLR_EL1.value = SCTLR_EL1::RESET_VALUE;
        TCR_EL1.value   = TCR_EL1::RESET_VALUE;
        TTBR0_EL1.value = 0;
        TTBR1_EL1.value = 0;
        ESR_EL1.value   = 0;
        FAR_EL1.value   = 0;
        ELR_EL1.value   = 0;
        VBAR_EL1.value  = 0x0;  // Will be set by guest
        MPIDR_EL1.value = MPIDR_EL1::RESET_VALUE;
        MIDR_EL1.value  = MIDR_EL1::APPLE_M1;
    }
    
    ARM64SystemRegisterState() { reset(); }
};

// ============================================================================
// System Instruction Decoder
// ============================================================================

/**
 * @brief Decoded ARM64 system register instruction
 */
struct SystemInstruction {
    enum class Type {
        UNKNOWN,
        MSR_IMM,      // MSR <sysreg>, #imm
        MSR_REG,      // MSR <sysreg>, <Xn>
        MRS_REG,      // MRS <Xn>, <sysreg>
        ISB,
        DSB,
        DMB,
        WFI,
        WFE,
        HLT,
        BRK,
    };
    
    Type type;
    uint64_t sysRegOp0;  // op0 field (for encoding)
    uint64_t sysRegOp1;  // op1 field
    uint64_t sysRegCRn;  // CRn field
    uint64_t sysRegCRm;  // CRm field
    uint64_t sysRegOp2;  // op2 field
    uint8_t  rt;         // Target register (Xn) or immediate
    uint64_t immediate;  // For MSR_IMM
    
    SystemInstruction() : type(Type::UNKNOWN), sysRegOp0(0), sysRegOp1(0), 
                          sysRegCRn(0), sysRegCRm(0), sysRegOp2(0), rt(0), immediate(0) {}
};

/**
 * @brief Decode ARM64 system instruction
 * 
 * @param instruction 32-bit ARM64 instruction
 * @return Decoded system instruction or UNKNOWN
 */
SystemInstruction decodeSystemInstruction(uint32_t instruction);

/**
 * @brief Get system register name from encoding
 */
std::string getSystemRegisterName(uint8_t op0, uint8_t op1, uint8_t crn, uint8_t crm, uint8_t op2);

// ============================================================================
// System Register Emulator
// ============================================================================

/**
 * @brief Emulates ARM64 system register access
 * 
 * This class provides the core emulation logic for trapped system instructions.
 * It can be used by any hypervisor backend (HVF, WHP, KVM, or pure JIT).
 */
class SystemRegisterEmulator {
public:
    SystemRegisterEmulator();
    
    /**
     * @brief Get current system register state
     */
    ARM64SystemRegisterState& getState() { return state_; }
    const ARM64SystemRegisterState& getState() const { return state_; }
    
    /**
     * @brief Emulate a trapped MSR instruction
     * 
     * @param instruction The decoded system instruction
     * @param getRegValue Callback to read general purpose register value
     * @param setRegValue Callback to write general purpose register value
     * @return true if instruction was emulated, false if unknown
     */
    bool emulateMSR(const SystemInstruction& instruction,
                    std::function<uint64_t(uint8_t)> getRegValue,
                    std::function<void(uint8_t, uint64_t)> setRegValue);
    
    /**
     * @brief Emulate a trapped MRS instruction
     */
    bool emulateMRS(const SystemInstruction& instruction,
                    std::function<uint64_t(uint8_t)> getRegValue,
                    std::function<void(uint8_t, uint64_t)> setRegValue);
    
    /**
     * @brief Handle exception entry
     * 
     * Updates ELR_EL1, ESR_EL1, and advances PC to exception vector.
     */
    void handleExceptionEntry(uint64_t currentPC, uint64_t esr, uint64_t far = 0);
    
    /**
     * @brief Handle exception return (ERET)
     * 
     * Returns PC from ELR_EL1.
     */
    uint64_t handleExceptionReturn() const;
    
    /**
     * @brief Read a system register by encoding
     */
    std::optional<uint64_t> readSystemRegister(uint8_t op0, uint8_t op1, uint8_t crn, 
                                                uint8_t crm, uint8_t op2) const;
    
    /**
     * @brief Write a system register by encoding
     */
    bool writeSystemRegister(uint8_t op0, uint8_t op1, uint8_t crn, 
                             uint8_t crm, uint8_t op2, uint64_t value);

private:
    ARM64SystemRegisterState state_;
    
    // Helper to extract system register fields from instruction
    static void extractSysRegFields(uint32_t instruction,
                                    uint8_t& op0, uint8_t& op1, uint8_t& crn,
                                    uint8_t& crm, uint8_t& op2, uint8_t& rt);
};

} // namespace acheronarm64
