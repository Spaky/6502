# 6502 Emulator

An NMOS 6502 emulator written in C++23.

## Project layout

- `cpu6502` DLL: CPU core, bus, instruction table, and opcode disassembler
- `6502cli` EXE: small command-line front end
- `test_cpu` EXE: unit tests and disassembler smoke tests

## Build

Open `6502.slnx` in Visual Studio and build the x64 configuration.

The solution contains:

- `cpu6502` DLL
- `6502cli` EXE
- `test_cpu` EXE

## Run tests

```bash
.\x64\Debug\test_cpu.exe
```

## CLI usage

```bash
6502cli disasm <file.bin> <start_hex> [stop_hex]
6502cli run <file.bin>
6502cli dump <file.bin>
```

### Commands

- `disasm`: loads the file at the requested start address and prints the disassembly for the requested range
- `run`: loads a binary at `0x8000`, sets the reset vector to `0x8000`, and runs up to 10,000 instructions
- `dump`: prints a 16x16 hex dump of the first 256 bytes loaded at `0x8000`

## CPU behavior

- NMOS 6502 instruction set
- decimal-mode ADC/SBC support
- NMOS indirect `JMP ($xxFF)` wraparound behavior
- branch cycle handling
- public CPU registers for debugger/test access
