#include <cstdint>
#include <expected>
#include <gsl/gsl>
#include "../include/bus.h"

// addr is uint16_t, so it always indexes the full 64 KiB address space.
#pragma warning(push)
#pragma warning(disable:26446 26482)
std::expected<uint8_t, BusError> Bus::safeRead(uint16_t addr) const noexcept
{
    return ram_[addr];
}

uint8_t Bus::read(uint16_t addr, [[maybe_unused]] bool bReadOnly) const noexcept
{
    return ram_[addr];
}


void Bus::write(uint16_t addr, uint8_t data) noexcept
{
    ram_[addr] = data;
}
#pragma warning(pop)

void Bus::clear() noexcept
{
    ram_.fill(0);
}
