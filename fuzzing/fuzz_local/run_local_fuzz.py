#!/usr/bin/env python3
"""
Local fuzzing runner for ledger-zxlib project.
This script runs fuzzing with correct path configuration.
"""

import os
import sys
import subprocess
import argparse


def main():
    parser = argparse.ArgumentParser(description="Run fuzzers for ledger-zxlib")
    parser.add_argument("--max-seconds", type=int, default=600, help="Maximum seconds per fuzzer run")
    parser.add_argument("--jobs", type=int, default=4, help="Number of parallel jobs")

    args = parser.parse_args()

    # Get paths - now we're in fuzzing/fuzz_local
    script_dir = os.path.dirname(os.path.abspath(__file__))
    fuzzing_dir = os.path.dirname(script_dir)  # fuzzing directory

    # Set environment to use correct working directory for logs
    env = os.environ.copy()

    # Change to fuzz directory for execution
    os.chdir(script_dir)

    # Run the common fuzzing runner with corrected arguments
    cmd = [
        sys.executable,
        os.path.join(fuzzing_dir, "run_fuzzers.py"),
        "--fuzz-dir",
        script_dir,
        "--max-seconds",
        str(args.max_seconds),
        "--jobs",
        str(args.jobs),
    ]

    print(f"🚀 Starting ledger-zxlib fuzzing...")
    print(f"Working directory: {script_dir}")
    print(f"Duration: {args.max_seconds}s, Jobs: {args.jobs}")

    return subprocess.call(cmd, env=env)


if __name__ == "__main__":
    sys.exit(main())
