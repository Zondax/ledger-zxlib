# Makefile for ledger-zxlib fuzzing
# This file contains both fuzzing targets and Python formatting rules

# === FUZZING CONFIGURATION ===
FUZZ_TIME ?= 20
FUZZ_JOBS ?= 4
FUZZ_BUILD_DIR = fuzz_local/build
FUZZ_DIR = fuzz_local

# === PYTHON FORMATTING CONFIGURATION ===
BLACK_LINE_LENGTH := 120
FUZZING_DIR := .

# Default target
.PHONY: help
help:
	@echo "ledger-zxlib Fuzzing & Development Tools"
	@echo ""
	@echo "=== FUZZING TARGETS ==="
	@echo "  build_fuzz    Build fuzzer targets"
	@echo "  fuzz          Run all fuzzers (default: 10 minutes)"
	@echo "  fuzz_crash    Analyze any crashes found"
	@echo "  fuzz_clean    Clean fuzzing artifacts"
	@echo ""
	@echo "=== PYTHON FORMATTING ==="
	@echo "  format        Format Python files with Black"
	@echo "  check         Check if files need formatting"
	@echo "  format_clean  Remove Python cache files"
	@echo ""
	@echo "Environment variables:"
	@echo "  FUZZ_TIME     Fuzzing duration in seconds (default: 600)"
	@echo "  FUZZ_JOBS     Number of parallel jobs (default: 4)"
	@echo ""
	@echo "Examples:"
	@echo "  make fuzz FUZZ_TIME=60    # Run for 1 minute"
	@echo "  make fuzz FUZZ_JOBS=2     # Use 2 parallel jobs"

# === FUZZING TARGETS ===

# Check if clang is available
.PHONY: check_clang
check_clang:
	@which clang >/dev/null 2>&1 || { \
		echo "❌ Error: clang not found. Please install clang for fuzzing."; \
		echo "On macOS: brew install llvm"; \
		echo "On Ubuntu: apt install clang"; \
		exit 1; \
	}
	@echo "✅ Clang found: $$(clang --version | head -n1)"

# Build fuzzer targets
.PHONY: build_fuzz
build_fuzz: check_clang
	@echo "🔨 Building fuzzer targets..."
	@mkdir -p $(FUZZ_BUILD_DIR)
	@cd $(FUZZ_BUILD_DIR) && \
		cmake -DCMAKE_C_COMPILER=clang \
			  -DCMAKE_CXX_COMPILER=clang++ \
			  -DENABLE_FUZZING=ON \
			  -DENABLE_SANITIZERS=ON \
			  -DCMAKE_BUILD_TYPE=Debug \
			  .. && \
		make -j$(FUZZ_JOBS)
	@echo "✅ Fuzzer build completed"

# Run fuzzers
.PHONY: fuzz
fuzz: build_fuzz
	@echo "🚀 Starting fuzzing session..."
	@echo "   Duration: $(FUZZ_TIME) seconds"
	@echo "   Jobs: $(FUZZ_JOBS)"
	@python3 $(FUZZ_DIR)/run_local_fuzz.py \
		--max-seconds $(FUZZ_TIME) \
		--jobs $(FUZZ_JOBS)
	@echo "🏁 Fuzzing session completed"

# Analyze crashes
.PHONY: fuzz_crash
fuzz_crash:
	@echo "🔍 Analyzing fuzzing crashes..."
	@python3 $(FUZZ_DIR)/analyze_local_crashes.py

# Clean fuzzing artifacts
.PHONY: fuzz_clean
fuzz_clean:
	@echo "🧹 Cleaning fuzzing artifacts..."
	@rm -rf $(FUZZ_BUILD_DIR)
	@rm -rf $(FUZZ_DIR)/corpora
	@rm -rf $(FUZZ_DIR)/logs
	@rm -rf $(FUZZ_DIR)/coverage
	@echo "✅ Fuzzing artifacts cleaned"

# === PYTHON FORMATTING TARGETS ===

# Find all Python files in fuzzing directories
PYTHON_FILES := $(shell find $(FUZZING_DIR) -name "*.py" -type f -not -path "*/.*" 2>/dev/null)

# Install Black if not present
.PHONY: format_install
format_install:
	@echo "🔧 Installing Black Python formatter..."
	@pip3 install black || { echo "❌ Failed to install Black"; exit 1; }
	@echo "✅ Black installed successfully"

# Format all Python files
.PHONY: format
format: format_install
	@echo "🎨 Formatting Python files with Black..."
	@if $(MAKE) check; then \
		echo "✅ All Python files are already properly formatted - no changes needed"; \
	else \
		echo "⚙️  Files need formatting - applying changes..."; \
		if [ -z "$(PYTHON_FILES)" ]; then \
			echo "⚠️  No Python files found"; \
		else \
			black --line-length $(BLACK_LINE_LENGTH) $(PYTHON_FILES) && \
			echo "✅ All Python files formatted successfully" || \
			{ echo "❌ Formatting failed"; exit 1; }; \
		fi; \
	fi

# Check formatting without making changes
.PHONY: check
check: format_install
	@echo "🔍 Checking Python file formatting..."
	@if [ -z "$(PYTHON_FILES)" ]; then \
		echo "⚠️  No Python files found"; \
	else \
		echo "📁 Checking $(shell echo $(PYTHON_FILES) | wc -w) Python files"; \
		black --check --line-length $(BLACK_LINE_LENGTH) $(PYTHON_FILES) && \
		echo "✅ All Python files are properly formatted" || \
		{ echo "❌ Some files need formatting - run 'make format'"; exit 1; }; \
	fi

# Clean Python cache files
.PHONY: format_clean
format_clean:
	@echo "🧹 Cleaning Python cache files..."
	@find $(FUZZING_DIR) -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@find $(FUZZING_DIR) -name "*.pyc" -delete 2>/dev/null || true
	@find $(FUZZING_DIR) -name "*.pyo" -delete 2>/dev/null || true
	@echo "✅ Python cache files cleaned"

# List Python files
.PHONY: format_list
format_list:
	@echo "Python files in fuzzing directories:"
	@if [ -z "$(PYTHON_FILES)" ]; then \
		echo "  No Python files found in: $(FUZZING_DIR)"; \
	else \
		for file in $(PYTHON_FILES); do \
			echo "  $$file"; \
		done; \
	fi