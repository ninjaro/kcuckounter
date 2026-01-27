SHELL := /usr/bin/env bash
.SHELLFLAGS := -Eeuo pipefail -c

SCRIPT_DIR := $(CURDIR)/scripts
CLI := $(SCRIPT_DIR)/cli.sh

.PHONY: help \
	build build-kde build-nonkde build-android build-all \
	test test-kde test-nonkde test-all \
	run run-kde run-nonkde run-android-emulator run-android-device \
	format check \
	android-env android-deps-emulator android-deps-device android-deps-build \
	android-build android-run-emulator android-run-device

help:
	$(CLI) help

build: build-all

build-kde:
	$(CLI) build kde

build-nonkde:
	$(CLI) build nonkde

build-android:
	$(CLI) build android

build-all:
	$(CLI) build all

test: test-all

test-kde:
	$(CLI) test kde

test-nonkde:
	$(CLI) test nonkde

test-all:
	$(CLI) test all

run-kde:
	$(CLI) run kde

run-nonkde:
	$(CLI) run nonkde

run-android-emulator:
	$(CLI) run android-emulator

run-android-device:
	$(CLI) run android-device

format:
	$(CLI) format

android-env:
	$(CLI) android env

android-deps-emulator:
	$(CLI) android deps emulator

android-deps-device:
	$(CLI) android deps device

android-deps-build:
	$(CLI) android deps build

android-build:
	$(CLI) android build

android-run-emulator:
	$(CLI) android run-emulator

android-run-device:
	$(CLI) android run-device

check:
	$(CLI) check
