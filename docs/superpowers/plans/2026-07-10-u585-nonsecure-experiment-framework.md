# U585 NonSecure Experiment Framework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a NonSecure-side experiment framework that maps U585 article order to isolated test code and keeps TrustZone-specific work separate.

**Architecture:** `NonSecure/Core/Src/main.c` remains the generated application entry and calls a small `U585_App_Init()` / `U585_App_Loop()` facade. `NonSecure/App` owns article-facing demo code, board helpers, and active demo dispatch selected by `U585_ACTIVE_DEMO`. The Secure project remains responsible for the existing TrustZone boot handoff and current CubeMX-generated Secure peripheral setup.

**Tech Stack:** STM32CubeMX CMake project, ARM GCC, STM32 HAL/LL, Python structural verification script.

---

### Task 1: Add Structural Verification

**Files:**
- Create: `tools/check_experiment_layout.py`

- [x] **Step 1: Write the failing structural test**

Add `tools/check_experiment_layout.py` to assert the planned files, CMake hooks, dispatcher entries, `main.c` calls, and project documentation exist.

- [x] **Step 2: Run test to verify it fails**

Run: `python tools/check_experiment_layout.py`

Expected: FAIL with missing `NonSecure/App/...` files and `U585_EXPERIMENTS.md`.

### Task 2: Add NonSecure Experiment Framework

**Files:**
- Create: `NonSecure/App/Inc/u585_app.h`
- Create: `NonSecure/App/Inc/u585_board.h`
- Create: `NonSecure/App/Inc/u585_demo.h`
- Create: `NonSecure/App/Src/u585_app.c`
- Create: `NonSecure/App/Src/u585_board.c`
- Create: `NonSecure/App/Src/exp03_led_button.c`
- Create: `NonSecure/App/Src/exp04_debug_fault.c`
- Create: `NonSecure/App/Src/exp05_clock_tree.c`

- [x] **Step 1: Implement the facade and dispatcher**

Expose `U585_App_Init()` and `U585_App_Loop()`, then dispatch to demo 03, 04, or 05 by `U585_ACTIVE_DEMO`.

- [x] **Step 2: Implement board helpers**

Provide local NonSecure GPIO aliases for PH6, PH7, and PC13, plus initialization helpers. These helpers are intentionally local to the app layer because the current CubeMX-generated NonSecure `main.h` does not define the board LED/button symbols.

- [x] **Step 3: Implement first article demos**

Demo 03 toggles LED state and polls the button. Demo 04 provides optional ITM output and a deliberate fault hook disabled by default. Demo 05 exposes a clock snapshot structure and heartbeat loop for clock-tree experiments.

### Task 3: Wire CMake and Generated Entry

**Files:**
- Modify: `NonSecure/CMakeLists.txt`
- Modify: `NonSecure/Core/Src/main.c`

- [x] **Step 1: Add CMake option and sources**

Add `U585_ACTIVE_DEMO` as a cache variable defaulting to `3`, include `App/Inc`, and compile the app sources.

- [x] **Step 2: Call app facade from generated `main.c` user sections**

Include `u585_app.h`, call `U585_App_Init()` in `USER CODE BEGIN 2`, and call `U585_App_Loop()` in the main loop.

### Task 4: Add Engineering Notes

**Files:**
- Create: `U585_EXPERIMENTS.md`

- [x] **Step 1: Document build and switching**

Document the active demo selection, root build behavior, direct NonSecure build behavior, and how to add future article demos.

- [x] **Step 2: Document TrustZone boundary**

State that non-TrustZone experiments default to NonSecure code, while articles 32 and 33 remain Secure/TrustZone-specific.

### Task 5: Verify

**Files:**
- Run-only verification

- [x] **Step 1: Run structural verification**

Run: `python tools/check_experiment_layout.py`

Expected: PASS and `experiment layout check passed`.

- [x] **Step 2: Reconfigure and build root Debug preset**

Run: `cmake --preset Debug` and `cmake --build --preset Debug`

Expected: both Secure and NonSecure images build successfully.

- [x] **Step 3: Build direct NonSecure demo selection**

Run direct NonSecure configure/build with `-DU585_ACTIVE_DEMO=4`, then with `-DU585_ACTIVE_DEMO=5`.

Expected: both selections build successfully.
