# AcheronARM64 Engineering Roadmap

**Goal:** Complete iPhone virtualization on any platform (Windows, Linux, macOS x86)

---

## Phase 1: Core CPU Emulation ✅ (COMPLETE)

- [x] ARM64 system register emulation
- [x] System instruction decoder (MSR, MRS, barriers)
- [x] Exception handling (entry/return)
- [x] Test suite (55+ tests)
- [x] Cross-platform examples (HVF, WHP, KVM, JIT)

**Status:** ✅ Production ready

---

## Phase 2: Memory Management Unit (MMU)

**Why:** XNU kernel requires proper virtual memory management

### Components:
- [ ] **Page Table Walker** (4-level ARM64)
  - [ ] TTBR0/TTBR1 translation
  - [ ] Block/Page descriptors
  - [ ] Attribute extraction (AP, UXN, PXN, etc.)
  
- [ ] **TLB Emulation**
  - [ ] Software TLB cache
  - [ ] ASID support
  - [ ] Flush operations (VA, ASID, All)
  
- [ ] **Memory Attributes**
  - [ ] Device vs Normal memory
  - [ ] Cache policies (Write-Back, Write-Through)
  - [ ] Shareability domains

- [ ] **Fault Handling**
  - [ ] Translation faults
  - [ ] Permission faults
  - [ ] Domain faults

**Files to create:**
```
include/acheronarm64/
  - MMUEmulator.hpp
  - PageTable.hpp
  - TLBCache.hpp
  - MemoryAttribute.hpp

src/
  - MMUEmulator.cpp
  - PageTable.cpp
  - TLBCache.cpp
```

---

## Phase 3: Interrupt Controller (GICv3)

**Why:** XNU uses GICv3 for interrupt management

### Components:
- [ ] **Distributor (GICD)**
  - [ ] Interrupt enable/disable
  - [ ] Priority configuration
  - [ ] Target CPU routing
  - [ ] Pending/Active state
  
- [ ] **Redistributor (GICR)**
  - [ ] Per-CPU interrupt state
  - [ ] LPI configuration
  - [ ] Wake processing
  
- [ ] **CPU Interface (GICC)**
  - [ ] Interrupt acknowledgment
  - [ ] EOI handling
  - [ ] Priority masking
  
- [ ] **SGI/PPI/SPI Support**
  - [ ] Software Generated Interrupts
  - [ ] Private Peripheral Interrupts
  - [ ] Shared Peripheral Interrupts

**Files to create:**
```
include/acheronarm64/
  - GICv3.hpp
  - GICDistributor.hpp
  - GICRedistributor.hpp
  - InterruptController.hpp

src/
  - GICv3.cpp
  - GICDistributor.cpp
  - GICRedistributor.cpp
```

---

## Phase 4: Timer Emulation

**Why:** XNU requires architectural timers for scheduling

### Components:
- [ ] **Generic Timer**
  - [ ] Counter-value register (CNTPCT_EL0)
  - [ ] Compare-value register (CNTP_CVAL_EL0)
  - [ ] Control register (CNTP_CTL_EL0)
  - [ ] Timer status
  
- [ ] **Virtual Timer**
  - [ ] HYP timer support
  - [ ] Guest timer injection
  
- [ ] **Watchdog Timer**
  - [ ] Apple PMU watchdog
  - [ ] System watchdog

**Files to create:**
```
include/acheronarm64/
  - ARMTimer.hpp
  - GenericTimer.hpp
  - WatchdogTimer.hpp

src/
  - ARMTimer.cpp
  - GenericTimer.cpp
```

---

## Phase 5: Device Tree Support

**Why:** XNU reads hardware configuration from device tree

### Components:
- [ ] **Device Tree Parser**
  - [ ] FDT (Flattened Device Tree) parsing
  - [ ] Node traversal
  - [ ] Property reading
  
- [ ] **Apple Device Tree (ADT)**
  - [ ] ADT format parsing
  - [ ] Apple-specific properties
  - [ ] Node patching
  
- [ ] **Device Tree Generation**
  - [ ] Virtual hardware description
  - [ ] Memory map
  - [ ] Device nodes (UART, GIC, Timer, etc.)

**Files to create:**
```
include/acheronarm64/
  - DeviceTree.hpp
  - FDTParser.hpp
  - ADTParser.hpp
  - DeviceTreeNode.hpp

src/
  - DeviceTree.cpp
  - FDTParser.cpp
  - ADTParser.cpp
```

---

## Phase 6: Boot Chain Support

**Why:** iPhone boot chain (AVPBooter → iBoot → XNU)

### Components:
- [ ] **AVPBooter Emulation**
  - [ ] AVP (Apple Video Processor) boot
  - [ ] Firmware loading
  - [ ] Handoff to iBoot
  
- [ ] **iBoot Emulation**
  - [ ] iBoot entry point
  - [ ] Device tree preparation
  - [ ] Kernel loading
  - [ ] Boot args setup
  
- [ ] **XNU Boot**
  - [ ] boot_args structure
  - [ ] Kernel entry point
  - [ ] Early console setup

**Files to create:**
```
include/acheronarm64/
  - BootChain.hpp
  - AVPBooter.hpp
  - IBoot.hpp
  - XNUBoot.hpp

src/
  - BootChain.cpp
  - AVPBooter.cpp
  - IBoot.cpp
  - XNUBoot.cpp
```

---

## Phase 7: UART & Console

**Why:** Early kernel debug output

### Components:
- [ ] **PL011 UART**
  - [ ] Register emulation
  - [ ] TX/RX FIFO
  - [ ] Interrupt generation
  - [ ] Baud rate configuration
  
- [ ] **Console Output**
  - [ ] Host console redirection
  - [ ] Log file output
  - [ ] Interactive console

**Files to create:**
```
include/acheronarm64/
  - PL011UART.hpp
  - UARTDevice.hpp
  - ConsoleOutput.hpp

src/
  - PL011UART.cpp
  - UARTDevice.cpp
```

---

## Phase 8: VirtIO Devices

**Why:** Paravirtualized devices for performance

### Components:
- [ ] **VirtIO Transport**
  - [ ] MMIO transport
  - [ ] Virtqueue management
  - [ ] Interrupt injection
  
- [ ] **VirtIO Block**
  - [ ] Disk image backing
  - [ ] Read/write requests
  - [ ] Flush operations
  
- [ ] **VirtIO Network**
  - [ ] Packet transmission
  - [ ] Packet reception
  - [ ] MAC address
  - [ ] User-mode NAT
  
- [ ] **VirtIO RNG**
  - [ ] Random number generation
  - [ ] Entropy source

**Files to create:**
```
include/acheronarm64/
  - VirtIODevice.hpp
  - VirtIOMMIO.hpp
  - VirtIOBlock.hpp
  - VirtIONetwork.hpp
  - VirtIORNG.hpp

src/
  - VirtIODevice.cpp
  - VirtIOMMIO.cpp
  - VirtIOBlock.cpp
  - VirtIONetwork.cpp
  - VirtIORNG.cpp
```

---

## Phase 9: Display & GPU

**Why:** iPhone display output

### Components:
- [ ] **Framebuffer Emulation**
  - [ ] Memory-mapped framebuffer
  - [ ] Resolution configuration
  - [ ] Pixel format (BGRA8888)
  
- [ ] **Display Controller**
  - [ ] Apple display controller
  - [ ] Timing configuration
  - [ ] VBLANK interrupts
  
- [ ] **Host Integration**
  - [ ] Metal (macOS)
  - [ ] DirectX (Windows)
  - [ ] OpenGL/Vulkan (Linux)

**Files to create:**
```
include/acheronarm64/
  - FramebufferDevice.hpp
  - DisplayController.hpp
  - HostDisplay.hpp

src/
  - FramebufferDevice.cpp
  - DisplayController.cpp
  - HostDisplay.cpp
```

---

## Phase 10: Input Devices

**Why:** Touch input, buttons

### Components:
- [ ] **Touchscreen**
  - [ ] Multitouch emulation
  - [ ] Touch coordinate mapping
  - [ ] Gesture recognition
  
- [ ] **Buttons**
  - [ ] Power button
  - [ ] Volume buttons
  - [ ] Home button (virtual)
  
- [ ] **HID Support**
  - [ ] USB HID emulation
  - [ ] Touch reports

**Files to create:**
```
include/acheronarm64/
  - TouchscreenDevice.hpp
  - ButtonDevice.hpp
  - HIDDevice.hpp

src/
  - TouchscreenDevice.cpp
  - ButtonDevice.cpp
  - HIDDevice.cpp
```

---

## Phase 11: Storage Emulation

**Why:** iPhone NAND storage

### Components:
- [ ] **NAND Controller**
  - [ ] NVMe emulation
  - [ ] Apple NAND protocol
  
- [ ] **Filesystem Support**
  - [ ] APFS read support
  - [ ] HFS+ read support
  
- [ ] **Disk Images**
  - [ ] Raw image support
  - [ ] QCOW2 support
  - [ ] Sparse images

**Files to create:**
```
include/acheronarm64/
  - NANDController.hpp
  - NVMeDevice.hpp
  - DiskImage.hpp

src/
  - NANDController.cpp
  - NVMeDevice.cpp
  - DiskImage.cpp
```

---

## Phase 12: Security Emulation

**Why:** Apple security features

### Components:
- [ ] **SEP (Secure Enclave)**
  - [ ] SEP communication
  - [ ] Keybag emulation
  - [ ] Secure boot
  
- [ ] **AMFI (Apple Mobile File Integrity)**
  - [ ] Code signing emulation
  - [ ] Trust cache
  - [ ] Development mode
  
- [ ] **KPP (Kernel Page Protection)**
  - [ ] Execute-only memory
  - [ ] Pointer authentication

**Files to create:**
```
include/acheronarm64/
  - SEPDevice.hpp
  - AMFIEmulator.hpp
  - SecurityFeatures.hpp

src/
  - SEPDevice.cpp
  - AMFIEmulator.cpp
```

---

## Phase 13: Power Management

**Why:** iPhone power states

### Components:
- [ ] **PMGR (Power Management)**
  - [ ] Power domains
  - [ ] Clock gating
  - [ ] Device power states
  
- [ ] **Battery Emulation**
  - [ ] Charge level
  - [ ] Charging state
  - [ ] Power source

**Files to create:**
```
include/acheronarm64/
  - PowerManager.hpp
  - BatteryDevice.hpp

src/
  - PowerManager.cpp
  - BatteryDevice.cpp
```

---

## Phase 14: Complete System Integration

**Why:** All components working together

### Components:
- [ ] **System-on-Chip (SoC) Model**
  - [ ] Apple A16/A17 model
  - [ ] Memory map
  - [ ] Device interconnect
  
- [ ] **Machine Model**
  - [ ] iPhone17,3 model
  - [ ] Hardware configuration
  - [ ] Device tree generation
  
- [ ] **Full Boot Sequence**
  - [ ] AVPBooter → iBoot → XNU
  - [ ] Userspace launch
  - [ ] First boot complete

**Files to create:**
```
include/acheronarm64/
  - AppleSoC.hpp
  - iPhoneModel.hpp
  - VirtualiPhone.hpp

src/
  - AppleSoC.cpp
  - iPhoneModel.cpp
  - VirtualiPhone.cpp
```

---

## Phase 15: Platform Backends

**Why:** Run on all platforms

### Components:
- [ ] **macOS HVF Backend** ✅ (started)
  - [ ] Full system register trapping
  - [ ] Memory mapping
  - [ ] Interrupt injection
  
- [ ] **Windows WHP Backend**
  - [ ] WHP configuration
  - [ ] ARM64 guest support
  - [ ] Exit handling
  
- [ ] **Linux KVM Backend**
  - [ ] KVM ARM64
  - [ ] VGIC support
  - [ ] Timer configuration
  
- [ ] **Pure JIT Backend (x86_64)**
  - [ ] ARM64 → x86_64 translation
  - [ ] Block caching
  - [ ] Register mapping

**Files to create:**
```
src/backends/
  - HVFBackend.cpp
  - WHPBackend.cpp
  - KVMBackend.cpp
  - JITBackend.cpp
  - JITBackend.h (x86)
  - JITBackend.cpp (x86)
```

---

## Phase 16: Tools & CLI

**Why:** User-friendly interface

### Components:
- [ ] **Command-Line Interface**
  - [ ] VM creation
  - [ ] Boot configuration
  - [ ] Device passthrough
  
- [ ] **GUI Frontend** (optional)
  - [ ] Qt-based UI
  - [ ] Display window
  - [ ] Input handling
  
- [ ] **Debug Tools**
  - [ ] GDB stub
  - [ ] Register dump
  - [ ] Memory inspection

**Files to create:**
```
tools/
  - acheron-cli.cpp
  - VMCommand.cpp
  - BootCommand.cpp
  - DebugCommand.cpp
```

---

## Timeline Estimate

| Phase | Components | Estimated Time |
|-------|------------|----------------|
| Phase 1 | Core CPU | ✅ Complete |
| Phase 2 | MMU | 2-3 weeks |
| Phase 3 | GICv3 | 2 weeks |
| Phase 4 | Timers | 1 week |
| Phase 5 | Device Tree | 1-2 weeks |
| Phase 6 | Boot Chain | 2-3 weeks |
| Phase 7 | UART | 3-5 days |
| Phase 8 | VirtIO | 2 weeks |
| Phase 9 | Display | 2-3 weeks |
| Phase 10 | Input | 1 week |
| Phase 11 | Storage | 2 weeks |
| Phase 12 | Security | 3-4 weeks |
| Phase 13 | Power | 1 week |
| Phase 14 | Integration | 2-3 weeks |
| Phase 15 | Backends | 4-6 weeks |
| Phase 16 | Tools | 2 weeks |

**Total:** 6-9 months for complete iPhone emulation

---

## Immediate Next Steps

1. **Phase 2 (MMU)** - Critical for XNU boot
2. **Phase 3 (GICv3)** - Required for interrupts
3. **Phase 4 (Timers)** - Needed for kernel scheduler
4. **Phase 7 (UART)** - Debug output during boot

These 4 phases will get XNU kernel booting to early console.

---

## Let's Start Building

**Which phase should we tackle first?**

I recommend **Phase 2 (MMU)** because:
1. XNU won't boot without virtual memory
2. Required for exception handling
3. Foundation for all other components

Ready to engineer the MMU? 🚀
