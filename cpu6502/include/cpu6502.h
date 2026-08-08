#pragma once
#include <cstdint>
#include <array>
#include <utility>
#include "instruction_set.h"

#ifdef CPU6502_EXPORTS
#define CPU_CORE_API __declspec(dllexport)
#else
#define CPU_CORE_API __declspec(dllimport)
#endif

class Bus;

enum class FLAGS6502 : uint8_t
{
    C = (1 << 0), // Carry
    Z = (1 << 1), // Zero
    I = (1 << 2), // Interrupt Disable
    D = (1 << 3), // Decimal Mode
    B = (1 << 4), // Break
    U = (1 << 5), // Unused
    V = (1 << 6), // Overflow
    N = (1 << 7)  // Negative
};

class CPU_CORE_API CPU6502
{
public:
    /// Construct the CPU.
    CPU6502() noexcept = default;
    ~CPU6502() noexcept = default;

    /// Registers are public for debugger and test access.
    uint8_t a_ = 0x00;
    uint8_t x_ = 0x00;
    uint8_t y_ = 0x00;
    uint8_t stkp_ = 0x00;
    uint16_t pc_ = 0x0000;
    uint8_t status_ = 0x00;

    /// Attach the CPU to its system bus.
    void connectBus(Bus* n) noexcept
    {
        bus_ = n;
    }

    /// Reset the CPU using the reset vector at $FFFC/$FFFD.
    void reset() noexcept;
    /// Trigger a maskable interrupt.
    void irq() noexcept;
    /// Trigger a non-maskable interrupt.
    void nmi() noexcept;
    /// Advance execution by one clock.
    void clock() noexcept;

    /// Return true when the current instruction has finished executing.
    [[nodiscard]] bool complete() const noexcept;
    /// Read a status flag.
    [[nodiscard]] uint8_t getFlag(FLAGS6502 f) const noexcept;
    /// Set or clear a status flag.
    void setFlag(FLAGS6502 f, bool v) noexcept;

    friend constexpr std::array<INSTRUCTION, 256> CREATE_LOOKUP_TABLE() noexcept;

private:
    static const std::array<INSTRUCTION, 256>& lookupTable() noexcept;

    Bus* bus_ = nullptr;

    [[nodiscard]] uint8_t read(uint16_t addr) const noexcept;
    void write(uint16_t addr, uint8_t d) noexcept;

    uint8_t fetched_ = 0x00;
    uint16_t addr_abs_ = 0x0000;
    uint16_t addr_rel_ = 0x0000;
    uint8_t opcode_ = 0x00;
    uint8_t cycles_ = 0;
    uint32_t clock_count_ = 0;

    uint8_t fetch() noexcept;
    uint8_t branch(bool condition) noexcept;

    // Addressing modes
    uint8_t IMP() noexcept; uint8_t IMM() noexcept; uint8_t ZP0() noexcept; uint8_t ZPX() noexcept;
    uint8_t ZPY() noexcept; uint8_t REL() noexcept; uint8_t ABS() noexcept; uint8_t ABX() noexcept;
    uint8_t ABY() noexcept; uint8_t IND() noexcept; uint8_t IZX() noexcept; uint8_t IZY() noexcept;

    // Opcodes
    uint8_t ADC() noexcept; uint8_t AND() noexcept; uint8_t ASL() noexcept; uint8_t BCC() noexcept;
    uint8_t BCS() noexcept; uint8_t BEQ() noexcept; uint8_t BIT() noexcept; uint8_t BMI() noexcept;
    uint8_t BNE() noexcept; uint8_t BPL() noexcept; uint8_t BRK() noexcept; uint8_t BVC() noexcept;
    uint8_t BVS() noexcept; uint8_t CLC() noexcept; uint8_t CLD() noexcept; uint8_t CLI() noexcept;
    uint8_t CLV() noexcept; uint8_t CMP() noexcept; uint8_t CPX() noexcept; uint8_t CPY() noexcept;
    uint8_t DEC() noexcept; uint8_t DEX() noexcept; uint8_t DEY() noexcept; uint8_t EOR() noexcept;
    uint8_t INC() noexcept; uint8_t INX() noexcept; uint8_t INY() noexcept; uint8_t JMP() noexcept;
    uint8_t JSR() noexcept; uint8_t LDA() noexcept; uint8_t LDX() noexcept; uint8_t LDY() noexcept;
    uint8_t LSR() noexcept; uint8_t NOP() noexcept; uint8_t ORA() noexcept; uint8_t PHA() noexcept;
    uint8_t PHP() noexcept; uint8_t PLA() noexcept; uint8_t PLP() noexcept; uint8_t ROL() noexcept;
    uint8_t ROR() noexcept; uint8_t RTI() noexcept; uint8_t RTS() noexcept; uint8_t SBC() noexcept;
    uint8_t SEC() noexcept; uint8_t SED() noexcept; uint8_t SEI() noexcept; uint8_t STA() noexcept;
    uint8_t STX() noexcept; uint8_t STY() noexcept; uint8_t TAX() noexcept; uint8_t TAY() noexcept;
    uint8_t TSX() noexcept; uint8_t TXA() noexcept; uint8_t TXS() noexcept; uint8_t TYA() noexcept;
    uint8_t XXX() noexcept;

};
