# 6502 Emulator: Fix Plan - Code Quality, Security, and Architecture

**Date:** 2026-08-09  
**Scope:** Address all findings from code review, security scan, and architecture review  
**Priority:** High, Medium, Low

---

## Executive Summary

This plan addresses **20 distinct findings** across three categories:
- **Critical bugs & security issues** (3 items): Fix before production use
- **Architecture & design issues** (10 items): Refactor to enable extensibility
- **Documentation & testing gaps** (4 items): Improve maintainability
- **Performance & polish** (3 items): Code quality and efficiency

Total estimated impact: Prevents future regressions, enables device attachment and system variants, hardens CLI input.

---

## CRITICAL BUGS & SECURITY ISSUES

### 1. Disassembler Infinite Loop on 0xFFFF

**Severity:** HIGH  
**File:** `cpu6502/src/disassembler.cpp:25-35`  
**Problem:** When disassembling to address 0xFFFF, the loop wraps uint16_t (0xFFFF → 0x0000), creating an infinite loop.

**Suggestion:**
```cpp
// Change addr from uint16_t to uint32_t
uint32_t addr = start;
while(addr <= static_cast<uint32_t>(stop))
{
    // ... disassembly logic ...
    addr++;
}
```

**Acceptance Criteria:**
- [ ] `addr` is declared as `uint32_t` instead of implicitly `uint16_t`
- [ ] Loop terminates correctly when stop = 0xFFFF
- [ ] All existing disassembly tests pass
- [ ] New test case: disassemble(0xFF00, 0xFFFF) completes without hang

---

### 2. CLI Input Validation - Unhandled Hex Parse Exceptions

**Severity:** HIGH  
**File:** `cli6502/cli6502.cpp:150-151`  
**Problem:** `std::stoul()` throws uncaught exceptions on invalid hex input (e.g., "0xZZZZ", "0x10000"), crashing the CLI (DoS).

**Suggestion:**
```cpp
try {
    const uint16_t start = (spanArg.size() >= 4) 
        ? gsl::narrow<uint16_t>(std::stoul(gsl::at(spanArg, 3), nullptr, 16))
        : 0x8000;
    const uint16_t stop = (spanArg.size() >= 5)
        ? gsl::narrow<uint16_t>(std::stoul(gsl::at(spanArg, 4), nullptr, 16))
        : gsl::narrow<uint16_t>(start + 0x00FF);
} catch (const std::exception& e) {
    std::cerr << "Error: Invalid hex address: " << e.what() << "\n";
    return 1;
}
```

**Acceptance Criteria:**
- [ ] CLI wraps hex parsing in try-catch block
- [ ] Invalid hex addresses (e.g., "0xZZZZ", "0xGGGG") produce user-friendly error message to stderr
- [ ] Out-of-range addresses (e.g., "0x10000") produce error message instead of crashing
- [ ] Exit code is non-zero on input error
- [ ] New test: `6502cli disasm file.bin INVALID` exits gracefully with error message

---

### 3. CLI File Path Validation

**Severity:** MEDIUM  
**File:** `cli6502/cli6502.cpp:30-62`  
**Problem:** `load_file()` accepts arbitrary file paths with no validation; could allow path traversal or unintended file access.

**Suggestion:**
```cpp
// Validate path doesn't escape or use absolute paths
std::string validate_path(const std::string& path) {
    if (path.find("..") != std::string::npos) {
        throw std::invalid_argument("Path traversal not allowed");
    }
    if (path[0] == '/' || (path.size() > 2 && path[1] == ':')) {
        throw std::invalid_argument("Absolute paths not allowed");
    }
    return path;
}
```

**Acceptance Criteria:**
- [ ] Path validation rejects ".." sequences
- [ ] Path validation rejects absolute paths (starting with "/" or drive letter)
- [ ] Paths like "../../../etc/passwd" are rejected with error message
- [ ] Valid relative paths like "roms/game.bin" work normally
- [ ] New test case validates path rejection behavior

---

## ARCHITECTURE & DESIGN ISSUES

### 4. Encapsulate CPU Registers - Break Public Access Pattern

**Severity:** CRITICAL (Design)  
**File:** `cpu6502/include/cpu6502.h:35-40`  
**Problem:** All registers exposed as public members; prevents adding validation, logging, or side effects. Tests and external code bypass the CPU interface.

**Suggestion:**
```cpp
class CPU6502 {
private:
    uint8_t a_, x_, y_, stkp_, status_;
    uint16_t pc_;
    // ... other state ...

public:
    // Read-only accessors
    uint8_t get_a() const { return a_; }
    uint8_t get_x() const { return x_; }
    uint8_t get_y() const { return y_; }
    uint8_t get_stkp() const { return stkp_; }
    uint8_t get_status() const { return status_; }
    uint16_t get_pc() const { return pc_; }
    
    // Validated setters (used internally and by tests/debugger)
    void set_a(uint8_t val) { a_ = val; }
    void set_x(uint8_t val) { x_ = val; }
    void set_y(uint8_t val) { y_ = val; }
    // ... etc ...
};
```

**Acceptance Criteria:**
- [ ] All CPU registers are private members
- [ ] Public read-only getter methods exist for all registers
- [ ] Public setter methods exist for test/debugger use
- [ ] All tests updated to use getters/setters instead of direct access (e.g., `cpu.get_a()` instead of `cpu.a_`)
- [ ] Disassembler continues to work without CPU register access (only accesses Bus)
- [ ] All existing test assertions pass
- [ ] No external code can directly mutate registers

---

### 5. Introduce Memory Device Abstraction

**Severity:** CRITICAL (Design)  
**File:** `cpu6502/include/bus.h`  
**Problem:** Bus is hardcoded to single RAM array; no mechanism for memory-mapped I/O or device attachment. Cannot add peripherals without modifying Bus.

**Suggestion:**
```cpp
// New abstraction for memory-mapped devices
class MemoryDevice {
public:
    virtual ~MemoryDevice() = default;
    virtual uint8_t read(uint16_t offset) = 0;
    virtual void write(uint16_t offset, uint8_t value) = 0;
};

// RAM device wrapper
class RAMDevice : public MemoryDevice {
private:
    std::array<uint8_t, 65536> ram_;
public:
    uint8_t read(uint16_t offset) override { return ram_[offset]; }
    void write(uint16_t offset, uint8_t value) override { ram_[offset] = value; }
};

// Refactored Bus with device registry
class Bus {
private:
    std::map<std::pair<uint16_t, uint16_t>, std::shared_ptr<MemoryDevice>> devices_;
    
public:
    void attach_device(uint16_t start, uint16_t end, std::shared_ptr<MemoryDevice> device) {
        devices_[{start, end}] = device;
    }
    
    uint8_t read(uint16_t addr, bool bReadOnly = false) const {
        for (auto& [range, device] : devices_) {
            if (addr >= range.first && addr <= range.second) {
                return device->read(addr - range.first);
            }
        }
        throw std::out_of_range("No device at address");
    }
    
    void write(uint16_t addr, uint8_t value) {
        for (auto& [range, device] : devices_) {
            if (addr >= range.first && addr <= range.second) {
                device->write(addr - range.first, value);
                return;
            }
        }
        throw std::out_of_range("No device at address");
    }
};
```

**Acceptance Criteria:**
- [ ] `MemoryDevice` interface created with `read()` and `write()` methods
- [ ] `RAMDevice` class wraps 64KB RAM and implements MemoryDevice
- [ ] `Bus` maintains registry of address ranges → devices
- [ ] `Bus::attach_device()` method allows registering new devices at runtime
- [ ] Existing RAM access (0x0000-0xFFFF) works via RAMDevice
- [ ] All existing tests pass without modification
- [ ] Can attach a test device to arbitrary address range without modifying Bus class
- [ ] New test: Attach dummy device at 0x6000-0x6FFF and verify read/write isolation

---

### 6. Implement Illegal Instruction Handling

**Severity:** HIGH (Design)  
**File:** `cpu6502/src/cpu6502.cpp:786-789`  
**Problem:** XXX() (undefined opcodes) silently returns 0; no error reporting or distinction from NOP. Makes debugging harder.

**Suggestion:**
```cpp
// Add instruction error tracking
class CPU6502 {
private:
    uint32_t illegal_instruction_count_ = 0;
    std::function<void(uint16_t pc, uint8_t opcode)> on_illegal_ = nullptr;

public:
    void set_illegal_instruction_handler(std::function<void(uint16_t, uint8_t)> handler) {
        on_illegal_ = handler;
    }
    
    uint32_t get_illegal_instruction_count() const { return illegal_instruction_count_; }
};

// Updated XXX() implementation
uint8_t CPU6502::XXX() noexcept {
    illegal_instruction_count_++;
    if (on_illegal_) {
        on_illegal_(pc_, opcode_);
    }
    return 0;
}
```

**Acceptance Criteria:**
- [ ] CPU tracks count of illegal instructions executed (`get_illegal_instruction_count()`)
- [ ] Optional handler callback can be installed via `set_illegal_instruction_handler()`
- [ ] When illegal instruction executed, handler is called with (pc, opcode)
- [ ] Tests can verify illegal instruction execution is detected
- [ ] New test: Execute opcode 0x02 (undefined) and verify counter increments
- [ ] New test: Install handler and verify it's called when illegal instruction hits

---

### 7. Encapsulate Clock/Complete Polling Pattern

**Severity:** MEDIUM (Design)  
**File:** `cpu6502/include/cpu6502.h:54-58`  
**Problem:** Clock/complete contract is implicit; every caller must implement the polling loop. Creates repetitive code and failure modes.

**Suggestion:**
```cpp
class CPU6502 {
public:
    // New method encapsulates the polling pattern
    void execute_single_instruction() noexcept {
        do { clock(); } while (!complete());
    }
    
    // Existing methods remain for fine-grained control
    void clock() noexcept;
    bool complete() const noexcept;
};
```

**Acceptance Criteria:**
- [ ] `execute_single_instruction()` method added to CPU6502
- [ ] Method encapsulates the do-while polling loop
- [ ] All tests updated to use `execute_single_instruction()` where applicable
- [ ] Tests that need fine-grained clock control can still use `clock()` + `complete()`
- [ ] Code is more readable; polling pattern not repeated 15+ times

---

### 8. Address bReadOnly Semantic - Implement or Remove

**Severity:** MEDIUM (Design)  
**File:** `cpu6502/src/bus.cpp:14-16`  
**Problem:** `bReadOnly` parameter is marked `[[maybe_unused]]` but ignored; creates semantic confusion about side effects.

**Suggestion (Option A - Implement):**
```cpp
class Bus {
    // Track which devices support read-only access
    std::map<uint16_t, bool> device_read_only_;
    
    uint8_t read(uint16_t addr, bool bReadOnly = false) const {
        if (bReadOnly && !device_read_only_.at(addr)) {
            // Could log warning or throw
        }
        return ram_[addr];
    }
};
```

**Suggestion (Option B - Remove):**
```cpp
// Simpler: Just remove the unused parameter
uint8_t Bus::read(uint16_t addr) const noexcept {
    return ram_[addr];
}
// Update callers (disassembler) to not pass bReadOnly
```

**Acceptance Criteria:**
- [ ] If implementing: `bReadOnly` parameter has observable effect (tracking, logging, or validation)
- [ ] If removing: Parameter deleted from Bus::read() and all call sites updated
- [ ] No more `[[maybe_unused]]` suppressions for this parameter
- [ ] Semantic intent is clear to new developers
- [ ] All tests pass

---

### 9. Support Runtime Instruction Set Extension

**Severity:** MEDIUM (Design)  
**File:** `cpu6502/include/instruction_set.h:48`, `cpu6502/src/instruction_set.cpp`  
**Problem:** Instruction set is baked in at compile time via constexpr. Cannot support 6502 variants (CMOS, WDC) or add custom opcodes for test/debug without recompilation.

**Suggestion:**
```cpp
// Keep compile-time lookup for performance (primary path)
const std::array<INSTRUCTION, 256>& CPU6502::lookupTable() noexcept {
    static const auto lookup = CREATE_LOOKUP_TABLE();
    return lookup;
}

// Add optional runtime override mechanism for debugging/variants
class CPU6502 {
private:
    std::map<uint8_t, INSTRUCTION> custom_instructions_;
    
public:
    void register_custom_instruction(uint8_t opcode, const INSTRUCTION& instr) {
        custom_instructions_[opcode] = instr;
    }
    
    const INSTRUCTION& get_instruction(uint8_t opcode) const {
        auto it = custom_instructions_.find(opcode);
        if (it != custom_instructions_.end()) {
            return it->second;
        }
        return lookupTable()[opcode];
    }
};
```

**Acceptance Criteria:**
- [ ] Compile-time instruction set remains the primary path (no performance regression)
- [ ] Optional `register_custom_instruction()` method allows runtime overrides
- [ ] Custom instructions checked before default lookup table
- [ ] Can be used to test alternate opcodes or 6502 variants
- [ ] All existing tests pass unchanged
- [ ] New test: Register custom instruction, verify it's used instead of default

---

### 10. Document JMP Indirect Wraparound Behavior

**Severity:** MEDIUM (Documentation)  
**File:** `cpu6502/src/cpu6502.cpp:204-217`, `README.md`  
**Problem:** JMP indirect wraparound is intentionally implemented but lacks clear documentation explaining why (historical 6502 hardware bug).

**Suggestion:**
```cpp
// In cpu6502.cpp IND addressing mode:
if(ptr_lo == 0x00FF)
{
    // NMOS 6502 hardware bug: JMP ($xxFF) reads the high byte from $xx00,
    // not $xx+1. The indirect address wraps within the same page.
    // Example: JMP ($00FF) reads low byte from $00FF and high byte from $0000.
    // This is NOT a bug in this emulator—it's the correct NMOS behavior.
    addr_abs_ = (read(ptr & 0xFF00) << 8) | read(ptr);
}
else
{
    // Normal case: indirect address spans two bytes across page boundary
    addr_abs_ = (read(ptr + 1) << 8) | read(ptr);
}
```

**Also update README.md:**
```markdown
### CPU Behavior Notes

#### JMP Indirect Wraparound (NMOS 6502)
The NMOS 6502 has a documented hardware quirk where `JMP ($xxFF)` does not read
the high byte from `$xx00+1` as one might expect. Instead, it wraps within the same page:
- Low byte read from `$xxFF`
- High byte read from `$xx00` (NOT `$xx+1`)

This emulator implements this behavior correctly. See cpu6502.cpp line 209-211.
```

**Acceptance Criteria:**
- [ ] Code comments explain the JMP indirect wraparound behavior in cpu6502.cpp
- [ ] Comments explain this is intentional NMOS hardware behavior, not a bug
- [ ] README.md includes section on CPU quirks/behavior notes
- [ ] Link or reference between code and documentation
- [ ] Test case (from issue #9) verifies and documents the behavior

---

### 11. Add JMP Indirect Wraparound Test Coverage

**Severity:** HIGH (Testing)  
**File:** `test_cpu/test_cpu.cpp`  
**Problem:** Documented feature (JMP indirect wraparound) is implemented but untested; regressions could go unnoticed.

**Suggestion:**
```cpp
void test_jmp_indirect_wraparound() {
    CPU6502 cpu;
    Bus bus;
    cpu.connect(&bus);
    
    // JMP ($00FF) should read:
    // - Low byte from $00FF
    // - High byte from $0000 (wraps within same page, not $0100)
    bus.write(0x00FF, 0x34);  // LSB of target address
    bus.write(0x0000, 0x12);  // MSB of target address (wrapped, not 0x0100)
    bus.write(0x0000, 0x4C);  // Opcode: JMP
    bus.write(0x0001, 0xFF);  // Operand: $00FF
    bus.write(0x0002, 0x00);  // Operand: $FF (high byte)
    
    // Execute: JMP ($00FF)
    do { cpu.clock(); } while (!cpu.complete());
    
    // PC should be 0x1234 (from $0000,$00FF wraparound)
    assert_eq(cpu.get_pc(), 0x1234u, "JMP indirect: PC set from wrapped address");
}
```

**Acceptance Criteria:**
- [ ] Test `test_jmp_indirect_wraparound()` added to test suite
- [ ] Test verifies that JMP ($xxFF) reads high byte from page-boundary-wrapped address
- [ ] Test verifies PC is set to correct address after wraparound
- [ ] Test passes with current implementation
- [ ] Code comment in cpu6502.cpp explains the 6502 hardware bug being emulated

---

### 12. Refactor Disassembler Output - Structured Data Over Strings

**Severity:** LOW (Design)  
**File:** `cpu6502/include/disassembler.h:16-17`, `cpu6502/src/disassembler.cpp`  
**Problem:** Disassembler returns `std::map<uint16_t, std::string>` where strings include formatting (address, mnemonic, operand). Hard to parse programmatically and mixes data with presentation.

**Suggestion:**
```cpp
// New structured representation
struct DisassemblyLine {
    uint16_t address;
    std::string mnemonic;     // e.g., "JMP"
    std::string operand;      // e.g., "$1234" or "#$42"
    std::string addressing_mode;  // e.g., "ABS", "IMM", "IND"
};

class Disassembler {
public:
    std::map<uint16_t, DisassemblyLine> disassemble(
        uint16_t start, uint16_t stop, const Bus& bus) const;
    
    // Helper to format as string if needed
    static std::string format_line(const DisassemblyLine& line) {
        return fmt::format("${:04X}: {} {}",
            line.address, line.mnemonic, line.operand);
    }
};
```

**Acceptance Criteria:**
- [ ] `DisassemblyLine` struct created with address, mnemonic, operand, addressing_mode
- [ ] Disassembler returns `std::map<uint16_t, DisassemblyLine>`
- [ ] CLI and tests updated to use structured data
- [ ] Optional formatting helper `format_line()` for backward compatibility
- [ ] All existing tests pass
- [ ] Output format unchanged (backward compatible)

---

### 13. Document Magic Memory Addresses

**Severity:** LOW (Maintainability)  
**File:** `cpu6502/src/cpu6502.cpp`, `cpu6502/src/bus.cpp`  
**Problem:** Memory addresses (stack page, interrupt vectors) hardcoded throughout; not self-documenting.

**Suggestion:**
```cpp
// cpu6502/include/cpu6502.h - Add constants section
namespace CPU6502Constants {
    // Memory layout
    static constexpr uint16_t STACK_PAGE = 0x0100;
    static constexpr uint16_t STACK_PAGE_END = 0x01FF;
    
    // Interrupt vectors (6502 uses little-endian addresses stored at these locations)
    static constexpr uint16_t RESET_VECTOR = 0xFFFC;  // Reset handler address
    static constexpr uint16_t IRQ_VECTOR = 0xFFFE;    // Maskable interrupt handler
    static constexpr uint16_t NMI_VECTOR = 0xFFFA;    // Non-maskable interrupt handler
}

// Then use in code:
// Old: addr_abs_ = (read(0xFFFC) | (read(0xFFFD) << 8));
// New: addr_abs_ = (read(RESET_VECTOR) | (read(RESET_VECTOR + 1) << 8));
```

**Acceptance Criteria:**
- [ ] Constants defined for all magic memory addresses
- [ ] All hardcoded addresses replaced with named constants
- [ ] Stack operations use `STACK_PAGE` constant
- [ ] Interrupt vector reads use `RESET_VECTOR`, `IRQ_VECTOR`, `NMI_VECTOR`
- [ ] Code is self-documenting; intent is clear without comments

---

### 14. Optimize lookupTable() Hot Path

**Severity:** LOW (Performance)  
**File:** `cpu6502/src/cpu6502.cpp:10-14`  
**Problem:** The static lookup table is called as a function 6+ times per clock cycle, adding indirection on the hot path.

**Suggestion:**
```cpp
// OLD: function call adds indirection
const std::array<INSTRUCTION, 256>& CPU6502::lookupTable() noexcept {
    static const auto lookup = CREATE_LOOKUP_TABLE();
    return lookup;
}
// Usage: const auto& instr = lookupTable()[opcode];  // Function call each time

// NEW: Direct static reference (no indirection)
// In cpu6502.h:
class CPU6502 {
private:
    static const std::array<INSTRUCTION, 256> LOOKUP_TABLE;
};

// In cpu6502.cpp:
const std::array<INSTRUCTION, 256> CPU6502::LOOKUP_TABLE = CREATE_LOOKUP_TABLE();

// Usage: const auto& instr = LOOKUP_TABLE[opcode];  // Direct access
```

**Acceptance Criteria:**
- [ ] Instruction lookup table moved to static class member or module-level variable
- [ ] Function call overhead eliminated from hot path
- [ ] All occurrences of `lookupTable()` replaced with direct access
- [ ] No performance regression in instruction execution
- [ ] All existing tests pass unchanged
- [ ] Optional: Measure performance improvement before/after

---

## IMPLEMENTATION ROADMAP

### Phase 1: Critical Bugs (1-2 sprints)
1. Fix disassembler uint16_t wraparound → **PR #1**
2. Add CLI hex input validation → **PR #1**
3. Add CLI file path validation → **PR #1**

### Phase 2: Core Architecture (2-3 sprints)
4. Encapsulate CPU registers + update all tests → **PR #2**
5. Introduce Bus device abstraction → **PR #3**
6. Implement illegal instruction tracking → **PR #3**

### Phase 3: Testing & Documentation (2-3 sprints)
7. Add JMP indirect wraparound test + documentation → **PR #4**
8. Document decimal ADC/SBC algorithm → **PR #4**
9. Add magic memory constants → **PR #4**
10. Support runtime instruction set extension → **PR #5**
11. Encapsulate clock/complete pattern (optional, nice-to-have) → **PR #5**

### Phase 4: Design Refactoring (1 sprint)
12. Address bReadOnly semantics → **PR #6**
13. Refactor disassembler to structured output → **PR #6**

### Phase 5: Performance & Polish (1 sprint)
14. Optimize lookupTable() hot path → **PR #7**

---

## Definition of Done

Each PR must:
- [ ] All acceptance criteria from respective issues are met
- [ ] All existing tests pass
- [ ] New tests added where specified
- [ ] Code compiles without warnings (C++23)
- [ ] Commit message references this plan
- [ ] Changes are reviewed and approved

---

## Risk Assessment

**Low Risk:**
- Disassembler uint16_t fix (isolated, single file)
- CLI input validation (isolated error handling)
- Adding constants for magic addresses
- Adding test coverage

**Medium Risk:**
- CPU register encapsulation (requires updating all tests, but tests are isolated)
- Bus device abstraction (larger refactor, needs careful integration)

**High Risk:**
- None; changes are additive or isolated refactors

---

## Success Criteria

After all phases complete:
- [ ] Zero unhandled exceptions in CLI (hex parsing, path validation)
- [ ] No more infinite loops in disassembler
- [ ] JMP indirect wraparound explicitly tested and documented
- [ ] CPU registers are private; tests use getters/setters
- [ ] Bus supports memory-mapped device attachment without modification
- [ ] Decimal mode arithmetic is self-documenting with detailed comments
- [ ] New developers can understand codebase without external research
- [ ] All original functionality preserved; no behavior changes
