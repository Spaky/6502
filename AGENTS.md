# AGENTS.md

## Repository overview
- Project: NMOS 6502 emulator in C++23.
- Solution file: `6502.slnx`.
- Main components:
  - `cpu6502/`: emulator core (`Bus`, `CPU6502`, instruction table, disassembler).
  - `cli6502/`: command-line frontend (`6502cli`).
  - `test_cpu/`: executable unit/smoke tests.

## Build and test
- Preferred build target: x64 Debug in Visual Studio.
- CLI build example:
  - `msbuild /m /p:Configuration=Debug /p:Platform=x64 6502.slnx`
- Test command (from README):
  - `.\x64\Debug\test_cpu.exe`

## Agent change guidelines
- Keep changes focused and minimal to the requested task.
- Preserve documented CPU behavior:
  - NMOS 6502 instruction semantics.
  - Decimal-mode `ADC`/`SBC`.
  - `JMP ($xxFF)` page-wrap quirk.
  - Branch cycle handling.
- Avoid broad refactors unless explicitly requested.
- When updating CPU behavior, add or update tests in `test_cpu/test_cpu.cpp`.

## Code style cues
- Follow existing C++ style in the repo:
  - Opening braces on a new line.
  - Prefer fixed-width integer types (`uint8_t`, `uint16_t`).
  - Use `std::print`/`std::println` formatting style already present.
