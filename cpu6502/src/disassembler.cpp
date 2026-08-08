#include "../include/disassembler.h"
#include "../include/bus.h"
#include "../include/instruction_set.h"
#include <format>
#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <gsl/gsl>

static const auto s_lookup = CREATE_LOOKUP_TABLE();

namespace
{
    using DisassemblyEntry = std::pair<uint16_t, std::string>;

    [[nodiscard]] uint16_t read_u16(const Bus& bus, uint16_t& addr) noexcept
    {
        const auto lo = bus.read(gsl::narrow_cast<uint16_t>(addr++), true);
        const auto hi = bus.read(gsl::narrow_cast<uint16_t>(addr++), true);
        return gsl::narrow_cast<uint16_t>((gsl::narrow_cast<uint16_t>(hi) << 8) | lo);
    }
}

std::map<uint16_t, std::string> Disassembler::disassemble(const Bus& bus, uint16_t start, uint16_t stop)
{
    std::vector<DisassemblyEntry> entries;
    entries.reserve(static_cast<size_t>(stop - start) + 1);
    auto addr = start;
    while(addr <= static_cast<uint32_t>(stop))
    {
        const auto line_addr = gsl::narrow_cast<uint16_t>(addr);
        const auto opcode = bus.read(gsl::narrow_cast<uint16_t>(addr), true);
        const auto& instr = gsl::at(s_lookup, opcode);
        addr++;

        std::string operand;

        switch(instr.mode_id_)
        {
            using enum AddrMode;
        case IMM:
            operand = std::format("#${:02X}",
                bus.read(gsl::narrow_cast<uint16_t>(addr++), true));
            break;
        case ZP0:
            operand = std::format("${:02X}",
                bus.read(gsl::narrow_cast<uint16_t>(addr++), true));
            break;
        case ZPX:
            operand = std::format("${:02X},X",
                bus.read(gsl::narrow_cast<uint16_t>(addr++), true));
            break;
        case ZPY:
            operand = std::format("${:02X},Y",
                bus.read(gsl::narrow_cast<uint16_t>(addr++), true));
            break;
        case ABS:
            operand = std::format("${:04X}", read_u16(bus, addr));
            break;
        case ABX:
            operand = std::format("${:04X},X", read_u16(bus, addr));
            break;
        case ABY:
            operand = std::format("${:04X},Y", read_u16(bus, addr));
            break;
        case IND:
            operand = std::format("(${:04X})", read_u16(bus, addr));
            break;
        case IZX:
            operand = std::format("(${:02X},X)",
                bus.read(gsl::narrow_cast<uint16_t>(addr++), true));
            break;
        case IZY:
            operand = std::format("(${:02X}),Y",
                bus.read(gsl::narrow_cast<uint16_t>(addr++), true));
            break;
        case REL:
            operand = std::format("${:04X}",
                gsl::narrow_cast<uint16_t>(static_cast<int32_t>(addr) + static_cast<int8_t>(bus.read(gsl::narrow_cast<uint16_t>(addr++), true))));
            break;
        default:
            break; // IMP — no operand
        }
        entries.emplace_back(line_addr, std::format("${:04X}: {:3s} {}", line_addr, instr.name_, operand));
    }
    return {entries.begin(), entries.end()};
}
