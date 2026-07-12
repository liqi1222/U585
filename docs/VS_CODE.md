# VS Code CMake Build and Debug

This workspace builds the STM32U585 TrustZone Secure and NonSecure images through the root CMake presets and debugs either image with the on-board ST-LINK.

## Prerequisites

Install these VS Code extensions:

- CMake Tools (`ms-vscode.cmake-tools`)
- C/C++ (`ms-vscode.cpptools`)
- Cortex-Debug (`marus25.cortex-debug`)

The checked-in Windows launch configurations target STM32CubeCLT 1.21.0 under `C:/ST/STM32CubeCLT_1.21.0`. If CubeCLT is installed elsewhere, update these three values in both entries in `.vscode/launch.json`:

- `armToolchainPath`
- `serverpath`
- `stm32cubeprogrammer`

`cmake`, `ninja`, and the `arm-none-eabi-*` tools must also be available on `PATH`. Open `U585.code-workspace`, not the `Secure` or `NonSecure` subdirectory by itself, so the root preset can preserve the required Secure-before-NonSecure build order.

## Configure and Build

On first open, CMake Tools configures the root `Debug` preset automatically. The equivalent command-palette workflow is:

1. Run **CMake: Select Configure Preset** and choose `Debug`.
2. Run **CMake: Configure**.
3. Run **CMake: Build**, or press `Ctrl+Shift+B` and select `CMake: build Debug`.

The terminal equivalent is:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Successful builds produce:

- `Secure/build/U585_S.elf`
- `NonSecure/build/U585_NS.elf`

The root project always builds Secure first because the NonSecure link consumes `Secure_nsclib/secure_nsclib.o`.

## Debug with the On-board ST-LINK

Connect the board's ST-LINK USB port, open **Run and Debug**, and select one of:

- **U585 NonSecure (ST-LINK)** for normal application development.
- **U585 Secure (ST-LINK)** for Secure boot, GTZC, and TrustZone handoff work.

Press `F5`. The pre-launch task configures and builds the Debug preset. Both ELF files are programmed on every launch, while GDB loads symbols only for the selected context so the two images' `main` functions are unambiguous.

The launch configurations use a verified `1000 kHz` SWD clock. This board/probe combination reports that rate as the reliable connection frequency.

Clicking **Stop Debugging** ends GDB and the GDB server, then runs `ST-LINK: reset and release target`. The post-debug task reconnects at 1000 kHz, resets the MCU, exits immediately, and leaves the probe available to the next session. If Cursor's toolbar still looks active after the processes have exited, run **Developer: Reload Window**; that is stale UI state rather than an occupied probe.

The NonSecure launch resets through the Secure image, hands off to the NonSecure vector table, and stops at `NonSecure/Core/Src/main.c:main`. The Secure launch stops at `Secure/Core/Src/main.c:main`.

## Build Another NonSecure Demo

The root `Debug` preset builds demo 03. To compile a different article demo without changing the shared preset, configure the NonSecure project directly:

```powershell
cmake -S NonSecure -B NonSecure/build/Debug-demo04 -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE=../gcc-arm-none-eabi.cmake `
  -DCMAKE_BUILD_TYPE=Debug `
  -DU585_ACTIVE_DEMO=4
cmake --build NonSecure/build/Debug-demo04
```

The checked-in F5 launch intentionally uses the root build and demo 03 because it must rebuild the matching Secure import library and program both TrustZone images.

## Troubleshooting

- **CMake cannot find Ninja or `arm-none-eabi-gcc`:** launch VS Code from an environment where STM32CubeCLT is on `PATH`, or add the CubeCLT tool directories to the VS Code integrated terminal environment.
- **ST-LINK GDB server cannot find STM32CubeProgrammer:** correct `stm32cubeprogrammer` in both `.vscode/launch.json` entries; the value must be the directory containing the programmer binaries. Session-level paths are intentional because Cortex-Debug 1.12.1 may ignore folder-level platform path settings when the project is opened through a multi-root `.code-workspace` file.
- **No ST-LINK detected:** verify the ST-LINK USB connector/cable, close STM32CubeProgrammer or another GDB server that owns the probe, then reconnect the board.
- **NonSecure breakpoint is not reached:** confirm the option bytes and TrustZone memory split documented in `U585_EXPERIMENTS.md`, then debug **U585 Secure (ST-LINK)** to inspect the handoff.
- **IntelliSense is stale:** run **CMake: Delete Cache and Reconfigure**. The C/C++ extension reads `NonSecure/build/compile_commands.json` after the root configure/build has created it.
