// kvm_integration.cpp - Linux KVM integration example

#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>
#include "acheronarm64/ARM64SystemRegisters.hpp"

using namespace acheronarm64;

/**
 * Example: Handling KVM system register traps with AcheronARM64
 * 
 * This demonstrates how to integrate AcheronARM64 with Linux KVM
 * to emulate trapped system instructions for ARM64 guests.
 */

class KVMIntegration {
private:
    int kvm_fd;
    int vm_fd;
    int vcpu_fd;
    SystemRegisterEmulator emulator;
    
public:
    KVMIntegration() : kvm_fd(-1), vm_fd(-1), vcpu_fd(-1) {}
    
    bool initialize() {
        std::cout << "[KVM] Opening /dev/kvm..." << std::endl;
        kvm_fd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
        if (kvm_fd < 0) {
            std::cerr << "[KVM] Failed to open /dev/kvm" << std::endl;
            return false;
        }
        
        std::cout << "[KVM] Checking KVM API version..." << std::endl;
        int api_version = ioctl(kvm_fd, KVM_GET_API_VERSION);
        if (api_version != KVM_API_VERSION) {
            std::cerr << "[KVM] API version mismatch" << std::endl;
            return false;
        }
        
        std::cout << "[KVM] Creating VM..." << std::endl;
        vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, 0);
        if (vm_fd < 0) {
            std::cerr << "[KVM] Failed to create VM" << std::endl;
            return false;
        }
        
        std::cout << "[KVM] Creating vCPU..." << std::endl;
        vcpu_fd = ioctl(vm_fd, KVM_CREATE_VCPU, 0);
        if (vcpu_fd < 0) {
            std::cerr << "[KVM] Failed to create vCPU" << std::endl;
            return false;
        }
        
        return true;
    }
    
    void handleSystemRegisterTrap(__u64 pc, struct kvm_run* run) {
        std::cout << "[KVM] System register trap at PC=0x" << std::hex << pc << std::endl;
        
        // Read trapped instruction from guest memory
        __u32 instr_bytes = 0;
        // In real implementation: Read from GPA via KVM_GET_ONE_REG or memory mapping
        
        // Decode instruction
        auto instr = decodeSystemInstruction(instr_bytes);
        
        // Register callbacks
        auto getReg = [this, run](__u8 r) -> __u64 {
            return run->s.regs.regs[r];
        };
        
        auto setReg = [this, run](__u8 r, __u64 v) {
            run->s.regs.regs[r] = v;
        };
        
        // Emulate
        if (instr.type == SystemInstruction::Type::MSR_REG ||
            instr.type == SystemInstruction::Type::MSR_IMM) {
            std::cout << "[KVM] Emulating MSR" << std::endl;
            if (emulator.emulateMSR(instr, getReg, setReg)) {
                run->s.regs.pc += 4;
            }
        } else if (instr.type == SystemInstruction::Type::MRS_REG) {
            std::cout << "[KVM] Emulating MRS" << std::endl;
            emulator.emulateMRS(instr, getReg, setReg);
            run->s.regs.pc += 4;
        } else if (instr.type == SystemInstruction::Type::ISB ||
                   instr.type == SystemInstruction::Type::DSB ||
                   instr.type == SystemInstruction::Type::DMB) {
            std::cout << "[KVM] Skipping barrier" << std::endl;
            run->s.regs.pc += 4;
        } else if (instr.type == SystemInstruction::Type::WFI ||
                   instr.type == SystemInstruction::Type::WFE) {
            std::cout << "[KVM] Skipping WFI/WFE" << std::endl;
            run->s.regs.pc += 4;
        }
    }
    
    void runLoop() {
        std::cout << "[KVM] Starting vCPU run loop..." << std::endl;
        
        struct kvm_run* run;
        size_t mmap_size = ioctl(kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
        run = (struct kvm_run*)mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu_fd, 0);
        
        while (true) {
            int ret = ioctl(vcpu_fd, KVM_RUN, 0);
            
            if (ret == -1 && errno == EINTR) {
                continue;
            }
            
            if (ret == -1) {
                std::cerr << "[KVM] KVM_RUN error: " << strerror(errno) << std::endl;
                break;
            }
            
            switch (run->exit_reason) {
                case KVM_EXIT_EXCEPTION:
                    handleSystemRegisterTrap(run->s.regs.pc, run);
                    break;
                    
                case KVM_EXIT_MMIO:
                    std::cout << "[KVM] MMIO exit" << std::endl;
                    break;
                    
                case KVM_EXIT_INTR:
                    std::cout << "[KVM] Interrupt exit" << std::endl;
                    break;
                    
                case KVM_EXIT_SHUTDOWN:
                    std::cout << "[KVM] Guest shutdown" << std::endl;
                    return;
                    
                default:
                    std::cout << "[KVM] Unknown exit reason: " << run->exit_reason << std::endl;
                    return;
            }
        }
    }
    
    void cleanup() {
        if (vcpu_fd >= 0) close(vcpu_fd);
        if (vm_fd >= 0) close(vm_fd);
        if (kvm_fd >= 0) close(kvm_fd);
    }
    
    ~KVMIntegration() {
        cleanup();
    }
};

int main() {
    std::cout << "=== AcheronARM64 Linux KVM Integration ===" << std::endl;
    
    KVMIntegration kvm;
    
    if (!kvm.initialize()) {
        std::cerr << "Failed to initialize KVM" << std::endl;
        return 1;
    }
    
    std::cout << "\n[KVM] KVM initialized successfully" << std::endl;
    std::cout << "[KVM] Ready to handle system register traps" << std::endl;
    
    return 0;
}
