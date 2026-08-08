#include <array>
#include <cstdint>
#include <utility>
#include <gsl/gsl>

#include "../include/cpu6502.h"
#include "../include/bus.h"
#include "../include/instruction_set.h"

const std::array<INSTRUCTION, 256>& CPU6502::lookupTable() noexcept
{
    static const auto lookup = CREATE_LOOKUP_TABLE();
    return lookup;
}

uint8_t CPU6502::read(uint16_t addr) const noexcept
{
    return bus_ != nullptr ? bus_->read(addr, false) : 0;
}

void CPU6502::write(uint16_t addr, uint8_t d) noexcept
{
    if(bus_ != nullptr)
    {
        bus_->write(addr, d);
    }
}

uint8_t CPU6502::getFlag(FLAGS6502 f) const noexcept
{
    return ((status_ & std::to_underlying(f)) > 0) ? 1 : 0;
}

void CPU6502::setFlag(FLAGS6502 f, bool v) noexcept
{
    if(v)
    {
        status_ |= std::to_underlying(f);
    }
    else
    {
        status_ &= ~std::to_underlying(f);
    }
}

// opcode_ is uint8_t, so it always indexes the full lookup table size.
#pragma warning(push)
#pragma warning(disable:26482)
void CPU6502::clock() noexcept
{
    if(cycles_ == 0)
    {
        const auto& lookup = lookupTable();
        opcode_ = read(pc_);
        setFlag(FLAGS6502::U, true);
        pc_++;
        const auto& instruction = lookup[opcode_];
        cycles_ = instruction.cycles_;
        const auto c1 = (this->*instruction.addrmode)();
        const auto c2 = (this->*instruction.operate)();
        cycles_ += (c1 | c2);
        setFlag(FLAGS6502::U, true);
    }
    clock_count_++;
    cycles_--;
}
#pragma warning(pop)

bool CPU6502::complete() const noexcept
{
    return cycles_ == 0;
}

void CPU6502::reset() noexcept
{
    addr_abs_ = 0xFFFC;
    const uint16_t lo = read(addr_abs_ + 0);
    const uint16_t hi = read(addr_abs_ + 1);
    pc_ = (hi << 8) | lo;
    a_ = 0;
    x_ = 0;
    y_ = 0;
    stkp_ = 0xFD;
    status_ = 0x00 | std::to_underlying(FLAGS6502::U);
    addr_rel_ = 0x0000;
    addr_abs_ = 0x0000;
    fetched_ = 0x00;
    cycles_ = 8;
}

void CPU6502::irq() noexcept
{
    if(getFlag(FLAGS6502::I) == 0)
    {
        write(0x0100 + stkp_, (pc_ >> 8) & 0x00FF);
        stkp_--;
        write(0x0100 + stkp_, pc_ & 0x00FF);
        stkp_--;
        setFlag(FLAGS6502::B, false);
        setFlag(FLAGS6502::U, true);
        setFlag(FLAGS6502::I, true);
        write(0x0100 + stkp_, status_);
        stkp_--;
        addr_abs_ = 0xFFFE;
        pc_ = static_cast<uint16_t>(read(addr_abs_)) | (static_cast<uint16_t>(read(addr_abs_ + 1)) << 8);
        cycles_ = 7;
    }
}

void CPU6502::nmi() noexcept
{
    write(0x0100 + stkp_, (pc_ >> 8) & 0x00FF);
    stkp_--;
    write(0x0100 + stkp_, pc_ & 0x00FF);
    stkp_--;
    setFlag(FLAGS6502::B, false);
    setFlag(FLAGS6502::U, true);
    setFlag(FLAGS6502::I, true);
    write(0x0100 + stkp_, status_);
    stkp_--;
    addr_abs_ = 0xFFFA;
    pc_ = static_cast<uint16_t>(read(addr_abs_)) | (static_cast<uint16_t>(read(addr_abs_ + 1)) << 8);
    cycles_ = 8;
}

// opcode_ is uint8_t, so it always indexes the full lookup table size.
#pragma warning(push)
#pragma warning(disable:26482)
uint8_t CPU6502::fetch() noexcept
{
    if(lookupTable()[opcode_].addrmode != &CPU6502::IMP)
    {
        fetched_ = read(addr_abs_);
    }
    return fetched_;
}
#pragma warning(pop)

// ---------- Addressing modes ----------
uint8_t CPU6502::IMP() noexcept
{
    fetched_ = a_;
    return 0;
}

uint8_t CPU6502::IMM() noexcept
{
    addr_abs_ = pc_++;
    return 0;
}

uint8_t CPU6502::ZP0() noexcept
{
    addr_abs_ = read(pc_++) & 0x00FF;
    return 0;
}

uint8_t CPU6502::ZPX() noexcept
{
    addr_abs_ = (read(pc_++) + x_) & 0x00FF;
    return 0;
}

uint8_t CPU6502::ZPY() noexcept
{
    addr_abs_ = (read(pc_++) + y_) & 0x00FF;
    return 0;
}

uint8_t CPU6502::REL() noexcept
{
    addr_rel_ = read(pc_++);
    if(addr_rel_ & 0x80)
    {
        addr_rel_ |= 0xFF00;
    }
    return 0;
}

uint8_t CPU6502::ABS() noexcept
{
    const uint16_t lo = read(pc_++);
    const uint16_t hi = read(pc_++);
    addr_abs_ = (hi << 8) | lo;
    return 0;
}

uint8_t CPU6502::ABX() noexcept
{
    const uint16_t lo = read(pc_++);
    const uint16_t hi = read(pc_++);
    addr_abs_ = ((hi << 8) | lo) + x_;
    return (addr_abs_ & 0xFF00) != (hi << 8) ? 1 : 0;
}

uint8_t CPU6502::ABY() noexcept
{
    const uint16_t lo = read(pc_++);
    const uint16_t hi = read(pc_++);
    addr_abs_ = ((hi << 8) | lo) + y_;
    return (addr_abs_ & 0xFF00) != (hi << 8) ? 1 : 0;
}

uint8_t CPU6502::IND() noexcept
{
    const uint16_t ptr_lo = read(pc_++);
    const uint16_t ptr_hi = read(pc_++);
    const uint16_t ptr = (ptr_hi << 8) | ptr_lo;
    if(ptr_lo == 0x00FF)
    {
        addr_abs_ = (read(ptr & 0xFF00) << 8) | read(ptr);
    }
    else
    {
        addr_abs_ = (read(ptr + 1) << 8) | read(ptr);
    }
    return 0;
}

uint8_t CPU6502::IZX() noexcept
{
    const uint16_t t = read(pc_++);
    const uint16_t lo = read((t + x_) & 0x00FF);
    const uint16_t hi = read((t + x_ + 1) & 0x00FF);
    addr_abs_ = (hi << 8) | lo;
    return 0;
}

uint8_t CPU6502::IZY() noexcept
{
    const uint16_t t = read(pc_++);
    const uint16_t lo = read(t & 0x00FF);
    const uint16_t hi = read((t + 1) & 0x00FF);
    addr_abs_ = ((hi << 8) | lo) + y_;
    return (addr_abs_ & 0xFF00) != (hi << 8) ? 1 : 0;
}

// ---------- Opcodes ----------
uint8_t CPU6502::ADC() noexcept
{
    fetch();
    const auto carry_in = getFlag(FLAGS6502::C);
    const uint16_t binary_sum = static_cast<uint16_t>(a_) + static_cast<uint16_t>(fetched_) + static_cast<uint16_t>(carry_in);

    setFlag(FLAGS6502::V, (~(static_cast<uint16_t>(a_) ^ static_cast<uint16_t>(fetched_)) & (static_cast<uint16_t>(a_) ^ binary_sum)) & 0x0080);

    if(getFlag(FLAGS6502::D) == 1)
    {
        auto decimal_sum = binary_sum;
        if(((a_ & 0x0F) + (fetched_ & 0x0F) + carry_in) > 9)
        {
            decimal_sum += 0x06;
        }
        if(decimal_sum > 0x99)
        {
            decimal_sum += 0x60;
        }

        setFlag(FLAGS6502::C, decimal_sum > 0x99);
        a_ = gsl::narrow_cast<uint8_t>(decimal_sum & 0x00FF);
    }
    else
    {
        setFlag(FLAGS6502::C, binary_sum > 0x00FF);
        a_ = gsl::narrow_cast<uint8_t>(binary_sum & 0x00FF);
    }

    setFlag(FLAGS6502::Z, a_ == 0x00);
    setFlag(FLAGS6502::N, (a_ & 0x80) != 0);
    return 1;
}

uint8_t CPU6502::SBC() noexcept
{
    fetch();
    const auto carry_in = getFlag(FLAGS6502::C);
    const uint16_t value = static_cast<uint16_t>(fetched_) ^ 0x00FF;
    const uint16_t binary_sum = static_cast<uint16_t>(a_) + value + static_cast<uint16_t>(carry_in);
    const auto binary_result = gsl::narrow_cast<uint8_t>(binary_sum & 0x00FF);

    setFlag(FLAGS6502::V, (binary_sum ^ static_cast<uint16_t>(a_)) & (binary_sum ^ value) & 0x0080);

    if(getFlag(FLAGS6502::D) == 1)
    {
        int16_t decimal_result = gsl::narrow_cast<int16_t>(a_) - gsl::narrow_cast<int16_t>(fetched_) - gsl::narrow_cast<int16_t>(1 - carry_in);
        if(((a_ & 0x0F) - (1 - carry_in)) < (fetched_ & 0x0F))
        {
            decimal_result -= 0x06;
        }
        if(decimal_result < 0)
        {
            decimal_result -= 0x60;
        }

        setFlag(FLAGS6502::C, binary_sum & 0xFF00);
        a_ = gsl::narrow_cast<uint8_t>(decimal_result & 0x00FF);
    }
    else
    {
        setFlag(FLAGS6502::C, binary_sum & 0xFF00);
        a_ = binary_result;
    }

    setFlag(FLAGS6502::Z, a_ == 0x00);
    setFlag(FLAGS6502::N, (a_ & 0x80) != 0);
    return 1;
}

uint8_t CPU6502::AND() noexcept
{
    fetch();
    a_ &= fetched_;
    setFlag(FLAGS6502::Z, a_ == 0);
    setFlag(FLAGS6502::N, a_ & 0x80);
    return 1;
}

uint8_t CPU6502::ORA() noexcept
{
    fetch();
    a_ |= fetched_;
    setFlag(FLAGS6502::Z, a_ == 0);
    setFlag(FLAGS6502::N, a_ & 0x80);
    return 1;
}

uint8_t CPU6502::EOR() noexcept
{
    fetch();
    a_ ^= fetched_;
    setFlag(FLAGS6502::Z, a_ == 0);
    setFlag(FLAGS6502::N, a_ & 0x80);
    return 1;
}

// opcode_ is uint8_t, so it always indexes the full lookup table size.
#pragma warning(push)
#pragma warning(disable:26482)
uint8_t CPU6502::ASL() noexcept
{
    fetch();
    const uint16_t t = static_cast<uint16_t>(fetched_) << 1;
    setFlag(FLAGS6502::C, (t & 0xFF00) > 0);
    setFlag(FLAGS6502::Z, (t & 0x00FF) == 0);
    setFlag(FLAGS6502::N, t & 0x80);
    if(lookupTable()[opcode_].addrmode == &CPU6502::IMP)
    {
        a_ = t & 0x00FF;
    }
    else
    {
        write(addr_abs_, t & 0x00FF);
    }
    return 0;
}

uint8_t CPU6502::LSR() noexcept
{
    fetch();
    setFlag(FLAGS6502::C, fetched_ & 0x0001);
    const uint8_t t = fetched_ >> 1;
    setFlag(FLAGS6502::Z, (t & 0x00FF) == 0);
    setFlag(FLAGS6502::N, t & 0x80);
    if(lookupTable()[opcode_].addrmode == &CPU6502::IMP)
    {
        a_ = t;
    }
    else
    {
        write(addr_abs_, t);
    }
    return 0;
}

uint8_t CPU6502::ROL() noexcept
{
    fetch();
    const uint16_t t = (static_cast<uint16_t>(fetched_) << 1) | getFlag(FLAGS6502::C);
    setFlag(FLAGS6502::C, t & 0xFF00);
    setFlag(FLAGS6502::Z, (t & 0x00FF) == 0);
    setFlag(FLAGS6502::N, t & 0x80);
    if(lookupTable()[opcode_].addrmode == &CPU6502::IMP)
    {
        a_ = t & 0x00FF;
    }
    else
    {
        write(addr_abs_, t & 0x00FF);
    }
    return 0;
}

uint8_t CPU6502::ROR() noexcept
{
    fetch();
    const uint16_t t = gsl::narrow_cast<uint16_t>((getFlag(FLAGS6502::C) << 7) | (fetched_ >> 1));
    setFlag(FLAGS6502::C, fetched_ & 0x01);
    setFlag(FLAGS6502::Z, (t & 0x00FF) == 0);
    setFlag(FLAGS6502::N, t & 0x80);
    if(lookupTable()[opcode_].addrmode == &CPU6502::IMP)
    {
        a_ = t & 0x00FF;
    }
    else
    {
        write(addr_abs_, t & 0x00FF);
    }
    return 0;
}
#pragma warning(pop)

uint8_t CPU6502::branch(bool condition) noexcept
{
    if(condition)
    {
        cycles_++;
        addr_abs_ = pc_ + addr_rel_;
        if((addr_abs_ & 0xFF00) != (pc_ & 0xFF00))
        {
            cycles_++;
        }
        pc_ = addr_abs_;
    }
    return 0;
}

uint8_t CPU6502::BCC() noexcept
{
    return branch(getFlag(FLAGS6502::C) == 0);
}

uint8_t CPU6502::BCS() noexcept
{
    return branch(getFlag(FLAGS6502::C) == 1);
}

uint8_t CPU6502::BEQ() noexcept
{
    return branch(getFlag(FLAGS6502::Z) == 1);
}

uint8_t CPU6502::BNE() noexcept
{
    return branch(getFlag(FLAGS6502::Z) == 0);
}

uint8_t CPU6502::BMI() noexcept
{
    return branch(getFlag(FLAGS6502::N) == 1);
}

uint8_t CPU6502::BPL() noexcept
{
    return branch(getFlag(FLAGS6502::N) == 0);
}

uint8_t CPU6502::BVC() noexcept
{
    return branch(getFlag(FLAGS6502::V) == 0);
}

uint8_t CPU6502::BVS() noexcept
{
    return branch(getFlag(FLAGS6502::V) == 1);
}

uint8_t CPU6502::BIT() noexcept
{
    fetch();
    const uint16_t t = a_ & fetched_;
    setFlag(FLAGS6502::Z, (t & 0x00FF) == 0);
    setFlag(FLAGS6502::N, fetched_ & (1 << 7));
    setFlag(FLAGS6502::V, fetched_ & (1 << 6));
    return 0;
}

uint8_t CPU6502::BRK() noexcept
{
    pc_++;
    setFlag(FLAGS6502::I, true);
    write(0x0100 + stkp_, (pc_ >> 8) & 0x00FF);
    stkp_--;
    write(0x0100 + stkp_, pc_ & 0x00FF);
    stkp_--;
    setFlag(FLAGS6502::B, true);
    write(0x0100 + stkp_, status_);
    stkp_--;
    setFlag(FLAGS6502::B, false);
    pc_ = static_cast<uint16_t>(read(0xFFFE)) | (static_cast<uint16_t>(read(0xFFFF)) << 8);
    return 0;
}

uint8_t CPU6502::CLC() noexcept
{
    setFlag(FLAGS6502::C, false);
    return 0;
}

uint8_t CPU6502::CLD() noexcept
{
    setFlag(FLAGS6502::D, false);
    return 0;
}

uint8_t CPU6502::CLI() noexcept
{
    setFlag(FLAGS6502::I, false);
    return 0;
}

uint8_t CPU6502::CLV() noexcept
{
    setFlag(FLAGS6502::V, false);
    return 0;
}

uint8_t CPU6502::SEC() noexcept
{
    setFlag(FLAGS6502::C, true);
    return 0;
}

uint8_t CPU6502::SED() noexcept
{
    setFlag(FLAGS6502::D, true);
    return 0;
}

uint8_t CPU6502::SEI() noexcept
{
    setFlag(FLAGS6502::I, true);
    return 0;
}

uint8_t CPU6502::CMP() noexcept
{
    fetch();
    const uint16_t t = static_cast<uint16_t>(a_) - static_cast<uint16_t>(fetched_);
    setFlag(FLAGS6502::C, a_ >= fetched_);
    setFlag(FLAGS6502::Z, (t & 0xFF) == 0);
    setFlag(FLAGS6502::N, t & 0x0080);
    return 1;
}

uint8_t CPU6502::CPX() noexcept
{
    fetch();
    const uint16_t t = static_cast<uint16_t>(x_) - static_cast<uint16_t>(fetched_);
    setFlag(FLAGS6502::C, x_ >= fetched_);
    setFlag(FLAGS6502::Z, (t & 0xFF) == 0);
    setFlag(FLAGS6502::N, t & 0x0080);
    return 0;
}

uint8_t CPU6502::CPY() noexcept
{
    fetch();
    const uint16_t t = static_cast<uint16_t>(y_) - static_cast<uint16_t>(fetched_);
    setFlag(FLAGS6502::C, y_ >= fetched_);
    setFlag(FLAGS6502::Z, (t & 0xFF) == 0);
    setFlag(FLAGS6502::N, t & 0x0080);
    return 0;
}

uint8_t CPU6502::DEC() noexcept
{
    fetch();
    const uint16_t t = static_cast<uint16_t>(fetched_) - 1;
    write(addr_abs_, t & 0xFF);
    setFlag(FLAGS6502::Z, (t & 0xFF) == 0);
    setFlag(FLAGS6502::N, t & 0x0080);
    return 0;
}

uint8_t CPU6502::DEX() noexcept
{
    x_--;
    setFlag(FLAGS6502::Z, x_ == 0);
    setFlag(FLAGS6502::N, x_ & 0x80);
    return 0;
}

uint8_t CPU6502::DEY() noexcept
{
    y_--;
    setFlag(FLAGS6502::Z, y_ == 0);
    setFlag(FLAGS6502::N, y_ & 0x80);
    return 0;
}

uint8_t CPU6502::INC() noexcept
{
    fetch();
    const uint16_t t = fetched_ + 1;
    write(addr_abs_, t & 0xFF);
    setFlag(FLAGS6502::Z, (t & 0xFF) == 0);
    setFlag(FLAGS6502::N, t & 0x0080);
    return 0;
}

uint8_t CPU6502::INX() noexcept
{
    x_++;
    setFlag(FLAGS6502::Z, x_ == 0);
    setFlag(FLAGS6502::N, x_ & 0x80);
    return 0;
}

uint8_t CPU6502::INY() noexcept
{
    y_++;
    setFlag(FLAGS6502::Z, y_ == 0);
    setFlag(FLAGS6502::N, y_ & 0x80);
    return 0;
}

uint8_t CPU6502::JMP() noexcept
{
    pc_ = addr_abs_;
    return 0;
}

uint8_t CPU6502::JSR() noexcept
{
    pc_--;
    write(0x0100 + stkp_, (pc_ >> 8) & 0xFF);
    stkp_--;
    write(0x0100 + stkp_, pc_ & 0xFF);
    stkp_--;
    pc_ = addr_abs_;
    return 0;
}

uint8_t CPU6502::LDA() noexcept
{
    fetch();
    a_ = fetched_;
    setFlag(FLAGS6502::Z, a_ == 0);
    setFlag(FLAGS6502::N, a_ & 0x80);
    return 1;
}

uint8_t CPU6502::LDX() noexcept
{
    fetch();
    x_ = fetched_;
    setFlag(FLAGS6502::Z, x_ == 0);
    setFlag(FLAGS6502::N, x_ & 0x80);
    return 1;
}

uint8_t CPU6502::LDY() noexcept
{
    fetch();
    y_ = fetched_;
    setFlag(FLAGS6502::Z, y_ == 0);
    setFlag(FLAGS6502::N, y_ & 0x80);
    return 1;
}

uint8_t CPU6502::NOP() noexcept
{
    return 0;
}

uint8_t CPU6502::PHA() noexcept
{
    write(0x0100 + stkp_, a_);
    stkp_--;
    return 0;
}

uint8_t CPU6502::PHP() noexcept
{
    write(0x0100 + stkp_, status_ | std::to_underlying(FLAGS6502::B) | std::to_underlying(FLAGS6502::U));
    setFlag(FLAGS6502::B, false);
    setFlag(FLAGS6502::U, false);
    stkp_--;
    return 0;
}

uint8_t CPU6502::PLA() noexcept
{
    stkp_++;
    a_ = read(0x0100 + stkp_);
    setFlag(FLAGS6502::Z, a_ == 0);
    setFlag(FLAGS6502::N, a_ & 0x80);
    return 0;
}

uint8_t CPU6502::PLP() noexcept
{
    stkp_++;
    status_ = read(0x0100 + stkp_);
    setFlag(FLAGS6502::U, true);
    return 0;
}

uint8_t CPU6502::RTI() noexcept
{
    stkp_++;
    status_ = read(0x0100 + stkp_);
    status_ &= ~std::to_underlying(FLAGS6502::B);
    status_ &= ~std::to_underlying(FLAGS6502::U);
    stkp_++;
    pc_ = static_cast<uint16_t>(read(0x0100 + stkp_));
    stkp_++;
    pc_ |= static_cast<uint16_t>(read(0x0100 + stkp_)) << 8;
    return 0;
}

uint8_t CPU6502::RTS() noexcept
{
    stkp_++;
    pc_ = static_cast<uint16_t>(read(0x0100 + stkp_));
    stkp_++;
    pc_ |= static_cast<uint16_t>(read(0x0100 + stkp_)) << 8;
    pc_++;
    return 0;
}

uint8_t CPU6502::STA() noexcept
{
    write(addr_abs_, a_);
    return 0;
}

uint8_t CPU6502::STX() noexcept
{
    write(addr_abs_, x_);
    return 0;
}

uint8_t CPU6502::STY() noexcept
{
    write(addr_abs_, y_);
    return 0;
}

uint8_t CPU6502::TAX() noexcept
{
    x_ = a_;
    setFlag(FLAGS6502::Z, x_ == 0);
    setFlag(FLAGS6502::N, x_ & 0x80);
    return 0;
}

uint8_t CPU6502::TAY() noexcept
{
    y_ = a_;
    setFlag(FLAGS6502::Z, y_ == 0);
    setFlag(FLAGS6502::N, y_ & 0x80);
    return 0;
}

uint8_t CPU6502::TSX() noexcept
{
    x_ = stkp_;
    setFlag(FLAGS6502::Z, x_ == 0);
    setFlag(FLAGS6502::N, x_ & 0x80);
    return 0;
}

uint8_t CPU6502::TXA() noexcept
{
    a_ = x_;
    setFlag(FLAGS6502::Z, a_ == 0);
    setFlag(FLAGS6502::N, a_ & 0x80);
    return 0;
}

uint8_t CPU6502::TXS() noexcept
{
    stkp_ = x_;
    return 0;
}

uint8_t CPU6502::TYA() noexcept
{
    a_ = y_;
    setFlag(FLAGS6502::Z, a_ == 0);
    setFlag(FLAGS6502::N, a_ & 0x80);
    return 0;
}

uint8_t CPU6502::XXX() noexcept
{
    return 0;
}
