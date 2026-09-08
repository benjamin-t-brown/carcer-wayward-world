# Convenience commands for humans. CMake remains the authoritative build graph.

ROOT := $(abspath $(CURDIR)/..)
PRESET ?= dev-debug
JS_PRESET ?= emscripten-release
BUILD_DIR := $(ROOT)/build/cmake/$(PRESET)

ifeq ($(OS),Windows_NT)
CARCER_EXECUTABLE := $(BUILD_DIR)/CARCER.exe
else
CARCER_EXECUTABLE := $(BUILD_DIR)/CARCER
endif

.DEFAULT_GOAL := build
.PHONY: build configure run js test ui setup clean help

build: configure
	@cd "$(ROOT)" && cmake --build --preset "$(PRESET)" --target CARCER

configure:
	@if [ ! -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		cd "$(ROOT)" && cmake --preset "$(PRESET)"; \
	fi

run: build
	@cd "$(ROOT)/src" && "$(CARCER_EXECUTABLE)"

js:
	@if [ -z "$$EMSDK" ]; then \
		echo "error: activate the Emscripten SDK so EMSDK is set" >&2; \
		exit 1; \
	fi
	@cd "$(ROOT)" && CARCER_CMAKE_PRESET="$(JS_PRESET)" ./scripts/ci-build.sh

test: configure
	@cd "$(ROOT)" && cmake --build --preset "$(PRESET)" --target carcer_non_ui_tests
	@cd "$(ROOT)" && ctest --preset "$(PRESET)" --output-on-failure

ui: configure
	@cd "$(ROOT)" && cmake --build --preset "$(PRESET)" --target carcer_ui_tests

setup:
	@cd "$(ROOT)" && ./scripts/setup-dev.sh

clean: configure
	@cd "$(ROOT)" && cmake --build --preset "$(PRESET)" --target clean

help:
	@echo "make          Build the native game for the current platform"
	@echo "make run      Build and run the native game"
	@echo "make test     Build and run all non-interactive unit tests"
	@echo "make ui       Compile all interactive UI test programs"
	@echo "make js       Build the Emscripten release into web/"
	@echo "make clean    Clean outputs known to the current CMake graph"
