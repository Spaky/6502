#pragma once
#include <cstdint>
#include <expected>
#include <array>

#ifdef CPU6502_EXPORTS
#define CPU_CORE_API __declspec(dllexport)
#else
#define CPU_CORE_API __declspec(dllimport)
#endif

enum class BusError
{
    OutOfBounds
};

/// Backing memory for the emulated address space is 64 KiB.
class CPU_CORE_API Bus
{
public:
    /// Read a byte from memory. When bReadOnly is true, the access is treated as
    /// a non-mutating probe for debugger-style reads.
    uint8_t read(uint16_t addr, bool bReadOnly = false) const noexcept;
    /// Write a byte to memory.
    void write(uint16_t addr, uint8_t data) noexcept;
    /// Reset all memory to zero.
    void clear() noexcept;

    /// Perform a read that matches the 16-bit address space contract.
    [[nodiscard]] std::expected<uint8_t, BusError> safeRead(uint16_t addr) const noexcept;

    static constexpr uint32_t RAM_SIZE = 65536;

private:
    std::array<uint8_t, RAM_SIZE> ram_ = {};
};
