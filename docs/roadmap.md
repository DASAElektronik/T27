<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Roadmap and acceptance gates

## 1. Consolidate the integer core

Integrate the 0.2.0 corrections, independent tests, install/package checks and
combined documentation. Keep old behavior changes explicit in the migration guide.
Validate Linux and Windows Debug/Release, sanitizers, coverage and timed fuzzing.
A release is gated on those results; a candidate version is not a published tag.

## 2. Executable ISA model

Define registers, word/address sizes, instruction encodings, memory semantics,
exceptions and arithmetic flags. Implement an emulator and small test programs.
Specify ABI, calling convention, boot path, I/O and interrupts before OS work.

## 3. FPGA reference

Map the accepted ISA and encodings into RTL, test against the software model,
then measure resources, timing and power on an explicitly identified platform.
Use a matched binary baseline before making efficiency claims.

## 4. System software

Build minimal boot/runtime services, drivers and then OS primitives on the
validated processor model. Keep reproducible tests across emulator and FPGA.

## Separate research tracks

Resolve the floating-point RFC value/encoding contract before implementation.
Stochastic computing needs representation, generator-correlation and error bounds
before throughput estimates become meaningful system results.
