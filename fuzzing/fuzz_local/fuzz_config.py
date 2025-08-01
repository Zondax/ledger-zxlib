#!/usr/bin/env python3
"""
Fuzzer configuration for ledger-zxlib project
"""

# Maximum number of parallel fuzzer jobs
FUZZER_JOBS = 4


class FuzzConfig:
    """Configuration for a single fuzzer target"""

    def __init__(self, name: str, max_len: int = 17000):
        self.name = name
        self.max_len = max_len


def get_fuzzer_configs():
    """Return list of fuzzer configurations for this project"""
    return [
        FuzzConfig("bech32", max_len=256),  # bech32 fuzzer with appropriate max length
        FuzzConfig("base58", max_len=512),  # base58 fuzzer for encoding/decoding
        FuzzConfig("base64", max_len=1024),  # base64 fuzzer for encoding
        FuzzConfig("hexutils", max_len=512),  # hexutils fuzzer for hex string parsing
        FuzzConfig("segwit_addr", max_len=256),  # segwit address fuzzer
    ]
