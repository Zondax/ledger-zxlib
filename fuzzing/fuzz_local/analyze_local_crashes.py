#!/usr/bin/env python3
"""
Local crash analyzer for ledger-zxlib project.
This script analyzes crashes with correct path configuration.
"""

import os
import sys
import subprocess


def main():
    # Get paths - now we're in fuzzing/fuzz_local
    script_dir = os.path.dirname(os.path.abspath(__file__))
    fuzzing_dir = os.path.dirname(script_dir)  # fuzzing directory
    project_root = os.path.dirname(fuzzing_dir)  # project root

    # Change to fuzz directory for execution
    os.chdir(script_dir)

    # Run the common crash analyzer
    cmd = [
        sys.executable,
        os.path.join(fuzzing_dir, "analyze_crashes.py"),
        "--project-root",
        project_root,
        "--build-dir",
        os.path.join(script_dir, "build"),
        "--fuzz-dir",
        script_dir,
        "--logs-dir",
        os.path.join(script_dir, "logs"),
    ]

    print(f"🔍 Analyzing ledger-zxlib crashes...")
    print(f"Working directory: {script_dir}")
    print(f"Project root: {project_root}")

    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())
