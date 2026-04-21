// test_emulator.cpp - Unit tests for System Register Emulator
#include <gtest/gtest.h>
#include "acheronarm64/ARM64SystemRegisters.hpp"

using namespace acheronarm64;

// ============================================================================
// Emulator State Tests
// ============================================================================

TEST(EmulatorTest, ResetState) {
    SystemRegisterEmulator emulator;
    const auto& state = emulator.getState();
    
    // Check reset values
    ASSERT_EQ(state.SCTLR_EL1.value, SCTLR_EL1::RESET_VALUE);
    ASSERT_EQ(state.TCR_EL1.value, TCR_EL1::RESET_VALUE);
    ASSERT_EQ(state.MPIDR_EL1.value, MPIDR_EL1::RESET_VALUE);
    ASSERT_EQ(state.MIDR_EL1.value, MIDR_EL1::APPLE_M1);
}

TEST(EmulatorTest, Reset ClearsAllRegisters) {
    SystemRegisterEmulator emulator;
    
    // Modify some registers
    emulator.getState().SCTLR_EL1.value = 0xDEADBEEF;
    emulator.getState().TCR_EL1.value = 0xCAFEBABE;
    emulator.getState().TTBR0_EL1.value = 0x12345678;
    
    // Reset
    emulator.getState().reset();
    
    // Verify reset
    ASSERT_EQ(emulator.getState().SCTLR_EL1.value, SCTLR_EL1::RESET_VALUE);
    ASSERT_EQ(emulator.getState().TCR_EL1.value, TCR_EL1::RESET_VALUE);
    ASSERT_EQ(emulator.getState().TTBR0_EL1.value, 0);
}

// ============================================================================
// MSR Emulation Tests
// ============================================================================

TEST(EmulatorTest, EmulateMSR_SCTLR_EL1) {
    SystemRegisterEmulator emulator;
    
    // Create MSR SCTLR_EL1, X0 instruction
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MSR_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0001;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b000;
    instr.rt = 0;
    
    // X0 = 0x00C50838 (MMU disabled, safe defaults)
    uint64_t x0_value = 0x00C50838ULL;
    auto getReg = [&](uint8_t r) { return (r == 0) ? x0_value : 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { /* ignore for MSR */ };
    
    bool result = emulator.emulateMSR(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(emulator.getState().SCTLR_EL1.value, 0x00C50838ULL);
    
    // Verify individual fields
    ASSERT_EQ(emulator.getState().SCTLR_EL1.M, 0);  // MMU disabled
    ASSERT_EQ(emulator.getState().SCTLR_EL1.C, 0);  // Cache disabled
    ASSERT_EQ(emulator.getState().SCTLR_EL1.I, 0);  // I-cache disabled
}

TEST(EmulatorTest, EmulateMSR_TCR_EL1) {
    SystemRegisterEmulator emulator;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MSR_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0010;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b010;
    instr.rt = 0;
    
    uint64_t x0_value = 0x00000000ULL;
    auto getReg = [&](uint8_t r) { return (r == 0) ? x0_value : 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { };
    
    bool result = emulator.emulateMSR(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(emulator.getState().TCR_EL1.value, 0x00000000ULL);
}

TEST(EmulatorTest, EmulateMSR_TTBR0_EL1) {
    SystemRegisterEmulator emulator;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MSR_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0010;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b000;
    instr.rt = 1;
    
    uint64_t x1_value = 0x0000000040000000ULL;  // 1GB aligned
    auto getReg = [&](uint8_t r) { return (r == 1) ? x1_value : 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { };
    
    bool result = emulator.emulateMSR(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(emulator.getState().TTBR0_EL1.value, 0x0000000040000000ULL);
    ASSERT_EQ(emulator.getState().TTBR0_EL1.getBaseAddress(), 0x0000000040000000ULL);
}

TEST(EmulatorTest, EmulateMSR_TTBR1_EL1) {
    SystemRegisterEmulator emulator;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MSR_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0010;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b001;
    instr.rt = 2;
    
    uint64_t x2_value = 0x0000000100000000ULL;
    auto getReg = [&](uint8_t r) { return (r == 2) ? x2_value : 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { };
    
    bool result = emulator.emulateMSR(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(emulator.getState().TTBR1_EL1.value, 0x0000000100000000ULL);
}

TEST(EmulatorTest, EmulateMSR_VBAR_EL1) {
    SystemRegisterEmulator emulator;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MSR_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b1100;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b000;
    instr.rt = 3;
    
    uint64_t x3_value = 0x0000000000100000ULL;  // Exception vectors at 1MB
    auto getReg = [&](uint8_t r) { return (r == 3) ? x3_value : 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { };
    
    bool result = emulator.emulateMSR(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(emulator.getState().VBAR_EL1.value, 0x0000000000100000ULL);
}

TEST(EmulatorTest, EmulateMSR_Immediate) {
    SystemRegisterEmulator emulator;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MSR_IMM;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0001;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b000;
    instr.immediate = 0x0;  // MSR SCTLR_EL1, #0
    
    auto getReg = [&](uint8_t r) { return 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { };
    
    bool result = emulator.emulateMSR(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(emulator.getState().SCTLR_EL1.value, 0x0);
}

// ============================================================================
// MRS Emulation Tests
// ============================================================================

TEST(EmulatorTest, EmulateMRS_SCTLR_EL1) {
    SystemRegisterEmulator emulator;
    
    // Set SCTLR_EL1 to known value
    emulator.getState().SCTLR_EL1.value = 0x00C50838ULL;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MRS_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0001;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b000;
    instr.rt = 0;
    
    uint64_t written_value = 0;
    auto getReg = [&](uint8_t r) { return 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { 
        if (r == 0) written_value = v;
    };
    
    bool result = emulator.emulateMRS(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(written_value, 0x00C50838ULL);
}

TEST(EmulatorTest, EmulateMRS_MPIDR_EL1) {
    SystemRegisterEmulator emulator;
    
    uint64_t written_value = 0;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MRS_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0000;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b101;
    instr.rt = 0;
    
    auto getReg = [&](uint8_t r) { return 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { 
        if (r == 0) written_value = v;
    };
    
    bool result = emulator.emulateMRS(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(written_value, MPIDR_EL1::RESET_VALUE);
}

TEST(EmulatorTest, EmulateMRS_MIDR_EL1) {
    SystemRegisterEmulator emulator;
    
    uint64_t written_value = 0;
    
    SystemInstruction instr;
    instr.type = SystemInstruction::Type::MRS_REG;
    instr.sysRegOp0 = 0b11;
    instr.sysRegOp1 = 0b000;
    instr.sysRegCRn = 0b0000;
    instr.sysRegCRm = 0b0000;
    instr.sysRegOp2 = 0b000;
    instr.rt = 1;
    
    auto getReg = [&](uint8_t r) { return 0; };
    auto setReg = [&](uint8_t r, uint64_t v) { 
        if (r == 1) written_value = v;
    };
    
    bool result = emulator.emulateMRS(instr, getReg, setReg);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(written_value, MIDR_EL1::APPLE_M1);
}

// ============================================================================
// Exception Handling Tests
// ============================================================================

TEST(EmulatorTest, ExceptionEntry) {
    SystemRegisterEmulator emulator;
    
    // Simulate exception entry
    uint64_t faulting_pc = 0x80000000ULL;
    uint64_t esr = 0x96000000;  // Data Abort
    uint64_t far = 0xDEADBEEF;
    
    emulator.handleExceptionEntry(faulting_pc, esr, far);
    
    ASSERT_EQ(emulator.getState().ELR_EL1.value, faulting_pc);
    ASSERT_EQ(emulator.getState().ESR_EL1.value, esr);
    ASSERT_EQ(emulator.getState().FAR_EL1.value, far);
}

TEST(EmulatorTest, ExceptionReturn) {
    SystemRegisterEmulator emulator;
    
    // Set up exception return
    emulator.getState().ELR_EL1.value = 0x80000100ULL;
    
    uint64_t return_pc = emulator.handleExceptionReturn();
    
    ASSERT_EQ(return_pc, 0x80000100ULL);
}

// ============================================================================
// Direct Register Access Tests
// ============================================================================

TEST(EmulatorTest, ReadSystemRegister_SCTLR_EL1) {
    SystemRegisterEmulator emulator;
    emulator.getState().SCTLR_EL1.value = 0x00C50838ULL;
    
    auto value = emulator.readSystemRegister(0b11, 0b000, 0b0001, 0b0000, 0b000);
    
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value.value(), 0x00C50838ULL);
}

TEST(EmulatorTest, ReadSystemRegister_Unknown) {
    SystemRegisterEmulator emulator;
    
    // Unknown register encoding
    auto value = emulator.readSystemRegister(0b00, 0b000, 0b0000, 0b0000, 0b000);
    
    ASSERT_FALSE(value.has_value());
}

TEST(EmulatorTest, WriteSystemRegister_SCTLR_EL1) {
    SystemRegisterEmulator emulator;
    
    bool result = emulator.writeSystemRegister(0b11, 0b000, 0b0001, 0b0000, 0b000, 0xDEADBEEF);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(emulator.getState().SCTLR_EL1.value, 0xDEADBEEF);
}

TEST(EmulatorTest, WriteSystemRegister_AllRegisters) {
    SystemRegisterEmulator emulator;
    
    // Write all supported registers
    emulator.writeSystemRegister(0b11, 0b000, 0b0001, 0b0000, 0b000, 0x11111111);  // SCTLR_EL1
    emulator.writeSystemRegister(0b11, 0b000, 0b0010, 0b0000, 0b010, 0x22222222);  // TCR_EL1
    emulator.writeSystemRegister(0b11, 0b000, 0b0010, 0b0000, 0b000, 0x33333333);  // TTBR0_EL1
    emulator.writeSystemRegister(0b11, 0b000, 0b0010, 0b0000, 0b001, 0x44444444);  // TTBR1_EL1
    emulator.writeSystemRegister(0b11, 0b000, 0b1100, 0b0000, 0b000, 0x55555555);  // VBAR_EL1
    
    ASSERT_EQ(emulator.getState().SCTLR_EL1.value, 0x11111111);
    ASSERT_EQ(emulator.getState().TCR_EL1.value, 0x22222222);
    ASSERT_EQ(emulator.getState().TTBR0_EL1.value, 0x33333333);
    ASSERT_EQ(emulator.getState().TTBR1_EL1.value, 0x44444444);
    ASSERT_EQ(emulator.getState().VBAR_EL1.value, 0x55555555);
}

// ============================================================================
// Field Access Tests
// ============================================================================

TEST(EmulatorTest, SCTLR_EL1_FieldAccess) {
    SystemRegisterEmulator emulator;
    
    // Test individual field access
    SCTLR_EL1 sctlr;
    sctlr.value = 0x00C50838ULL;
    
    ASSERT_EQ(sctlr.M, 0);   // Bit 0: MMU disabled
    ASSERT_EQ(sctlr.A, 0);   // Bit 1: Alignment check disabled
    ASSERT_EQ(sctlr.C, 0);   // Bit 2: Data cache disabled
    ASSERT_EQ(sctlr.I, 0);   // Bit 12: I-cache disabled
    ASSERT_EQ(sctlr.nTWE, 1);  // Bit 17: Trap WFE enabled
    ASSERT_EQ(sctlr.nTWI, 1);  // Bit 19: Trap WFI enabled
}

TEST(EmulatorTest, TCR_EL1_FieldAccess) {
    SystemRegisterEmulator emulator;
    
    TCR_EL1 tcr;
    tcr.value = 0x00000000ULL;
    
    ASSERT_EQ(tcr.T0SZ, 0);  // Bits [5:0]: Size offset
    ASSERT_EQ(tcr.IRGN0, 0); // Bits [9:8]: Inner cacheability
    ASSERT_EQ(tcr.ORGN0, 0); // Bits [11:10]: Outer cacheability
    ASSERT_EQ(tcr.SH0, 0);   // Bits [13:12]: Shareability
    ASSERT_EQ(tcr.TG0, 0);   // Bits [15:14]: Granule
}

TEST(EmulatorTest, TTBR0_EL1_BaseAddress) {
    TTBR0_EL1 ttbr;
    
    // Test 4KB aligned address
    ttbr.setBaseAddress(0x0000000040000000ULL);
    ASSERT_EQ(ttbr.getBaseAddress(), 0x0000000040000000ULL);
    
    // Test alignment (lower 12 bits should be cleared)
    ttbr.setBaseAddress(0x0000000040000123ULL);
    ASSERT_EQ(ttbr.getBaseAddress(), 0x0000000040000000ULL);
}

TEST(EmulatorTest, MPIDR_EL1_Affinity) {
    MPIDR_EL1 mpidr;
    mpidr.value = 0x80000000ULL;
    
    ASSERT_EQ(mpidr.Aff0, 0);
    ASSERT_EQ(mpidr.Aff1, 0);
    ASSERT_EQ(mpidr.Aff2, 0);
    ASSERT_EQ(mpidr.Aff3, 0);
    ASSERT_EQ(mpidr.U, 1);  // Uniprocessor non-affine
}
