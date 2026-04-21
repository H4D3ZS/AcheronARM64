// basic_usage.cpp - Basic AcheronARM64 usage example

#include <iostream>
#include <cassert>
#include "acheronarm64/ARM64SystemRegisters.hpp"

using namespace acheronarm64;

int main() {
    std::cout << "=== AcheronARM64 Basic Usage Example ===" << std::endl;
    
    // 1. Create emulator instance
    SystemRegisterEmulator emulator;
    
    std::cout << "\n1. Initial state (after reset):" << std::endl;
    std::cout << "   SCTLR_EL1 = 0x" << std::hex << emulator.getState().SCTLR_EL1.value << std::endl;
    std::cout << "   TCR_EL1   = 0x" << emulator.getState().TCR_EL1.value << std::endl;
    std::cout << "   MIDR_EL1  = 0x" << emulator.getState().MIDR_EL1.value << std::endl;
    
    // 2. Emulate: MSR SCTLR_EL1, X0 (where X0 = 0x00C50838)
    std::cout << "\n2. Emulating: MSR SCTLR_EL1, X0 (X0 = 0x00C50838)" << std::endl;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MSR_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0001;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b000;
    instr.rt = 0;
    
    uint64_t x0_value = 0x00C50838ULL;
    auto getReg = [&](uint8_t r) { return (r == 0) ? x0_value : 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { /* ignore for MSR */ };
    
    bool success = emulator.emulateMSR(instr, getReg, setReg);
    assert(success);
    
    std::cout << "   SCTLR_EL1 = 0x" << emulator.getState().SCTLR_EL1.value << std::endl;
    std::cout << "   ✓ Emulation successful!" << std::endl;
    
    // 3. Verify individual fields
    std::cout << "\n3. SCTLR_EL1 field values:" << std::endl;
    std::cout << "   M (MMU)     = " << emulator.getState().SCTLR_EL1.M << " (disabled)" << std::endl;
    std::cout << "   C (D-Cache) = " << emulator.getState().SCTLR_EL1.C << " (disabled)" << std::endl;
    std::cout << "   I (I-Cache) = " << emulator.getState().SCTLR_EL1.I << " (disabled)" << std::endl;
    
    // 4. Emulate: MRS X0, MPIDR_EL1
    std::cout << "\n4. Emulating: MRS X0, MPIDR_EL1" << std::endl;
    
    instr.type = SystemInstruction::Type::MRS_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0000;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b101;
    instr.rt = 0;
    
    uint64_t x0_result = 0;
    auto getReg2 = [&](uint8_t r) { return 0; };
    auto setReg2 = [&](uint8_t r, uint64_t v) { 
        if (r == 0) x0_result = v;
    };
    
    success = emulator.emulateMRS(instr, getReg2, setReg2);
    assert(success);
    
    std::cout << "   X0 = 0x" << x0_result << " (CPU affinity)" << std::endl;
    std::cout << "   ✓ Emulation successful!" << std::endl;
    
    // 5. Direct register access
    std::cout << "\n5. Direct register access:" << std::endl;
    emulator.getState().TTBR0_EL1.value = 0x0000000040000000ULL;
    std::cout << "   TTBR0_EL1 = 0x" << emulator.getState().TTBR0_EL1.value << std::endl;
    std::cout << "   Base addr = 0x" << emulator.getState().TTBR0_EL1.getBaseAddress() << std::endl;
    
    std::cout << "\n=== Example Complete ===" << std::endl;
    return 0;
}
