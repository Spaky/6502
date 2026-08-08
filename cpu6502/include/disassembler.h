#pragma once
#include <cstdint>
#include <string>
#include <map>
#ifdef CPU6502_EXPORTS
#define DISASSEMBLER_API __declspec(dllexport)
#else
#define DISASSEMBLER_API __declspec(dllimport)
#endif

class Bus;

class DISASSEMBLER_API Disassembler
{
public:
    /// Return a map of address -> disassembled string for the given range.
    static std::map<uint16_t, std::string> disassemble(const Bus& bus, uint16_t start, uint16_t stop);
};
