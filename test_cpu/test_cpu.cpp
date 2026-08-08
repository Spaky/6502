#include <print>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <memory>

#include <bus.h>
#include <cpu6502.h>
#include <disassembler.h>

// ---- helpers ----
static void assert_eq(uint8_t actual, uint8_t expected, const char* test)
{
    if(actual != expected)
    {
        std::println("[FAIL] {} - expected 0x{:02X}, got 0x{:02X}", test, expected, actual);
        std::exit(1);
    }
}

static void assert_eq(uint16_t actual, uint16_t expected, const char* test)
{
    if(actual != expected)
    {
        std::println("[FAIL] {} - expected 0x{:04X}, got 0x{:04X}", test, expected, actual);
        std::exit(1);
    }
}

// ---- setup helper ----
static void reset_cpu(Bus& bus, CPU6502& cpu) noexcept
{
    bus.write(0xFFFC, 0x00);
    bus.write(0xFFFD, 0x80);
    cpu.reset();
    for(int i = 0; i < 8; i++)
    {
        cpu.clock();
    }
}

static Bus bus;
static CPU6502 cpu;

// ---- Addressing mode tests ----
static void test_imm()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x42);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x42u, "IMM: LDA #$42");
    std::println("[PASS] test_imm");
}

static void test_zp0()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x0050, 0x55);
    bus.write(0x8000, 0xA5);
    bus.write(0x8001, 0x50);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x55u, "ZP0: LDA $50");
    std::println("[PASS] test_zp0");
}

static void test_zpx()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x0055, 0x77);
    bus.write(0x8000, 0xB5);
    bus.write(0x8001, 0x50);
    reset_cpu(bus, cpu);
    cpu.x_ = 0x05;
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x77u, "ZPX: LDA $50,X");
    std::println("[PASS] test_zpx");
}

static void test_abs()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x3000, 0xAB);
    bus.write(0x8000, 0xAD);
    bus.write(0x8001, 0x00);
    bus.write(0x8002, 0x30);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0xABu, "ABS: LDA $3000");
    std::println("[PASS] test_abs");
}

// ---- Arithmetic tests ----
static void test_adc_no_carry()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x50);
    bus.write(0x8002, 0x69);
    bus.write(0x8003, 0x30);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x80u, "ADC: 0x50+0x30=0x80");
    assert_eq(cpu.getFlag(FLAGS6502::N), 1u, "ADC: N flag");
    std::println("[PASS] test_adc_no_carry");
}

static void test_adc_carry()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x80);
    bus.write(0x8002, 0x69);
    bus.write(0x8003, 0x80);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x00u, "ADC: 0x80+0x80=0");
    assert_eq(cpu.getFlag(FLAGS6502::C), 1u, "ADC: C flag");
    assert_eq(cpu.getFlag(FLAGS6502::Z), 1u, "ADC: Z flag");
    assert_eq(cpu.getFlag(FLAGS6502::V), 1u, "ADC: V flag");
    std::println("[PASS] test_adc_carry");
}

static void test_sbc()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x50);
    bus.write(0x8002, 0xE9);
    bus.write(0x8003, 0x30);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x1Fu, "SBC: 0x50-0x30=0x1F");
    std::println("[PASS] test_sbc");
}

static void test_adc_decimal()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xF8); // SED
    bus.write(0x8001, 0xA9); bus.write(0x8002, 0x45); // LDA #$45
    bus.write(0x8003, 0x18); // CLC
    bus.write(0x8004, 0x69); bus.write(0x8005, 0x55); // ADC #$55 -> 0x00, C=1 in BCD
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x00u, "ADC decimal result");
    assert_eq(cpu.getFlag(FLAGS6502::C), 1u, "ADC decimal carry");
    assert_eq(cpu.getFlag(FLAGS6502::Z), 1u, "ADC decimal zero");
    std::println("[PASS] test_adc_decimal");
}

static void test_sbc_decimal()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xF8); // SED
    bus.write(0x8001, 0xA9); bus.write(0x8002, 0x50); // LDA #$50
    bus.write(0x8003, 0x38); // SEC (no borrow)
    bus.write(0x8004, 0xE9); bus.write(0x8005, 0x01); // SBC #$01 -> 0x49 in BCD
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x49u, "SBC decimal result");
    assert_eq(cpu.getFlag(FLAGS6502::C), 1u, "SBC decimal carry");
    std::println("[PASS] test_sbc_decimal");
}

// ---- Bitwise tests ----
static void test_and()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0xF0);
    bus.write(0x8002, 0x29);
    bus.write(0x8003, 0x0F);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x00u, "AND");
    assert_eq(cpu.getFlag(FLAGS6502::Z), 1u, "AND: Z");
    std::println("[PASS] test_and");
}

static void test_ora()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0xF0);
    bus.write(0x8002, 0x09);
    bus.write(0x8003, 0x0F);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0xFFu, "ORA");
    std::println("[PASS] test_ora");
}

// ---- Shift tests ----
static void test_asl()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x40);
    bus.write(0x8002, 0x0A);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x80u, "ASL");
    std::println("[PASS] test_asl");
}

static void test_lsr()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x81);
    bus.write(0x8002, 0x4A);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x40u, "LSR");
    assert_eq(cpu.getFlag(FLAGS6502::C), 1u, "LSR: C");
    std::println("[PASS] test_lsr");
}

// ---- Compare tests ----
static void test_cmp_eq()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x42);
    bus.write(0x8002, 0xC9);
    bus.write(0x8003, 0x42);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.getFlag(FLAGS6502::Z), 1u, "CMP eq: Z");
    assert_eq(cpu.getFlag(FLAGS6502::C), 1u, "CMP eq: C");
    std::println("[PASS] test_cmp_eq");
}

static void test_cmp_lt()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x30);
    bus.write(0x8002, 0xC9);
    bus.write(0x8003, 0x50);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.getFlag(FLAGS6502::C), 0u, "CMP lt: C clear");
    std::println("[PASS] test_cmp_lt");
}

// ---- Branch tests ----
static void test_beq_taken()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x00); // LDA #0 -> Z=1
    bus.write(0x8002, 0xF0);
    bus.write(0x8003, 0x0A); // BEQ +10 -> 0x800E
    bus.write(0x800E, 0xA9);
    bus.write(0x800F, 0x42); // LDA #$42
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x42u, "BEQ taken");
    std::println("[PASS] test_beq_taken");
}

static void test_bne_not_taken()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x00);
    bus.write(0x8002, 0xD0);
    bus.write(0x8003, 0x0A); // BNE not taken (Z=1)
    bus.write(0x8004, 0xA9);
    bus.write(0x8005, 0x55);
    reset_cpu(bus, cpu);
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x55u, "BNE not taken");
    std::println("[PASS] test_bne_not_taken");
}

// ---- Register transfer tests ----
static void test_tax()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xAA);
    reset_cpu(bus, cpu);
    cpu.a_ = 0x42;
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.x_, 0x42u, "TAX");
    std::println("[PASS] test_tax");
}

static void test_tya()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0x98);
    reset_cpu(bus, cpu);
    cpu.y_ = 0x99;
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.a_, 0x99u, "TYA");
    std::println("[PASS] test_tya");
}

// ---- Inc/Dec tests ----
static void test_inx()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xE8);
    reset_cpu(bus, cpu);
    cpu.x_ = 0x42;
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.x_, 0x43u, "INX");
    std::println("[PASS] test_inx");
}

static void test_dey()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0x88);
    reset_cpu(bus, cpu);
    cpu.y_ = 0x42;
    do
    {
        cpu.clock();
    }
    while(!cpu.complete());
    assert_eq(cpu.y_, 0x41u, "DEY");
    std::println("[PASS] test_dey");
}

// ---- Disassembler test ----
static void test_disassembler()
{
    bus.clear();
    cpu.connectBus(&bus);
    bus.write(0x8000, 0xA9);
    bus.write(0x8001, 0x42); // LDA #$42
    bus.write(0x8002, 0x69);
    bus.write(0x8003, 0x10); // ADC #$10
    auto r = Disassembler::disassemble(bus, 0x8000, 0x8003);
    assert(r.count(0x8000) > 0);
    assert(r.count(0x8002) > 0);
    std::println("[PASS] test_disassembler ({} lines)", r.size());
}

int main()
{
    std::println("=== 6502 CPU Unit Tests ===\n");

    std::println("-- Addressing --");
    test_imm(); test_zp0(); test_zpx(); test_abs();

    std::println("\n-- Arithmetic --");
    test_adc_no_carry(); test_adc_carry(); test_sbc(); test_adc_decimal(); test_sbc_decimal();

    std::println("\n-- Bitwise --");
    test_and(); test_ora();

    std::println("\n-- Shifts --");
    test_asl(); test_lsr();

    std::println("\n-- Compare --");
    test_cmp_eq(); test_cmp_lt();

    std::println("\n-- Branches --");
    test_beq_taken(); test_bne_not_taken();

    std::println("\n-- Registers --");
    test_tax(); test_tya();

    std::println("\n-- Inc/Dec --");
    test_inx(); test_dey();

    std::println("\n-- Disassembler --");
    test_disassembler();

    std::println("\n=== ALL TESTS PASSED ===");

    return 0;
}
