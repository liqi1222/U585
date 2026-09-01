# VS Code CMake Build and Debug Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a reproducible VS Code workflow that builds both STM32U585 TrustZone images with the root CMake Debug preset and debugs the board through its on-board ST-LINK.

**Architecture:** CMake Tools owns preset-based configure/build operations. Cortex-Debug launches the STM32CubeCLT ST-LINK GDB server, programs both ELF images, loads both symbol tables, and stops in the NonSecure `main` function.

**Tech Stack:** VS Code, CMake Tools, CMake presets, Ninja, GNU Arm Embedded Toolchain, Cortex-Debug, STM32CubeCLT ST-LINK GDB server

---

### Task 1: Add VS Code project configuration

**Files:**
- Create: `.vscode/settings.json`
- Create: `.vscode/tasks.json`
- Create: `.vscode/launch.json`
- Create: `.vscode/extensions.json`

- [ ] **Step 1: Verify the configuration does not exist yet**

Run: `Test-Path .vscode/settings.json; Test-Path .vscode/tasks.json; Test-Path .vscode/launch.json; Test-Path .vscode/extensions.json`

Expected: each command reports `False`.

- [ ] **Step 2: Add preset and IntelliSense settings**

Create `.vscode/settings.json` with preset mode enabled, configure-on-open enabled, the NonSecure compile database selected for C/C++ IntelliSense, and Windows CubeCLT paths centralized for Cortex-Debug.

- [ ] **Step 3: Add configure/build tasks**

Create `.vscode/tasks.json` with CMake Tools `configure` and `build` tasks bound to the `Debug` preset. Make the build task depend on configure so a clean checkout can start debugging directly.

- [ ] **Step 4: Add the TrustZone debug launch**

Create `.vscode/launch.json` with `type: cortex-debug`, `servertype: stlink`, `interface: swd`, the NonSecure ELF as `executable`, both ELFs in `loadFiles` and `symbolFiles`, the build task as `preLaunchTask`, and `runToEntryPoint: main`.

- [ ] **Step 5: Add extension recommendations**

Create `.vscode/extensions.json` recommending `ms-vscode.cmake-tools`, `ms-vscode.cpptools`, and `marus25.cortex-debug`.

- [ ] **Step 6: Parse every JSON file**

Run: `Get-ChildItem .vscode -Filter *.json | ForEach-Object { Get-Content -Raw $_.FullName | ConvertFrom-Json | Out-Null }`

Expected: exit code 0 with no JSON parse errors.

### Task 2: Document the workflow

**Files:**
- Create: `docs/VS_CODE.md`
- Modify: `U585.code-workspace`

- [ ] **Step 1: Document prerequisites and commands**

Describe required extensions, CubeCLT path overrides, `CMake: Configure`, `CMake: Build`, F5 debugging, dual-image programming order, direct demo selection, and common ST-LINK errors in `docs/VS_CODE.md`.

- [ ] **Step 2: Add workspace recommendations without duplicating folder settings**

Update `U585.code-workspace` to recommend opening the workspace while leaving build/debug settings in `.vscode` as the single source of truth.

- [ ] **Step 3: Check documentation paths against the filesystem**

Run: `Test-Path CMakePresets.json; Test-Path Secure/build/U585_S.elf; Test-Path NonSecure/build/U585_NS.elf`

Expected after Task 3 build: all three report `True`.

### Task 3: Verify build and hardware debugging

**Files:**
- Verify: `build/Debug/`
- Verify: `Secure/build/U585_S.elf`
- Verify: `NonSecure/build/U585_NS.elf`

- [ ] **Step 1: Run the structural test**

Run: `python tools/check_experiment_layout.py`

Expected: exit code 0 and the layout check reports success.

- [ ] **Step 2: Configure from the root Debug preset**

Run: `cmake --preset Debug`

Expected: exit code 0, Ninja generation succeeds, and `build/Debug` is written.

- [ ] **Step 3: Build both TrustZone images**

Run: `cmake --build --preset Debug`

Expected: exit code 0 and both `Secure/build/U585_S.elf` and `NonSecure/build/U585_NS.elf` exist.

- [ ] **Step 4: Verify debug information and entry symbols**

Run: `arm-none-eabi-readelf -S NonSecure/build/U585_NS.elf` and `arm-none-eabi-nm NonSecure/build/U585_NS.elf | Select-String ' main$'`

Expected: `.debug_info` is present and `main` resolves to a text symbol.

- [ ] **Step 5: Query the connected ST-LINK**

Run `ST-LINK_gdbserver -cp C:/ST/STM32CubeCLT_1.21.0/STM32CubeProgrammer/bin -q` using the full CubeCLT executable path.

Expected: at least one ST-LINK serial number is listed. If none is connected, report hardware verification as blocked while preserving successful configuration/build evidence.

- [ ] **Step 6: Start the GDB server and perform a scripted GDB smoke test**

Start `ST-LINK_gdbserver` on a temporary local port with SWD and the CubeProgrammer path. Connect `arm-none-eabi-gdb` with both symbol files, load both ELF images, reset, set a temporary breakpoint on NonSecure `main`, continue, and confirm the breakpoint is hit.

Expected: GDB reports a breakpoint at `NonSecure/Core/Src/main.c` and the program counter stops there.

- [ ] **Step 7: Re-parse configuration and re-run the complete build**

Run the JSON parse command, `python tools/check_experiment_layout.py`, `cmake --preset Debug`, and `cmake --build --preset Debug` again.

Expected: every command exits 0.

No commit steps are included because this workspace is not a Git repository.

