# Makefile for Python formatting in fuzzing infrastructure
# This file provides standardized Python code formatting rules for Zondax Ledger applications

# Configuration
BLACK_LINE_LENGTH := 120
# Only search in fuzzing-related directories (relative to this Makefile's location)
FUZZING_DIR := .
PROJECT_FUZZ_DIR := $(shell cd ../../../ && pwd)/fuzz

# Validate that the project fuzz directory exists
validate_fuzz_dir:
	@if [ ! -d "$(PROJECT_FUZZ_DIR)" ]; then \
		echo "⚠️  WARNING: Project fuzz directory not found: $(PROJECT_FUZZ_DIR)"; \
		echo "   Only formatting files in the common fuzzing library directory."; \
		echo "   To format project-specific files, please create the 'fuzz' directory."; \
	fi

# Find all Python files in fuzzing directories only  
PYTHON_FILES := $(shell find $(FUZZING_DIR) -name "*.py" -type f -not -path "*/.*" 2>/dev/null) \
                $(shell if [ -d "$(PROJECT_FUZZ_DIR)" ]; then find $(PROJECT_FUZZ_DIR) -name "*.py" -type f -not -path "*/.*" 2>/dev/null; fi)

# Default target
.PHONY: help
help:
	@echo "Python Formatting Rules for Zondax Ledger Fuzzing"
	@echo ""
	@echo "Available targets:"
	@echo "  format     Format Python files in fuzzing directories with Black"
	@echo "  check      Check if files need formatting (no changes)"
	@echo "  install    Install Black formatter"
	@echo "  clean      Remove __pycache__ directories from fuzzing dirs"
	@echo "  list       List Python files in fuzzing directories"
	@echo ""
	@echo "Configuration:"
	@echo "  Line length: $(BLACK_LINE_LENGTH)"
	@echo "  Fuzzing directories: $(FUZZING_DIR) $(PROJECT_FUZZ_DIR)"

# Install Black if not present
.PHONY: install
install:
	@echo "🔧 Installing Black Python formatter..."
	@pip3 install black || { echo "❌ Failed to install Black"; exit 1; }
	@echo "✅ Black installed successfully"

# Format all Python files (checks first, then formats only if needed)
.PHONY: format
format: install validate_fuzz_dir
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
check: install validate_fuzz_dir
	@echo "🔍 Checking Python file formatting..."
	@if [ -z "$(PYTHON_FILES)" ]; then \
		echo "⚠️  No Python files found"; \
	else \
		echo "📁 Checking $(shell echo $(PYTHON_FILES) | wc -w) Python files:"; \
		for file in $(PYTHON_FILES); do \
			echo "   • $$(basename $$file)"; \
		done; \
		echo ""; \
		black --check --line-length $(BLACK_LINE_LENGTH) $(PYTHON_FILES) && \
		echo "✅ All Python files are properly formatted" || \
		{ echo "❌ Some files need formatting - run 'make format'"; exit 1; }; \
	fi

# List all Python files that would be formatted
.PHONY: list
list: validate_fuzz_dir
	@echo "Python files in fuzzing directories:"
	@if [ -z "$(PYTHON_FILES)" ]; then \
		echo "  No Python files found in: $(FUZZING_DIR) $(PROJECT_FUZZ_DIR)"; \
	else \
		for file in $(PYTHON_FILES); do \
			echo "  $$file"; \
		done; \
	fi

# Clean Python cache files
.PHONY: clean
clean:
	@echo "🧹 Cleaning Python cache files..."
	@find $(PYTHON_DIRS) -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@find $(PYTHON_DIRS) -name "*.pyc" -delete 2>/dev/null || true
	@find $(PYTHON_DIRS) -name "*.pyo" -delete 2>/dev/null || true
	@echo "✅ Python cache files cleaned"

# Verify Black installation
.PHONY: version
version:
	@echo "🔧 Black version information:"
	@black --version 2>/dev/null || echo "❌ Black not installed - run 'make install'"