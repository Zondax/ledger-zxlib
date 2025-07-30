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
    ]
