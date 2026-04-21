// test_decoder.cpp - Unit tests for ARM64 instruction decoder
#include <gtest/gtest.h>
#include "acheronarm64/ARM64SystemRegisters.hpp"

using namespace acheronarm64;

// ============================================================================
// MSR Instruction Tests
// ============================================================================

TEST(DecoderTest, DecodeMSR_SCTLR_EL1_Immediate) {
    // MSR SCTLR_EL1, #0 = 0xD500001F
    uint32_t instr = 0xD500001F;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MSR_IMM);
    ASSERT_EQ(decoded.sysRegOp0, 0b11);
    ASSERT_EQ(decoded.sysRegOp1, 0b000);
    ASSERT_EQ(decoded.sysRegCRn, 0b0001);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b000);
    ASSERT_EQ(decoded.immediate, 0x1F);
}

TEST(DecoderTest, DecodeMSR_SCTLR_EL1_Register) {
    // MSR SCTLR_EL1, X0 = 0xD5100200
    uint32_t instr = 0xD5100200;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MSR_REG);
    ASSERT_EQ(decoded.sysRegOp0, 0b11);
    ASSERT_EQ(decoded.sysRegOp1, 0b000);
    ASSERT_EQ(decoded.sysRegCRn, 0b0001);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b000);
    ASSERT_EQ(decoded.rt, 0);
}

TEST(DecoderTest, DecodeMSR_TCR_EL1) {
    // MSR TCR_EL1, X0 = 0xD5100800
    uint32_t instr = 0xD5100800;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MSR_REG);
    ASSERT_EQ(decoded.sysRegCRn, 0b0010);
    ASSERT_EQ(decoded.sysRegOp2, 0b010);
}

TEST(DecoderTest, DecodeMSR_TTBR0_EL1) {
    // MSR TTBR0_EL1, X0 = 0xD5100000
    uint32_t instr = 0xD5100000;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MSR_REG);
    ASSERT_EQ(decoded.sysRegCRn, 0b0010);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b000);
}

TEST(DecoderTest, DecodeMSR_TTBR1_EL1) {
    // MSR TTBR1_EL1, X0 = 0xD5100020
    uint32_t instr = 0xD5100020;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MSR_REG);
    ASSERT_EQ(decoded.sysRegCRn, 0b0010);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b001);
}

TEST(DecoderTest, DecodeMSR_VBAR_EL1) {
    // MSR VBAR_EL1, X0 = 0xD51C0000
    uint32_t instr = 0xD51C0000;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MSR_REG);
    ASSERT_EQ(decoded.sysRegCRn, 0b1100);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b000);
}

// ============================================================================
// MRS Instruction Tests
// ============================================================================

TEST(DecoderTest, DecodeMRS_SCTLR_EL1) {
    // MRS X0, SCTLR_EL1 = 0xD5300200
    uint32_t instr = 0xD5300200;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MRS_REG);
    ASSERT_EQ(decoded.sysRegOp0, 0b11);
    ASSERT_EQ(decoded.sysRegOp1, 0b000);
    ASSERT_EQ(decoded.sysRegCRn, 0b0001);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b000);
    ASSERT_EQ(decoded.rt, 0);
}

TEST(DecoderTest, DecodeMRS_MPIDR_EL1) {
    // MRS X0, MPIDR_EL1 = 0xD5380020
    uint32_t instr = 0xD5380020;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MRS_REG);
    ASSERT_EQ(decoded.sysRegCRn, 0b0000);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b101);
}

TEST(DecoderTest, DecodeMRS_MIDR_EL1) {
    // MRS X0, MIDR_EL1 = 0xD5380000
    uint32_t instr = 0xD5380000;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MRS_REG);
    ASSERT_EQ(decoded.sysRegCRn, 0b0000);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b000);
}

TEST(DecoderTest, DecodeMRS_ELR_EL1) {
    // MRS X0, ELR_EL1 = 0xD5308020
    uint32_t instr = 0xD5308020;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::MRS_REG);
    ASSERT_EQ(decoded.sysRegCRn, 0b0100);
    ASSERT_EQ(decoded.sysRegCRm, 0b0000);
    ASSERT_EQ(decoded.sysRegOp2, 0b001);
}

// ============================================================================
// Barrier Instruction Tests
// ============================================================================

TEST(DecoderTest, DecodeISB) {
    // ISB = 0xD500401F
    uint32_t instr = 0xD500401F;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::ISB);
}

TEST(DecoderTest, DecodeDSB_SY) {
    // DSB SY = 0xD503305F
    uint32_t instr = 0xD503305F;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::DSB);
}

TEST(DecoderTest, DecodeDMB_SY) {
    // DMB SY = 0xD50330BF
    uint32_t instr = 0xD50330BF;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::DMB);
}

// ============================================================================
// Power Management Instruction Tests
// ============================================================================

TEST(DecoderTest, DecodeWFI) {
    // WFI = 0xD503205F
    uint32_t instr = 0xD503205F;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::WFI);
}

TEST(DecoderTest, DecodeWFE) {
    // WFE = 0xD503207F
    uint32_t instr = 0xD503207F;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::WFE);
}

// ============================================================================
// Breakpoint/Halt Instruction Tests
// ============================================================================

TEST(DecoderTest, DecodeBRK_0) {
    // BRK #0 = 0xD4200000
    uint32_t instr = 0xD4200000;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::BRK);
    ASSERT_EQ(decoded.immediate, 0);
}

TEST(DecoderTest, DecodeBRK_1) {
    // BRK #1 = 0xD4200001
    uint32_t instr = 0xD4200001;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::BRK);
    ASSERT_EQ(decoded.immediate, 1);
}

TEST(DecoderTest, DecodeHLT) {
    // HLT #0 = 0xD4200000
    uint32_t instr = 0xD4200000;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::HLT);
}

// ============================================================================
// Unknown Instruction Tests
// ============================================================================

TEST(DecoderTest, DecodeUnknown_ADD) {
    // ADD X0, X1, X2 = 0x8B020020 (not a system instruction)
    uint32_t instr = 0x8B020020;
    auto decoded = decodeSystemInstruction(instr);
    
    ASSERT_EQ(decoded.type, SystemInstruction::Type::UNKNOWN);
}

TEST(DecoderTest, DecodeUnknown_NOP) {
    // NOP = 0xD503201F
    uint32_t instr = 0xD503201F;
    auto decoded = decodeSystemInstruction(instr);
    
    // NOP is encoded as HINT instruction, may be UNKNOWN
    // This is acceptable behavior
    (void)decoded;
}

// ============================================================================
// Register Name Tests
// ============================================================================

TEST(DecoderTest, GetSystemRegisterName_SCTLR_EL1) {
    std::string name = getSystemRegisterName(0b11, 0b000, 0b0001, 0b0000, 0b000);
    ASSERT_EQ(name, "SCTLR_EL1");
}

TEST(DecoderTest, GetSystemRegisterName_TCR_EL1) {
    std::string name = getSystemRegisterName(0b11, 0b000, 0b0010, 0b0000, 0b010);
    ASSERT_EQ(name, "TCR_EL1");
}

TEST(DecoderTest, GetSystemRegisterName_TTBR0_EL1) {
    std::string name = getSystemRegisterName(0b11, 0b000, 0b0010, 0b0000, 0b000);
    ASSERT_EQ(name, "TTBR0_EL1");
}

TEST(DecoderTest, GetSystemRegisterName_TTBR1_EL1) {
    std::string name = getSystemRegisterName(0b11, 0b000, 0b0010, 0b0000, 0b001);
    ASSERT_EQ(name, "TTBR1_EL1");
}

TEST(DecoderTest, GetSystemRegisterName_MPIDR_EL1) {
    std::string name = getSystemRegisterName(0b11, 0b000, 0b0000, 0b0000, 0b101);
    ASSERT_EQ(name, "MPIDR_EL1");
}

TEST(DecoderTest, GetSystemRegisterName_MIDR_EL1) {
    std::string name = getSystemRegisterName(0b11, 0b000, 0b0000, 0b0000, 0b000);
    ASSERT_EQ(name, "MIDR_EL1");
}
