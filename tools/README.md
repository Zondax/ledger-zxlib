# Tools

This directory contains development tools for ledger-zxlib.

## generate_bech32_tests.py

A comprehensive tool for generating and verifying bech32 test cases using the official reference implementation from sipa/bech32.

### Features

- **Reference Implementation**: Uses the official bech32 reference implementation for accurate test vectors
- **Multiple Encodings**: Supports both bech32 (BIP173) and bech32m (BIP350) encodings
- **Flexible Parameters**: Test different HRPs, padding modes, and witness versions
- **Verification Mode**: Verify individual test cases against the reference implementation
- **Test Generation**: Generate comprehensive C++ test cases for bech32.cpp

### Usage

#### Verify a specific test case

```bash
python3 generate_bech32_tests.py --verify <hex_data> <hrp> <pad> <encoding>
```

**Examples:**
```bash
# Verify deadbeef with bc HRP, padding enabled, bech32 encoding
python3 generate_bech32_tests.py --verify deadbeef bc 1 bech32

# Verify same data with bech32m encoding
python3 generate_bech32_tests.py --verify deadbeef bc 1 bech32m

# Verify with testnet HRP
python3 generate_bech32_tests.py --verify deadbeef tb 1 bech32

# Verify with padding disabled (pad=0)
python3 generate_bech32_tests.py --verify 0102030405 bc 0 bech32
```

#### Generate C++ test cases

```bash
python3 generate_bech32_tests.py > new_test_cases.cpp
```

This generates comprehensive C++ test cases that can be added to `tests/bech32.cpp`.

### Parameters

- **hex_data**: Hexadecimal input data (without 0x prefix)
- **hrp**: Human Readable Part (e.g., "bc", "tb", "test", "a")
- **pad**: Padding mode (1 for enabled, 0 for disabled)
- **encoding**: Encoding type ("bech32" or "bech32m")

### Online Verification

You can verify the generated test vectors using online tools:
- [bech32-buffer tool](https://slowli.github.io/bech32-buffer/) - Convert hex to bech32 addresses
- Set witver=0 for raw data encoding (equivalent to witver=None in this tool)

### Implementation Details

The tool implements the complete bech32/bech32m specification:

1. **Data Conversion**: Converts hex strings to byte arrays
2. **Bit Conversion**: Uses the standard 8-to-5 bit conversion with configurable padding
3. **Checksum**: Implements both bech32 (const=1) and bech32m (const=0x2bc830a3) checksums
4. **Encoding**: Uses the standard bech32 character set

### Integration with Tests

This tool was used to generate the verified test cases in `tests/bech32.cpp`:
- `BECH32.verified_hex_test_vectors`
- `BECH32M.verified_hex_test_vectors` 
- `BECH32.pad_zero_test_vectors`
- `BECH32.additional_comprehensive_cases`

All expected values in these tests can be independently verified using this tool.