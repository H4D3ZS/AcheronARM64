// MMUEmulator.cpp - ARM64 Memory Management Unit Emulation
// Implementation

#include "MMUEmulator.hpp"
#include <iostream>
#include <cstring>

namespace acheronarm64 {

MMUEmulator::MMUEmulator()
    : ttbr0_(0), ttbr1_(0), tcr_(0) {
    reset_stats();
}

bool MMUEmulator::is_mmu_enabled(uint64_t sctlr) const {
    SCTLR_EL1 sctlr_reg;
    sctlr_reg.value = sctlr;
    return sctlr_reg.M == 1;  // Bit 0 = MMU enable
}

void MMUEmulator::set_ttbr0(uint64_t ttbr0) {
    ttbr0_ = ttbr0;
    tlb_invalidate_all();  // TTBR change requires TLB flush
}

void MMUEmulator::set_ttbr1(uint64_t ttbr1) {
    ttbr1_ = ttbr1;
    tlb_invalidate_all();
}

void MMUEmulator::set_tcr(uint64_t tcr) {
    tcr_ = tcr;
    tlb_invalidate_all();
}

uint64_t MMUEmulator::get_ttbr0_mask() const {
    // TTBR0 mask based on T0SZ
    uint8_t t0sz = get_t0sz();
    uint64_t ipa_bits = 48 - t0sz;
    return (1ULL << ipa_bits) - 1;
}

uint64_t MMUEmulator::get_ttbr1_mask() const {
    // TTBR1 mask based on T1SZ
    uint8_t t1sz = get_t1sz();
    uint64_t ipa_bits = 48 - t1sz;
    return (1ULL << ipa_bits) - 1;
}

bool MMUEmulator::va_in_ttbr0(uint64_t va) const {
    // TTBR0 covers lower half of address space
    uint8_t t0sz = get_t0sz();
    uint64_t ttbr0_size = 1ULL << (64 - t0sz);
    return va < ttbr0_size;
}

bool MMUEmulator::va_in_ttbr1(uint64_t va) const {
    // TTBR1 covers upper half of address space
    uint8_t t1sz = get_t1sz();
    uint64_t ttbr1_start = 1ULL << (64 - t1sz);
    return va >= ttbr1_start;
}

std::optional<uint64_t> MMUEmulator::translate(uint64_t va, uint16_t asid) {
    MemoryRegion region;
    return translate_with_attributes(va, region, asid);
}

std::optional<uint64_t> MMUEmulator::translate_with_attributes(
    uint64_t va, MemoryRegion& region, uint16_t asid) {
    
    // Check TLB first
    uint64_t tlb_key = (va & ~0xFFFULL) | (static_cast<uint64_t>(asid) << 48);
    auto tlb_it = tlb_.find(tlb_key);
    if (tlb_it != tlb_.end() && tlb_it->second.valid) {
        stats_.tlb_hits++;
        const TLBEntry& entry = tlb_it->second;
        region.base_address = entry.physical_address;
        region.size = entry.size;
        region.type = entry.type;
        region.ap = entry.ap;
        region.uxn = entry.uxn;
        region.pxn = entry.pxn;
        
        // Calculate offset within page
        uint64_t offset = va & (entry.size - 1);
        return entry.physical_address + offset;
    }
    
    stats_.tlb_misses++;
    
    // Walk page tables
    auto result = walk_page_tables(va, region);
    
    if (result.has_value()) {
        // Add to TLB
        TLBEntry entry;
        entry.virtual_address = va & ~(region.size - 1);
        entry.physical_address = region.base_address & ~(region.size - 1);
        entry.size = region.size;
        entry.type = region.type;
        entry.ap = region.ap;
        entry.uxn = region.uxn;
        entry.pxn = region.pxn;
        entry.asid = asid;
        entry.valid = true;
        
        // Evict old entries if TLB is full
        if (tlb_.size() >= TLB_SIZE) {
            tlb_.erase(tlb_.begin());
        }
        
        tlb_[tlb_key] = entry;
    }
    
    return result;
}

std::optional<uint64_t> MMUEmulator::walk_page_tables(
    uint64_t va, MemoryRegion& region) {
    
    stats_.page_walks++;
    
    // Determine which TTBR to use
    uint64_t table_base;
    if (va_in_ttbr0(va)) {
        table_base = ttbr0_ & 0xFFFFFFFFFFFF0000ULL;  // Clear ASID and reserved bits
    } else if (va_in_ttbr1(va)) {
        table_base = ttbr1_ & 0xFFFFFFFFFFFF0000ULL;
    } else {
        // Translation fault - VA not in any TTBR range
        stats_.translation_faults++;
        return std::nullopt;
    }
    
    // Start page table walk from level 0
    return walk_level(va, table_base, 0, region);
}

std::optional<uint64_t> MMUEmulator::walk_level(
    uint64_t va, uint64_t table_base, int level, MemoryRegion& region) {
    
    // Calculate index for this level
    // ARM64 uses 9 bits per level (512 entries per table)
    int shift = 39 - (level * 9);
    uint64_t index = (va >> shift) & 0x1FF;
    
    // Read descriptor from table
    PageTableDescriptor desc;
    // In real implementation: read from memory at table_base + (index * 8)
    // For now, this is a stub - real implementation needs memory access callback
    desc.value = 0;  // Stub - needs memory read
    
    if (!desc.is_valid()) {
        stats_.translation_faults++;
        return std::nullopt;  // Translation fault
    }
    
    if (desc.is_table()) {
        // Descriptor points to next level table
        uint64_t next_table = desc.get_output_address();
        return walk_level(va, next_table, level + 1, region);
    }
    
    // Block or page descriptor
    uint64_t output_addr = desc.get_output_address();
    
    // Determine block/page size based on level
    switch (level) {
        case 0:  // 1GB block
            region.size = 1024 * 1024 * 1024;  // 1GB
            output_addr &= ~(region.size - 1);
            break;
        case 1:  // 2MB block
            region.size = 2 * 1024 * 1024;  // 2MB
            output_addr &= ~(region.size - 1);
            break;
        case 2:  // Could be 2MB block or 4KB page (depends on descriptor)
            // For simplicity, assume 2MB block
            region.size = 2 * 1024 * 1024;
            output_addr &= ~(region.size - 1);
            break;
        case 3:  // 4KB page
            region.size = 4096;  // 4KB
            output_addr &= ~(region.size - 1);
            break;
        default:
            stats_.translation_faults++;
            return std::nullopt;
    }
    
    // Extract memory attributes
    region.base_address = output_addr;
    region.type = decode_memory_type(desc.attr_index);
    region.ap = decode_permissions(desc.ap);
    region.uxn = false;  // Would be in extended descriptor
    region.pxn = false;  // Would be in extended descriptor
    region.contiguous = false;
    region.shareable = (desc.sh != 0);
    
    // Add offset within block/page
    uint64_t offset = va & (region.size - 1);
    return output_addr + offset;
}

MemoryType MMUEmulator::decode_memory_type(uint8_t attr_index) const {
    // In real implementation, decode from MAIR_EL1
    // For now, return defaults based on index
    switch (attr_index) {
        case 0x00: return MemoryType::DEVICE_nGnRnE;
        case 0x01: return MemoryType::DEVICE_nGnRE;
        case 0x04: return MemoryType::NORMAL_NC;
        case 0x44: return MemoryType::NORMAL_WB;
        default: return MemoryType::NORMAL;
    }
}

AccessPermission MMUEmulator::decode_permissions(uint8_t ap_bits) const {
    switch (ap_bits) {
        case 0b000: return AccessPermission::EL1_RW;  // RW at EL1, none at EL0
        case 0b001: return AccessPermission::EL0_RO;  // RW at EL1, RO at EL0
        case 0b010: return AccessPermission::EL0_RW;  // RW at EL1, RW at EL0
        case 0b100: return AccessPermission::EL1_NONE;  // None at EL1, none at EL0
        case 0b101: return AccessPermission::EL1_RO;  // RO at EL1, RO at EL0
        case 0b110: return AccessPermission::EL1_RW;  // RW at EL1, RW at EL0
        default: return AccessPermission::EL1_RW;
    }
}

void MMUEmulator::tlb_invalidate_va(uint64_t va, uint16_t asid) {
    uint64_t key = (va & ~0xFFFULL) | (static_cast<uint64_t>(asid) << 48);
    tlb_.erase(key);
}

void MMUEmulator::tlb_invalidate_asid(uint16_t asid) {
    for (auto it = tlb_.begin(); it != tlb_.end(); ) {
        if ((it->second.asid == asid || asid == 0xFFFF) && it->second.valid) {
            it = tlb_.erase(it);
        } else {
            ++it;
        }
    }
}

void MMUEmulator::tlb_invalidate_all() {
    tlb_.clear();
}

} // namespace acheronarm64
