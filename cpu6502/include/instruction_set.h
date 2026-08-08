#pragma once
#include <cstdint>
#include <string_view>
#include <array>

#ifdef CPU6502_EXPORTS
#define CPU_CORE_API __declspec(dllexport)
#else
#define CPU_CORE_API __declspec(dllimport)
#endif

/// Addressing mode identifiers used by the disassembler without CPU internals.
enum class AddrMode : uint8_t
{
    IMP,
    IMM,
    ZP0,
    ZPX,
    ZPY,
    REL,
    ABS,
    ABX,
    ABY,
    IND,
    IZX,
    IZY
};

class CPU6502;

struct INSTRUCTION
{
    /// Mnemonic text shown by the disassembler.
    std::string_view name_;

    /// CPU operation implementation for the opcode.
    uint8_t(CPU6502::* operate)() noexcept = nullptr;
    /// Addressing-mode implementation for the opcode.
    uint8_t(CPU6502::* addrmode)() noexcept = nullptr;

    /// Base cycle count for the opcode.
    uint8_t cycles_ = 0;
    /// Stable addressing mode identifier for tooling and disassembly.
    AddrMode mode_id_ = AddrMode::IMP;
};

/// Build the 6502 opcode lookup table.
CPU_CORE_API constexpr std::array<INSTRUCTION, 256> CREATE_LOOKUP_TABLE() noexcept;
