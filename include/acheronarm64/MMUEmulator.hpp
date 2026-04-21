// MMUEmulator.hpp - ARM64 Memory Management Unit Emulation
// Part of AcheronARM64 - ARM64 System Register Emulation Library
//
// Implements 4-level page table walking for ARM64 virtual memory translation

#pragma once

#include "ARM64SystemRegisters.hpp"
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace acheronarm64 {

// ============================================================================
// ARM64 Memory Attributes
// ============================================================================

/**
 * @brief Memory type for ARM64 memory regions
 */
enum class MemoryType : uint8_t {
    DEVICE_nGnRnE = 0x00,  // Non-Gathering, Non-Reordering, No Early Write Ack
    DEVICE_nGnRE  = 0x01,  // Non-Gathering, Non-Reordering, Early Write Ack
    DEVICE_nGRE   = 0x02,  // Non-Gathering, Reordering, Early Write Ack
    DEVICE_GRE    = 0x03,  // Gathering, Reordering, Early Write Ack
    NORMAL        = 0x04,  // Normal memory
    NORMAL_NC     = 0x05,  // Normal Non-Cacheable
    NORMAL_WB     = 0x06,  // Normal Write-Back
    NORMAL_WT     = 0x07,  // Normal Write-Through
};

/**
 * @brief Access permissions for ARM64 memory regions
 */
enum class AccessPermission : uint8_t {
    EL0_NONE = 0b00,  // EL0: No access
    EL0_RO   = 0b01,  // EL0: Read-only
    EL0_RW   = 0b10,  // EL0: Read-write
    EL1_NONE = 0b00,  // EL1: No access (when AP[2]=1)
    EL1_RO   = 0b01,  // EL1: Read-only
    EL1_RW   = 0b10,  // EL1: Read-write
};

/**
 * @brief Memory region descriptor
 */
struct MemoryRegion {
    uint64_t base_address;
    uint64_t size;
    MemoryType type;
    AccessPermission ap;
    bool uxn;      // Unprivileged eXecute Never
    bool pxn;      // Privileged eXecute Never
    bool contiguous;
    bool shareable;
    
    MemoryRegion()
        : base_address(0), size(0), type(MemoryType::NORMAL),
          ap(AccessPermission::EL1_RW), uxn(false), pxn(false),
          contiguous(false), shareable(false) {}
};

// ============================================================================
// TLB Entry
// ============================================================================

/**
 * @brief Translation Lookaside Buffer entry
 */
struct TLBEntry {
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t size;  // Page size (4KB, 2MB, 1GB)
    MemoryType type;
    AccessPermission ap;
    bool uxn;
    bool pxn;
    uint16_t asid;  // Address Space ID
    bool valid;
    
    TLBEntry()
        : virtual_address(0), physical_address(0), size(0),
          type(MemoryType::NORMAL), ap(AccessPermission::EL1_RW),
          uxn(false), pxn(false), asid(0), valid(false) {}
    
    bool matches(uint64_t va, uint16_t asid_match) const {
        if (!valid) return false;
        if (asid != asid_match && asid != 0xFFFF) return false;
        
        uint64_t mask = ~(size - 1);
        return (virtual_address & mask) == (va & mask);
    }
};

// ============================================================================
// Page Table Descriptor
// ============================================================================

/**
 * @brief ARM64 page table descriptor (block or page)
 */
struct PageTableDescriptor {
    union {
        uint64_t value;
        struct {
            uint64_t valid        : 1;  // [0] Valid bit
            uint64_t table        : 1;  // [1] Table bit (0=block/page, 1=table)
            uint64_t sh           : 2;  // [3:2] Shareability
            uint64_t ng           : 1;  // [4] Not Global
            uint64_t ap           : 3;  // [6:5] Access permissions
            uint64_t ns           : 1;  // [7] Non-Secure
            uint64_t attr_index   : 3;  // [10:8] Attribute index
            uint64_t reserved1    : 1;  // [11]
            uint64_t output_addr  : 36; // [47:12] Output address (PA)
            uint64_t reserved2    : 16; // [63:48]
        };
        struct {
            uint64_t reserved3    : 2;  // [1:0]
            uint64_t s2ap         : 2;  // [3:2] Stage 2 AP
            uint64_t reserved4    : 4;  // [7:4]
            uint64_t s2attr_index : 3;  // [10:8] Stage 2 attribute index
            uint64_t reserved5    : 1;  // [11]
            uint64_t s2output_addr : 36; // [47:12] Stage 2 output address
        } s2;  // Stage 2 translation
    };
    
    PageTableDescriptor() : value(0) {}
    
    bool is_valid() const { return valid == 1; }
    bool is_table() const { return table == 1; }
    bool is_block() const { return valid == 1 && table == 0; }
    bool is_page() const { return valid == 1 && table == 0; }
    
    uint64_t get_output_address() const {
        return output_addr << 12;
    }
    
    void set_output_address(uint64_t pa) {
        output_addr = (pa >> 12) & 0xFFFFFFFFF;
    }
};

// ============================================================================
// MMU Emulator
// ============================================================================

/**
 * @brief ARM64 MMU emulator with TLB caching
 * 
 * Implements 4-level page table walking for ARM64 virtual memory.
 * Supports:
 * - TTBR0 and TTBR1 translation
 * - Block mappings (1GB, 2MB)
 * - Page mappings (4KB)
 * - ASID-based TLB tagging
 * - Memory attribute extraction
 */
class MMUEmulator {
public:
    MMUEmulator();
    
    /**
     * @brief Translate virtual address to physical address
     * 
     * @param va Virtual address to translate
     * @param asid Address Space ID (optional)
     * @return Physical address or nullopt if translation fails
     */
    std::optional<uint64_t> translate(uint64_t va, uint16_t asid = 0);
    
    /**
     * @brief Translate virtual address with memory attributes
     * 
     * @param va Virtual address to translate
     * @param region Output memory region attributes
     * @param asid Address Space ID
     * @return Physical address or nullopt if translation fails
     */
    std::optional<uint64_t> translate_with_attributes(
        uint64_t va, MemoryRegion& region, uint16_t asid = 0);
    
    /**
     * @brief Invalidate TLB entry for specific VA
     * 
     * @param va Virtual address to invalidate
     * @param asid Address Space ID
     */
    void tlb_invalidate_va(uint64_t va, uint16_t asid = 0);
    
    /**
     * @brief Invalidate TLB entries for specific ASID
     * 
     * @param asid Address Space ID
     */
    void tlb_invalidate_asid(uint16_t asid);
    
    /**
     * @brief Invalidate entire TLB
     */
    void tlb_invalidate_all();
    
    /**
     * @brief Set base address for TTBR0
     * 
     * @param ttbr0 TTBR0 value
     */
    void set_ttbr0(uint64_t ttbr0);
    
    /**
     * @brief Set base address for TTBR1
     * 
     * @param ttbr1 TTBR1 value
     */
    void set_ttbr1(uint64_t ttbr1);
    
    /**
     * @brief Set TCR configuration
     * 
     * @param tcr TCR value
     */
    void set_tcr(uint64_t tcr);
    
    /**
     * @brief Check if MMU is enabled
     * 
     * @param sctlr SCTLR_EL1 value
     * @return true if MMU enabled
     */
    bool is_mmu_enabled(uint64_t sctlr) const;
    
    /**
     * @brief Get statistics
     */
    struct Stats {
        uint64_t tlb_hits;
        uint64_t tlb_misses;
        uint64_t page_walks;
        uint64_t translation_faults;
    };
    
    const Stats& get_stats() const { return stats_; }
    void reset_stats() { stats_ = Stats{}; }

private:
    // TLB cache
    std::unordered_map<uint64_t, TLBEntry> tlb_;
    static constexpr size_t TLB_SIZE = 1024;  // Max TLB entries
    
    // Translation registers
    uint64_t ttbr0_;
    uint64_t ttbr1_;
    uint64_t tcr_;
    
    // Statistics
    Stats stats_;
    
    // Internal methods
    std::optional<uint64_t> walk_page_tables(uint64_t va, MemoryRegion& region);
    std::optional<uint64_t> walk_level(uint64_t va, uint64_t table_base, 
                                        int level, MemoryRegion& region);
    MemoryType decode_memory_type(uint8_t attr_index) const;
    AccessPermission decode_permissions(uint8_t ap_bits) const;
    
    // TCR helpers
    uint8_t get_t0sz() const { return tcr_ & 0x3F; }
    uint8_t get_t1sz() const { return (tcr_ >> 16) & 0x3F; }
    uint64_t get_ttbr0_mask() const;
    uint64_t get_ttbr1_mask() const;
    bool va_in_ttbr0(uint64_t va) const;
    bool va_in_ttbr1(uint64_t va) const;
};

} // namespace acheronarm64
