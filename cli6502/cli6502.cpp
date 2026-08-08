#include <cstdint>
#include <cstdio>
#include <print>
#include <string>
#include <fstream>
#include <algorithm>
#include <string_view>
#include <vector>

#include <gsl/span>
#include <gsl/narrow>
#include <gsl/util>

#include <bus.h>
#include <cpu6502.h>
#include <disassembler.h>

static void print_usage()
{
    std::println("6502 CLI Emulator");
    std::println("Usage:");
    std::println("\t6502cli disasm <file.bin> <start_hex> [stop_hex]   Disassemble binary");
    std::println("\t6502cli run    <file.bin>                          Run binary from reset vector");
    std::println("\t6502cli dump   <file.bin>                          Hex dump first 256 bytes");
}

// idx can't exceed buffer's size
#pragma warning(push)
#pragma warning(disable:26482 26446)
static bool load_file(Bus& bus, std::string_view path, uint16_t load_addr = 0x8000)
{
    // The CLI treats the input file as a raw ROM image loaded contiguously into memory.
    std::ifstream f(path.data(), std::ios::binary);
    if(!f)
    {
        std::println(stderr, "Error: cannot open '{}'", path);
        return false;
    }
    f.seekg(0, std::ios::end);
    const auto file_end = f.tellg();
    if(file_end == std::streampos(-1))
    {
        std::println(stderr, "Error: cannot determine size of '{}'", path);
        return false;
    }

    const auto file_size = static_cast<std::streamsize>(file_end);
    const auto max_bytes = static_cast<std::streamsize>(Bus::RAM_SIZE - load_addr);
    const auto bytes_to_read = (file_size < max_bytes) ? file_size : max_bytes;
    std::vector<char> buffer(gsl::narrow_cast<size_t>(bytes_to_read));

    f.seekg(0, std::ios::beg);
    if(!buffer.empty())
    {
        f.read(buffer.data(), bytes_to_read);
        const auto bytes_read = f.gcount();
        auto addr = load_addr;
        for(std::streamsize idx = 0; idx < bytes_read; idx++)
        {
            bus.write(addr++, gsl::narrow_cast<uint8_t>(buffer[idx]));
        }
    }

    return true;
}
#pragma warning(pop)

static int disasm(const uint16_t start, const uint16_t stop, Bus& bus, std::string_view file)
{
    if(!load_file(bus, file, start))
    {
        return 1;
    }
    auto lines = Disassembler::disassemble(bus, start, stop);
    for(const auto& [addr, text] : lines)
    {
        std::println("{}", text);
    }
    return 0;
}

static int run(Bus& bus, std::string_view file, CPU6502& cpu)
{
    if(!load_file(bus, file))
    {
        return 1;
    }
    // Set reset vector to 0x8000
    bus.write(0xFFFC, 0x00);
    bus.write(0xFFFD, 0x80);

    cpu.reset();
    // Drain reset cycles
    while(!cpu.complete())
    {
        cpu.clock();
    }

    // Run up to 10000 instructions so the CLI terminates on finite test programs.
    for(int i = 0; i < 10000; i++)
    {
        do
        {
            cpu.clock();
        }
        while(!cpu.complete());
    }
    std::println("Done. A={:02X} X={:02X} Y={:02X} PC={:04X} SP={:02X} SR={:02X}", cpu.a_, cpu.x_, cpu.y_, cpu.pc_, cpu.stkp_, cpu.status_);
    return 0;
}

static int dump(Bus& bus, std::string_view file)
{
    if(!load_file(bus, file))
    {
        return 1;
    }
    for(int row = 0; row < 16; row++)
    {
        std::print("${:04X}: ", row * 16);
        for(int col = 0; col < 16; col++)
        {
            std::print("{:02X} ", bus.read(gsl::narrow<uint16_t>(row * 16 + col)));
        }
        std::println();
    }
    return 0;
}

int main(int argc, char* argv[])
{
    const gsl::span<char*> spanArg(argv, argc);

    if(spanArg.size() < 3)
    {
        print_usage();
        return 0;
    }

    std::string cmd = gsl::at(spanArg, 1);
    std::string file = gsl::at(spanArg, 2);

    auto bus = Bus();
    CPU6502 cpu;
    cpu.connectBus(&bus);

    int retVal = 0;
    if(cmd == "disasm")
    {
        const uint16_t start = (spanArg.size() >= 4) ? gsl::narrow<uint16_t>(std::stoul(gsl::at(spanArg, 3), nullptr, 16)) : 0x8000;
        const uint16_t stop = (spanArg.size() >= 5) ? gsl::narrow<uint16_t>(std::stoul(gsl::at(spanArg, 4), nullptr, 16)) : gsl::narrow<uint16_t>(start + 0x00FF);
        retVal = disasm(start, stop, bus, file);
    }
    else if(cmd == "run")
    {
        retVal = run(bus, file, cpu);
    }
    else if(cmd == "dump")
    {
        retVal = dump(bus, file);
    }
    else
    {
        print_usage();
    }
    return retVal;
}
