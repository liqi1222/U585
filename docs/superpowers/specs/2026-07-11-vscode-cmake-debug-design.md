# VS Code CMake Build and Debug Design

## Goal

Provide a checked-in VS Code setup that configures and builds the existing STM32U585 TrustZone project through its root CMake presets, then launches a board-level SWD debug session through the on-board ST-LINK.

## Selected Approach

Use CMake Tools for configure/build and Cortex-Debug for the debug session. The root `Debug` preset remains the single build entry point, so its existing `ExternalProject` flow builds Secure first and NonSecure second. Separate Secure and NonSecure launch configurations each program both ELF files but load symbols from only their primary executable. This avoids the ambiguous `main` symbol created when both TrustZone images are placed in the same GDB symbol namespace.

This is preferred over the STM32 vendor-specific VS Code debug type because the project already uses standard CMake presets and Cortex-Debug is installed. A manual `tasks.json` plus `cppdbg` setup would duplicate CMake Tools and require manually managing the GDB server lifecycle.

## Files and Responsibilities

- `.vscode/settings.json`: enable preset-driven CMake configuration and expose NonSecure compile commands.
- `.vscode/tasks.json`: provide the debugger's pre-launch build task and a post-debug target reset task that exits after releasing ST-LINK.
- `.vscode/launch.json`: provide Secure and NonSecure Cortex-Debug launches using ST-LINK/SWD, with session-level CubeCLT paths that remain effective when the project is opened through the workspace file; each programs both TrustZone images and stops at the selected image's `main` function.
- `.vscode/extensions.json`: recommend the minimum extensions needed on another workstation.
- `U585.code-workspace`: keep the workspace rooted at the project and allow folder-level `.vscode` settings to remain authoritative.
- `docs/VS_CODE.md`: document prerequisites, normal build/debug workflow, selectable demo builds, and troubleshooting.

## Portability

The configuration prefers executable names resolved from `PATH` for CMake and Ninja. Windows-specific Cortex-Debug paths point to the installed STM32CubeCLT 1.21.0 tree on this machine because the ST-LINK server otherwise cannot reliably locate STM32CubeProgrammer. They are set directly on both launch configurations because Cortex-Debug 1.12.1 can ignore folder-level platform settings when this multi-root workspace file is used.

## Debug Flow

1. CMake Tools configures the root `Debug` preset in `build/Debug`.
2. The pre-launch task builds the root project, which builds `Secure/build/U585_S.elf` and then `NonSecure/build/U585_NS.elf`.
3. Cortex-Debug starts `ST-LINK_gdbserver`, passing the STM32CubeProgrammer location and the hardware-verified 1000 kHz SWD frequency.
4. GDB programs both ELF images and loads the symbol table for the selected Secure or NonSecure context.
5. The target resets and stops at the selected image's `main`; the NonSecure launch first runs through Secure boot and handoff.

## Error Handling

The pre-launch task stops the debug launch if configuration or compilation fails. Missing build artifacts are reported by Cortex-Debug before connecting. Missing CubeCLT paths are documented with the exact settings to update. Hardware validation distinguishes between a successful configuration/build and a board connection failure.

## Verification

- Parse all JSON configuration files.
- Run the repository's experiment-layout check.
- Delete or use a clean VS Code build directory, then run `cmake --preset Debug` and `cmake --build --preset Debug`.
- Confirm both ELF files exist and contain debug information.
- Query the ST-LINK probe, start the GDB server, connect with `arm-none-eabi-gdb`, load both images, reset, and confirm a breakpoint can be reached in NonSecure `main` when hardware is connected.
