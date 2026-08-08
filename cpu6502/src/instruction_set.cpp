#include <array>
#include <cstdint>
#include <string_view>

#include "../include/instruction_set.h"
#include "../include/cpu6502.h"

namespace
{
    template <auto Operate, auto AddrModeFn>
    constexpr INSTRUCTION Instr(std::string_view name, AddrMode mode_id, uint8_t cycles) noexcept
    {
        return INSTRUCTION{name, Operate, AddrModeFn, cycles, mode_id};
    }
}

constexpr std::array<INSTRUCTION, 256> CREATE_LOOKUP_TABLE() noexcept
{
    return {{
            // 0x0X
            Instr<&CPU6502::BRK, &CPU6502::IMP>("BRK", AddrMode::IMP, 7), Instr<&CPU6502::ORA, &CPU6502::IZX>("ORA", AddrMode::IZX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
            Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 3), Instr<&CPU6502::ORA, &CPU6502::ZP0>("ORA", AddrMode::ZP0, 3), Instr<&CPU6502::ASL, &CPU6502::ZP0>("ASL", AddrMode::ZP0, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
            Instr<&CPU6502::PHP, &CPU6502::IMP>("PHP", AddrMode::IMP, 3), Instr<&CPU6502::ORA, &CPU6502::IMM>("ORA", AddrMode::IMM, 2), Instr<&CPU6502::ASL, &CPU6502::IMP>("ASL", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
            Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::ORA, &CPU6502::ABS>("ORA", AddrMode::ABS, 4), Instr<&CPU6502::ASL, &CPU6502::ABS>("ASL", AddrMode::ABS, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
            // 0x1X
        Instr<&CPU6502::BPL, &CPU6502::REL>("BPL", AddrMode::REL, 2), Instr<&CPU6502::ORA, &CPU6502::IZY>("ORA", AddrMode::IZY, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::ORA, &CPU6502::ZPX>("ORA", AddrMode::ZPX, 4), Instr<&CPU6502::ASL, &CPU6502::ZPX>("ASL", AddrMode::ZPX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::CLC, &CPU6502::IMP>("CLC", AddrMode::IMP, 2), Instr<&CPU6502::ORA, &CPU6502::ABY>("ORA", AddrMode::ABY, 4), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::ORA, &CPU6502::ABX>("ORA", AddrMode::ABX, 4), Instr<&CPU6502::ASL, &CPU6502::ABX>("ASL", AddrMode::ABX, 7), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        // 0x2X
        Instr<&CPU6502::JSR, &CPU6502::ABS>("JSR", AddrMode::ABS, 6), Instr<&CPU6502::AND, &CPU6502::IZX>("AND", AddrMode::IZX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::BIT, &CPU6502::ZP0>("BIT", AddrMode::ZP0, 3), Instr<&CPU6502::AND, &CPU6502::ZP0>("AND", AddrMode::ZP0, 3), Instr<&CPU6502::ROL, &CPU6502::ZP0>("ROL", AddrMode::ZP0, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        Instr<&CPU6502::PLP, &CPU6502::IMP>("PLP", AddrMode::IMP, 4), Instr<&CPU6502::AND, &CPU6502::IMM>("AND", AddrMode::IMM, 2), Instr<&CPU6502::ROL, &CPU6502::IMP>("ROL", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
        Instr<&CPU6502::BIT, &CPU6502::ABS>("BIT", AddrMode::ABS, 4), Instr<&CPU6502::AND, &CPU6502::ABS>("AND", AddrMode::ABS, 4), Instr<&CPU6502::ROL, &CPU6502::ABS>("ROL", AddrMode::ABS, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        // 0x3X
        Instr<&CPU6502::BMI, &CPU6502::REL>("BMI", AddrMode::REL, 2), Instr<&CPU6502::AND, &CPU6502::IZY>("AND", AddrMode::IZY, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::AND, &CPU6502::ZPX>("AND", AddrMode::ZPX, 4), Instr<&CPU6502::ROL, &CPU6502::ZPX>("ROL", AddrMode::ZPX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::SEC, &CPU6502::IMP>("SEC", AddrMode::IMP, 2), Instr<&CPU6502::AND, &CPU6502::ABY>("AND", AddrMode::ABY, 4), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::AND, &CPU6502::ABX>("AND", AddrMode::ABX, 4), Instr<&CPU6502::ROL, &CPU6502::ABX>("ROL", AddrMode::ABX, 7), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        // 0x4X
        Instr<&CPU6502::RTI, &CPU6502::IMP>("RTI", AddrMode::IMP, 6), Instr<&CPU6502::EOR, &CPU6502::IZX>("EOR", AddrMode::IZX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 3), Instr<&CPU6502::EOR, &CPU6502::ZP0>("EOR", AddrMode::ZP0, 3), Instr<&CPU6502::LSR, &CPU6502::ZP0>("LSR", AddrMode::ZP0, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        Instr<&CPU6502::PHA, &CPU6502::IMP>("PHA", AddrMode::IMP, 3), Instr<&CPU6502::EOR, &CPU6502::IMM>("EOR", AddrMode::IMM, 2), Instr<&CPU6502::LSR, &CPU6502::IMP>("LSR", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
        Instr<&CPU6502::JMP, &CPU6502::ABS>("JMP", AddrMode::ABS, 3), Instr<&CPU6502::EOR, &CPU6502::ABS>("EOR", AddrMode::ABS, 4), Instr<&CPU6502::LSR, &CPU6502::ABS>("LSR", AddrMode::ABS, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        // 0x5X
        Instr<&CPU6502::BVC, &CPU6502::REL>("BVC", AddrMode::REL, 2), Instr<&CPU6502::EOR, &CPU6502::IZY>("EOR", AddrMode::IZY, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::EOR, &CPU6502::ZPX>("EOR", AddrMode::ZPX, 4), Instr<&CPU6502::LSR, &CPU6502::ZPX>("LSR", AddrMode::ZPX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::CLI, &CPU6502::IMP>("CLI", AddrMode::IMP, 2), Instr<&CPU6502::EOR, &CPU6502::ABY>("EOR", AddrMode::ABY, 4), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::EOR, &CPU6502::ABX>("EOR", AddrMode::ABX, 4), Instr<&CPU6502::LSR, &CPU6502::ABX>("LSR", AddrMode::ABX, 7), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        // 0x6X
        Instr<&CPU6502::RTS, &CPU6502::IMP>("RTS", AddrMode::IMP, 6), Instr<&CPU6502::ADC, &CPU6502::IZX>("ADC", AddrMode::IZX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 3), Instr<&CPU6502::ADC, &CPU6502::ZP0>("ADC", AddrMode::ZP0, 3), Instr<&CPU6502::ROR, &CPU6502::ZP0>("ROR", AddrMode::ZP0, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        Instr<&CPU6502::PLA, &CPU6502::IMP>("PLA", AddrMode::IMP, 4), Instr<&CPU6502::ADC, &CPU6502::IMM>("ADC", AddrMode::IMM, 2), Instr<&CPU6502::ROR, &CPU6502::IMP>("ROR", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
        Instr<&CPU6502::JMP, &CPU6502::IND>("JMP", AddrMode::IND, 5), Instr<&CPU6502::ADC, &CPU6502::ABS>("ADC", AddrMode::ABS, 4), Instr<&CPU6502::ROR, &CPU6502::ABS>("ROR", AddrMode::ABS, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        // 0x7X
        Instr<&CPU6502::BVS, &CPU6502::REL>("BVS", AddrMode::REL, 2), Instr<&CPU6502::ADC, &CPU6502::IZY>("ADC", AddrMode::IZY, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::ADC, &CPU6502::ZPX>("ADC", AddrMode::ZPX, 4), Instr<&CPU6502::ROR, &CPU6502::ZPX>("ROR", AddrMode::ZPX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::SEI, &CPU6502::IMP>("SEI", AddrMode::IMP, 2), Instr<&CPU6502::ADC, &CPU6502::ABY>("ADC", AddrMode::ABY, 4), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::ADC, &CPU6502::ABX>("ADC", AddrMode::ABX, 4), Instr<&CPU6502::ROR, &CPU6502::ABX>("ROR", AddrMode::ABX, 7), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        // 0x8X
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::STA, &CPU6502::IZX>("STA", AddrMode::IZX, 6), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::STY, &CPU6502::ZP0>("STY", AddrMode::ZP0, 3), Instr<&CPU6502::STA, &CPU6502::ZP0>("STA", AddrMode::ZP0, 3), Instr<&CPU6502::STX, &CPU6502::ZP0>("STX", AddrMode::ZP0, 3), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 3),
        Instr<&CPU6502::DEY, &CPU6502::IMP>("DEY", AddrMode::IMP, 2), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::TXA, &CPU6502::IMP>("TXA", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
        Instr<&CPU6502::STY, &CPU6502::ABS>("STY", AddrMode::ABS, 4), Instr<&CPU6502::STA, &CPU6502::ABS>("STA", AddrMode::ABS, 4), Instr<&CPU6502::STX, &CPU6502::ABS>("STX", AddrMode::ABS, 4), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 4),
        // 0x9X
        Instr<&CPU6502::BCC, &CPU6502::REL>("BCC", AddrMode::REL, 2), Instr<&CPU6502::STA, &CPU6502::IZY>("STA", AddrMode::IZY, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::STY, &CPU6502::ZPX>("STY", AddrMode::ZPX, 4), Instr<&CPU6502::STA, &CPU6502::ZPX>("STA", AddrMode::ZPX, 4), Instr<&CPU6502::STX, &CPU6502::ZPY>("STX", AddrMode::ZPY, 4), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 4),
        Instr<&CPU6502::TYA, &CPU6502::IMP>("TYA", AddrMode::IMP, 2), Instr<&CPU6502::STA, &CPU6502::ABY>("STA", AddrMode::ABY, 5), Instr<&CPU6502::TXS, &CPU6502::IMP>("TXS", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 5), Instr<&CPU6502::STA, &CPU6502::ABX>("STA", AddrMode::ABX, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        // 0xAX
        Instr<&CPU6502::LDY, &CPU6502::IMM>("LDY", AddrMode::IMM, 2), Instr<&CPU6502::LDA, &CPU6502::IZX>("LDA", AddrMode::IZX, 6), Instr<&CPU6502::LDX, &CPU6502::IMM>("LDX", AddrMode::IMM, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::LDY, &CPU6502::ZP0>("LDY", AddrMode::ZP0, 3), Instr<&CPU6502::LDA, &CPU6502::ZP0>("LDA", AddrMode::ZP0, 3), Instr<&CPU6502::LDX, &CPU6502::ZP0>("LDX", AddrMode::ZP0, 3), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 3),
        Instr<&CPU6502::TAY, &CPU6502::IMP>("TAY", AddrMode::IMP, 2), Instr<&CPU6502::LDA, &CPU6502::IMM>("LDA", AddrMode::IMM, 2), Instr<&CPU6502::TAX, &CPU6502::IMP>("TAX", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
        Instr<&CPU6502::LDY, &CPU6502::ABS>("LDY", AddrMode::ABS, 4), Instr<&CPU6502::LDA, &CPU6502::ABS>("LDA", AddrMode::ABS, 4), Instr<&CPU6502::LDX, &CPU6502::ABS>("LDX", AddrMode::ABS, 4), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 4),
        // 0xBX
        Instr<&CPU6502::BCS, &CPU6502::REL>("BCS", AddrMode::REL, 2), Instr<&CPU6502::LDA, &CPU6502::IZY>("LDA", AddrMode::IZY, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        Instr<&CPU6502::LDY, &CPU6502::ZPX>("LDY", AddrMode::ZPX, 4), Instr<&CPU6502::LDA, &CPU6502::ZPX>("LDA", AddrMode::ZPX, 4), Instr<&CPU6502::LDX, &CPU6502::ZPY>("LDX", AddrMode::ZPY, 4), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 4),
        Instr<&CPU6502::CLV, &CPU6502::IMP>("CLV", AddrMode::IMP, 2), Instr<&CPU6502::LDA, &CPU6502::ABY>("LDA", AddrMode::ABY, 4), Instr<&CPU6502::TSX, &CPU6502::IMP>("TSX", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 4),
        Instr<&CPU6502::LDY, &CPU6502::ABX>("LDY", AddrMode::ABX, 4), Instr<&CPU6502::LDA, &CPU6502::ABX>("LDA", AddrMode::ABX, 4), Instr<&CPU6502::LDX, &CPU6502::ABY>("LDX", AddrMode::ABY, 4), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 4),
        // 0xCX
        Instr<&CPU6502::CPY, &CPU6502::IMM>("CPY", AddrMode::IMM, 2), Instr<&CPU6502::CMP, &CPU6502::IZX>("CMP", AddrMode::IZX, 6), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::CPY, &CPU6502::ZP0>("CPY", AddrMode::ZP0, 3), Instr<&CPU6502::CMP, &CPU6502::ZP0>("CMP", AddrMode::ZP0, 3), Instr<&CPU6502::DEC, &CPU6502::ZP0>("DEC", AddrMode::ZP0, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        Instr<&CPU6502::INY, &CPU6502::IMP>("INY", AddrMode::IMP, 2), Instr<&CPU6502::CMP, &CPU6502::IMM>("CMP", AddrMode::IMM, 2), Instr<&CPU6502::DEX, &CPU6502::IMP>("DEX", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
        Instr<&CPU6502::CPY, &CPU6502::ABS>("CPY", AddrMode::ABS, 4), Instr<&CPU6502::CMP, &CPU6502::ABS>("CMP", AddrMode::ABS, 4), Instr<&CPU6502::DEC, &CPU6502::ABS>("DEC", AddrMode::ABS, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        // 0xDX
        Instr<&CPU6502::BNE, &CPU6502::REL>("BNE", AddrMode::REL, 2), Instr<&CPU6502::CMP, &CPU6502::IZY>("CMP", AddrMode::IZY, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::CMP, &CPU6502::ZPX>("CMP", AddrMode::ZPX, 4), Instr<&CPU6502::DEC, &CPU6502::ZPX>("DEC", AddrMode::ZPX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::CLD, &CPU6502::IMP>("CLD", AddrMode::IMP, 2), Instr<&CPU6502::CMP, &CPU6502::ABY>("CMP", AddrMode::ABY, 4), Instr<&CPU6502::NOP, &CPU6502::IMP>("NOP", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::CMP, &CPU6502::ABX>("CMP", AddrMode::ABX, 4), Instr<&CPU6502::DEC, &CPU6502::ABX>("DEC", AddrMode::ABX, 7), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        // 0xEX
        Instr<&CPU6502::CPX, &CPU6502::IMM>("CPX", AddrMode::IMM, 2), Instr<&CPU6502::SBC, &CPU6502::IZX>("SBC", AddrMode::IZX, 6), Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::CPX, &CPU6502::ZP0>("CPX", AddrMode::ZP0, 3), Instr<&CPU6502::SBC, &CPU6502::ZP0>("SBC", AddrMode::ZP0, 3), Instr<&CPU6502::INC, &CPU6502::ZP0>("INC", AddrMode::ZP0, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 5),
        Instr<&CPU6502::INX, &CPU6502::IMP>("INX", AddrMode::IMP, 2), Instr<&CPU6502::SBC, &CPU6502::IMM>("SBC", AddrMode::IMM, 2), Instr<&CPU6502::NOP, &CPU6502::IMP>("NOP", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2),
        Instr<&CPU6502::CPX, &CPU6502::ABS>("CPX", AddrMode::ABS, 4), Instr<&CPU6502::SBC, &CPU6502::ABS>("SBC", AddrMode::ABS, 4), Instr<&CPU6502::INC, &CPU6502::ABS>("INC", AddrMode::ABS, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        // 0xFX
        Instr<&CPU6502::BEQ, &CPU6502::REL>("BEQ", AddrMode::REL, 2), Instr<&CPU6502::SBC, &CPU6502::IZY>("SBC", AddrMode::IZY, 5), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 8),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::SBC, &CPU6502::ZPX>("SBC", AddrMode::ZPX, 4), Instr<&CPU6502::INC, &CPU6502::ZPX>("INC", AddrMode::ZPX, 6), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 6),
        Instr<&CPU6502::SED, &CPU6502::IMP>("SED", AddrMode::IMP, 2), Instr<&CPU6502::SBC, &CPU6502::ABY>("SBC", AddrMode::ABY, 4), Instr<&CPU6502::NOP, &CPU6502::IMP>("NOP", AddrMode::IMP, 2), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        Instr<&CPU6502::NOP, &CPU6502::IMP>("???", AddrMode::IMP, 4), Instr<&CPU6502::SBC, &CPU6502::ABX>("SBC", AddrMode::ABX, 4), Instr<&CPU6502::INC, &CPU6502::ABX>("INC", AddrMode::ABX, 7), Instr<&CPU6502::XXX, &CPU6502::IMP>("???", AddrMode::IMP, 7),
        }};
}
