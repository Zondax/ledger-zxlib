# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ledger-zxlib is a common C/C++ library for building Ledger hardware wallet applications. It provides core utilities, UI components for different Ledger devices, EVM support, and cryptographic operations.

## Architecture

### Directory Structure
- `/include` - Public headers for library functions
  - Encoding: base58.h, base64.h, bech32.h, hexutils.h
  - Crypto: sigutils.h, segwit_addr.h
  - Display: zxformat.h, view_templates.h
  - Core: zxerror.h, zxcanary.h, zxmacros.h
- `/src` - Implementation files
- `/app` - Ledger app-specific code
  - `/common` - Common app functionality
  - `/ui` - Device-specific UI (view_nano.c, view_nbgl.c)
- `/evm` - Ethereum support (RLP, uint256, transaction parsing)
- `/fuzzing` - Security testing infrastructure
- `/tests` - C++ unit tests

### Key Components

1. **Multi-Device Support**: The library abstracts UI differences between Nano S/X/S Plus (BAGL) and Stax/Flex (NBGL) devices.

2. **EVM Module**: Provides Ethereum transaction parsing, RLP encoding/decoding, uint256 arithmetic, and EIP-191/ERC-20 support.

3. **Encoding Utilities**: Implements base58, base64, bech32, and hex encoding/decoding with Ledger-specific optimizations.

4. **Error Handling**: Uses zxerror.h error codes and zxcanary.h for stack protection.

## Development Patterns

### Adding New Features
1. Add public interface in `/include`
2. Implement in `/src`
3. Add unit tests in `/tests`
4. Run `make cpp_test` to verify
5. Format with `make format`

### Device-Specific Code
- Use `#ifdef TARGET_NANOS` etc. for device-specific code
- UI code is split between BAGL (Nano devices) and NBGL (Stax/Flex)
- Common logic goes in `/app/common`

### Testing Approach
- Unit tests use Google Test framework
- Fuzzing uses LibFuzzer with Python orchestration
- Integration tests use Zemu (Ledger emulator)

### Version Management
- Version defined in `include/zxversion.h`
- Use `scripts/get_version.py` to check version
- CI auto-tags releases on main branch