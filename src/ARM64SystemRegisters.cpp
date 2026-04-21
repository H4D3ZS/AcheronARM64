// ARM64SystemRegisters.cpp
// Cross-platform ARM64 system register emulation
// Implementation

#include "ARM64SystemRegisters.hpp"
#include <iostream>
#include <cstring>

namespace acheronarm64 {

// ============================================================================
// System Instruction Decoder
// ============================================================================

SystemInstruction decodeSystemInstruction(uint32_t instruction) {
    SystemInstruction result;
    
    // ARM64 instruction format (32-bit):
    // [31:26] opcode
    // [25:24] op2
    // [23:22] op1
    // [21:20] op0
    // [19:16] CRn
    // [15:12] Rt
    // [11:8]  CRm
    // [7:5]   op2 (for MRS/MSR)
    // [4:0]   op1 (for MRS/MSR)
    
    uint32_t opcode = (instruction >> 26) & 0x3F;
    
    // MSR (immediate) - 0xD5000000 with specific pattern
    // D500 0000: MSR <system register>, #imm
    if ((instruction & 0xFFE0001F) == 0xD5000000) {
        result.type = SystemInstruction::Type::MSR_IMM;
        result.sysRegOp0 = (instruction >> 19) & 0x7;
        result.sysRegOp1 = (instruction >> 16) & 0x7;
        result.sysRegCRn = (instruction >> 12) & 0xF;
        result.sysRegCRm = (instruction >> 8) & 0xF;
        result.sysRegOp2 = (instruction >> 5) & 0x7;
        result.immediate = (instruction >> 0) & 0x1F;
        return result;
    }
    
    // MSR (register) - 0xD5100000
    // D510 0000: MSR <system register>, <Xt>
    if ((instruction & 0xFFE00000) == 0xD5100000) {
        result.type = SystemInstruction::Type::MSR_REG;
        result.rt = (instruction >> 5) & 0x1F;
        result.sysRegOp0 = (instruction >> 19) & 0x7;
        result.sysRegOp1 = (instruction >> 16) & 0x7;
        result.sysRegCRn = (instruction >> 12) & 0xF;
        result.sysRegCRm = (instruction >> 8) & 0xF;
        result.sysRegOp2 = (instruction >> 5) & 0x7;
        return result;
    }
    
    // MRS - 0xD5300000
    // D530 0000: MRS <Xt>, <system register>
    if ((instruction & 0xFFE00000) == 0xD5300000) {
        result.type = SystemInstruction::Type::MRS_REG;
        result.rt = (instruction >> 5) & 0x1F;
        result.sysRegOp0 = (instruction >> 19) & 0x7;
        result.sysRegOp1 = (instruction >> 16) & 0x7;
        result.sysRegCRn = (instruction >> 12) & 0xF;
        result.sysRegCRm = (instruction >> 8) & 0xF;
        result.sysRegOp2 = (instruction >> 5) & 0x7;
        return result;
    }
    
    // ISB - 0xD500401F
    if (instruction == 0xD500401F) {
        result.type = SystemInstruction::Type::ISB;
        return result;
    }
    
    // DSB - 0xD5033000 | op2
    if ((instruction & 0xFFFFF01F) == 0xD5033000) {
        result.type = SystemInstruction::Type::DSB;
        return result;
    }
    
    // DMB - 0xD5033000 | op2 (different encoding)
    if ((instruction & 0xFFFFF01F) == 0xD5033000) {
        result.type = SystemInstruction::Type::DMB;
        return result;
    }
    
    // WFI - 0xD503205F
    if (instruction == 0xD503205F) {
        result.type = SystemInstruction::Type::WFI;
        return result;
    }
    
    // WFE - 0xD503207F
    if (instruction == 0xD503207F) {
        result.type = SystemInstruction::Type::WFE;
        return result;
    }
    
    // HLT - 0xD4200000
    if ((instruction & 0xFFE0001F) == 0xD4200000) {
        result.type = SystemInstruction::Type::HLT;
        result.immediate = (instruction >> 0) & 0xFFFF;
        return result;
    }
    
    // BRK - 0xD4200000
    if ((instruction & 0xFFE0001F) == 0xD4200000) {
        result.type = SystemInstruction::Type::BRK;
        result.immediate = (instruction >> 0) & 0xFFFF;
        return result;
    }
    
    result.type = SystemInstruction::Type::UNKNOWN;
    return result;
}

std::string getSystemRegisterName(uint8_t op0, uint8_t op1, uint8_t crn, 
                                   uint8_t crm, uint8_t op2) {
    // Common system registers for EL1
    // Reference: ARM ARM Table C5-1
    
    // SCTLR_EL1: op0=0b11, op1=0b000, CRn=0b0001, CRm=0b0000, op2=0b000
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0001 && crm == 0b0000 && op2 == 0b000)
        return "SCTLR_EL1";
    
    // TCR_EL1: op0=0b11, op1=0b000, CRn=0b0010, CRm=0b0000, op2=0b010
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b010)
        return "TCR_EL1";
    
    // TTBR0_EL1: op0=0b11, op1=0b000, CRn=0b0010, CRm=0b0000, op2=0b000
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b000)
        return "TTBR0_EL1";
    
    // TTBR1_EL1: op0=0b11, op1=0b000, CRn=0b0010, CRm=0b0000, op2=0b001
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b001)
        return "TTBR1_EL1";
    
    // ESR_EL1: op0=0b11, op1=0b000, CRn=0b0101, CRm=0b0010, op2=0b000
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0101 && crm == 0b0010 && op2 == 0b000)
        return "ESR_EL1";
    
    // FAR_EL1: op0=0b11, op1=0b000, CRn=0b0110, CRm=0b0000, op2=0b000
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0110 && crm == 0b0000 && op2 == 0b000)
        return "FAR_EL1";
    
    // ELR_EL1: op0=0b11, op1=0b000, CRn=0b0100, CRm=0b0000, op2=0b001
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0100 && crm == 0b0000 && op2 == 0b001)
        return "ELR_EL1";
    
    // VBAR_EL1: op0=0b11, op1=0b000, CRn=0b1100, CRm=0b0000, op2=0b000
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b1100 && crm == 0b0000 && op2 == 0b000)
        return "VBAR_EL1";
    
    // MPIDR_EL1: op0=0b11, op1=0b000, CRn=0b0000, CRm=0b0000, op2=0b101
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0000 && crm == 0b0000 && op2 == 0b101)
        return "MPIDR_EL1";
    
    // MIDR_EL1: op0=0b11, op1=0b000, CRn=0b0000, CRm=0b0000, op2=0b000
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0000 && crm == 0b0000 && op2 == 0b000)
        return "MIDR_EL1";
    
    // SCTLR_EL2: op0=0b11, op1=0b100, CRn=0b0001, CRm=0b0000, op2=0b000
    if (op0 == 0b11 && op1 == 0b100 && crn == 0b0001 && crm == 0b0000 && op2 == 0b000)
        return "SCTLR_EL2";
    
    // HCR_EL2: op0=0b11, op1=0b100, CRn=0b0001, CRm=0b0001, op2=0b000
    if (op0 == 0b11 && op1 == 0b100 && crn == 0b0001 && crm == 0b0001 && op2 == 0b000)
        return "HCR_EL2";
    
    // Unknown
    char buf[64];
    snprintf(buf, sizeof(buf), "S%d_%d_C%d_C%d_%d", op0, op1, crn, crm, op2);
    return std::string(buf);
}

// ============================================================================
// System Register Emulator
// ============================================================================

SystemRegisterEmulator::SystemRegisterEmulator() {
    state_.reset();
}

void SystemRegisterEmulator::extractSysRegFields(uint32_t instruction,
                                                  uint8_t& op0, uint8_t& op1, uint8_t& crn,
                                                  uint8_t& crm, uint8_t& op2, uint8_t& rt) {
    // Extract fields from MSR/MRS instruction
    op0 = (instruction >> 19) & 0x7;
    op1 = (instruction >> 16) & 0x7;
    crn = (instruction >> 12) & 0xF;
    crm = (instruction >> 8) & 0xF;
    op2 = (instruction >> 5) & 0x7;
    rt = (instruction >> 5) & 0x1F;  // For register variants
}

bool SystemRegisterEmulator::emulateMSR(const SystemInstruction& instruction,
                                         std::function<uint64_t(uint8_t)> getRegValue,
                                         std::function<void(uint8_t, uint64_t)> setRegValue) {
    (void)setRegValue;  // MSR doesn't write to GPRs
    
    uint64_t value = 0;
    if (instruction.type == SystemInstruction::Type::MSR_REG) {
        value = getRegValue(instruction.rt);
    } else if (instruction.type == SystemInstruction::Type::MSR_IMM) {
        // Immediate is zero-extended
        value = instruction.immediate;
    } else {
        return false;
    }
    
    // Write to the appropriate system register
    return writeSystemRegister(instruction.sysRegOp0, instruction.sysRegOp1,
                               instruction.sysRegCRn, instruction.sysRegCRm,
                               instruction.sysRegOp2, value);
}

bool SystemRegisterEmulator::emulateMRS(const SystemInstruction& instruction,
                                         std::function<uint64_t(uint8_t)> getRegValue,
                                         std::function<void(uint8_t, uint64_t)> setRegValue) {
    (void)getRegValue;  // MRS doesn't read from GPRs
    
    if (instruction.type != SystemInstruction::Type::MRS_REG) {
        return false;
    }
    
    // Read from the appropriate system register
    auto value = readSystemRegister(instruction.sysRegOp0, instruction.sysRegOp1,
                                    instruction.sysRegCRn, instruction.sysRegCRm,
                                    instruction.sysRegOp2);
    
    if (value.has_value()) {
        setRegValue(instruction.rt, value.value());
        return true;
    }
    
    // Unknown register - return 0
    setRegValue(instruction.rt, 0);
    return true;
}

void SystemRegisterEmulator::handleExceptionEntry(uint64_t currentPC, uint64_t esr, uint64_t far) {
    // Save exception context
    state_.ELR_EL1.value = currentPC;
    state_.ESR_EL1.value = esr;
    state_.FAR_EL1.value = far;
    
    // Note: In a full implementation, we would also:
    // - Save SPSR_EL1
    // - Update PSTATE
    // - Switch to exception level
    // For now, we just record the exception for debugging
}

uint64_t SystemRegisterEmulator::handleExceptionReturn() const {
    // Return to the address in ELR_EL1
    return state_.ELR_EL1.value;
}

std::optional<uint64_t> SystemRegisterEmulator::readSystemRegister(
    uint8_t op0, uint8_t op1, uint8_t crn, uint8_t crm, uint8_t op2) const {
    
    // SCTLR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0001 && crm == 0b0000 && op2 == 0b000)
        return state_.SCTLR_EL1.value;
    
    // TCR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b010)
        return state_.TCR_EL1.value;
    
    // TTBR0_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b000)
        return state_.TTBR0_EL1.value;
    
    // TTBR1_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b001)
        return state_.TTBR1_EL1.value;
    
    // ESR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0101 && crm == 0b0010 && op2 == 0b000)
        return state_.ESR_EL1.value;
    
    // FAR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0110 && crm == 0b0000 && op2 == 0b000)
        return state_.FAR_EL1.value;
    
    // ELR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0100 && crm == 0b0000 && op2 == 0b001)
        return state_.ELR_EL1.value;
    
    // VBAR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b1100 && crm == 0b0000 && op2 == 0b000)
        return state_.VBAR_EL1.value;
    
    // MPIDR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0000 && crm == 0b0000 && op2 == 0b101)
        return state_.MPIDR_EL1.value;
    
    // MIDR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0000 && crm == 0b0000 && op2 == 0b000)
        return state_.MIDR_EL1.value;
    
    // Unknown register
    std::cerr << "[SystemRegisterEmulator] Unknown MRS: " 
              << getSystemRegisterName(op0, op1, crn, crm, op2) << std::endl;
    return std::nullopt;
}

bool SystemRegisterEmulator::writeSystemRegister(
    uint8_t op0, uint8_t op1, uint8_t crn, uint8_t crm, uint8_t op2, uint64_t value) {
    
    // SCTLR_EL1 - MOST CRITICAL for boot
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0001 && crm == 0b0000 && op2 == 0b000) {
        state_.SCTLR_EL1.value = value;
        std::cout << "[SystemRegisterEmulator] MSR SCTLR_EL1 = 0x" << std::hex << value << std::dec << std::endl;
        return true;
    }
    
    // TCR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b010) {
        state_.TCR_EL1.value = value;
        std::cout << "[SystemRegisterEmulator] MSR TCR_EL1 = 0x" << std::hex << value << std::dec << std::endl;
        return true;
    }
    
    // TTBR0_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b000) {
        state_.TTBR0_EL1.value = value;
        std::cout << "[SystemRegisterEmulator] MSR TTBR0_EL1 = 0x" << std::hex << value << std::dec << std::endl;
        return true;
    }
    
    // TTBR1_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0010 && crm == 0b0000 && op2 == 0b001) {
        state_.TTBR1_EL1.value = value;
        std::cout << "[SystemRegisterEmulator] MSR TTBR1_EL1 = 0x" << std::hex << value << std::dec << std::endl;
        return true;
    }
    
    // ESR_EL1 (should be written by HW, but allow for completeness)
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0101 && crm == 0b0010 && op2 == 0b000) {
        state_.ESR_EL1.value = value;
        return true;
    }
    
    // FAR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0110 && crm == 0b0000 && op2 == 0b000) {
        state_.FAR_EL1.value = value;
        return true;
    }
    
    // ELR_EL1 (should be written by HW on exception)
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b0100 && crm == 0b0000 && op2 == 0b001) {
        state_.ELR_EL1.value = value;
        return true;
    }
    
    // VBAR_EL1
    if (op0 == 0b11 && op1 == 0b000 && crn == 0b1100 && crm == 0b0000 && op2 == 0b000) {
        state_.VBAR_EL1.value = value;
        std::cout << "[SystemRegisterEmulator] MSR VBAR_EL1 = 0x" << std::hex << value << std::dec << std::endl;
        return true;
    }
    
    // Unknown register
    std::string regName = getSystemRegisterName(op0, op1, crn, crm, op2);
    std::cout << "[SystemRegisterEmulator] MSR " << regName << " = 0x" << std::hex << value << std::dec << std::endl;
    
    // Store in a generic way for unknown registers (could add a map later)
    // For now, just acknowledge and return true to avoid trapping
    return true;
}

} // namespace acheronarm64
