from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


DEMO_SOURCES = {
    3: "exp03_led_button.c",
    4: "exp04_debug_fault.c",
    5: "exp05_clock_tree.c",
    6: "exp06_power_supply.c",
    7: "exp07_low_power_current.c",
    8: "exp08_rtc_wakeup_backup.c",
    9: "exp09_gpio_exti.c",
    10: "exp10_tim_pwm_input_capture.c",
    11: "exp11_uart_printf_log.c",
    12: "exp12_i2c_sensor_bus.c",
    13: "exp13_spi_bus.c",
    14: "exp14_adc_dma.c",
    15: "exp15_dac_output.c",
    16: "exp16_gpdma_transfer.c",
    17: "exp17_hts221_sensor.c",
    18: "exp18_lps22hh_sensor.c",
    19: "exp19_ism330dhcx_imu.c",
    20: "exp20_iis2mdc_compass.c",
    21: "exp21_vl53l5cx_tof.c",
    22: "exp22_pdm_microphone.c",
    23: "exp23_ospi_flash_xip.c",
    24: "exp24_ospi_psram_cache.c",
    25: "exp25_st25dv_nfc.c",
    26: "exp26_usb_ucpd_device.c",
    27: "exp27_wifi_emw3080.c",
    28: "exp28_mqtt_cloud.c",
    29: "exp29_icache_fetch.c",
    30: "exp30_rng_aes_pka.c",
    31: "exp31_trustzone_gtzc.c",
    32: "exp32_tfm_secure_boot.c",
}


REQUIRED_FILES = [
    "NonSecure/App/Inc/u585_app.h",
    "NonSecure/App/Inc/u585_board.h",
    "NonSecure/App/Inc/u585_demo.h",
    "NonSecure/App/Inc/u585_log.h",
    "NonSecure/App/Src/u585_app.c",
    "NonSecure/App/Src/u585_board.c",
    "NonSecure/App/Src/u585_log.c",
    "U585_EXPERIMENTS.md",
] + [f"NonSecure/App/Src/{name}" for name in DEMO_SOURCES.values()]


def read_text(path):
    return (ROOT / path).read_text(encoding="utf-8")


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def main():
    missing = [path for path in REQUIRED_FILES if not (ROOT / path).is_file()]
    require(not missing, "Missing files: " + ", ".join(missing))

    cmake = read_text("NonSecure/CMakeLists.txt")
    require("U585_ACTIVE_DEMO" in cmake, "NonSecure CMake must expose U585_ACTIVE_DEMO")
    require("NonSecure/App/Inc" not in cmake, "Use paths relative to NonSecure in CMake")
    base_sources = [
        "App/Src/u585_app.c",
        "App/Src/u585_board.c",
        "App/Src/u585_log.c",
    ]
    for source in base_sources + [f"App/Src/{name}" for name in DEMO_SOURCES.values()]:
        require(source in cmake, f"Missing CMake source: {source}")

    main_c = read_text("NonSecure/Core/Src/main.c")
    require('#include "u585_app.h"' in main_c, "main.c must include u585_app.h")
    require("U585_App_Init();" in main_c, "main.c must call U585_App_Init")
    require("U585_App_Loop();" in main_c, "main.c must call U585_App_Loop")

    secure_header = read_text("Secure_nsclib/secure_nsc.h")
    secure_source = read_text("Secure/Core/Src/secure_nsc.c")
    secure_gpio = read_text("Secure/Core/Src/gpio.c")
    partition = read_text("Secure/Core/Inc/partition_stm32u585xx.h")
    ioc = read_text("U585.ioc")
    require("SECURE_UART1_WriteString" in secure_header, "Secure NSC header must export UART logging")
    require("SECURE_UART1_WriteString" in secure_source, "Secure NSC source must implement UART logging")
    require("#define SAU_INIT_REGION0    1" in partition, "NSC veneer flash SAU region must be enabled")
    require("#define SAU_INIT_REGION1    1" in partition, "NonSecure flash SAU region must be enabled")
    require("#define SAU_INIT_REGION2    1" in partition, "NonSecure SRAM SAU region must be enabled")
    require("#define SAU_INIT_NSC0       1" in partition, "NSC veneer flash region must be marked NSC")
    require("#define SAU_INIT_START1     0x08100000" in partition, "NonSecure flash SAU start must match linker")
    require("#define SAU_INIT_START2     0x20040000" in partition, "NonSecure SRAM SAU start must match linker")
    require("GPIOH->SECCFGR" in secure_gpio, "Secure GPIO init must configure GPIOH security attributes")
    require("GPIO_SECCFGR_SEC6" in secure_gpio, "Secure GPIO init must release PH6 to NonSecure")
    require("GPIO_SECCFGR_SEC7" in secure_gpio, "Secure GPIO init must release PH7 to NonSecure")
    require("PH6.PinAttribute=CortexM33NS" in ioc, "IOC must document PH6 as NonSecure")
    require("PH7.PinAttribute=CortexM33NS" in ioc, "IOC must document PH7 as NonSecure")

    log_source = read_text("NonSecure/App/Src/u585_log.c")
    require("SECURE_UART1_WriteString" in log_source, "NonSecure log must use Secure UART bridge")

    exp03_source = read_text("NonSecure/App/Src/exp03_led_button.c")
    require("g_u585_exp03_state" in exp03_source, "Demo 03 must expose a SWD-readable state block")
    require("0xA5850003UL" in exp03_source, "Demo 03 state block must include its magic value")
    require("red_led_state" in exp03_source, "Demo 03 state block must expose red LED state")
    require("green_led_state" in exp03_source, "Demo 03 state block must expose green LED state")
    require("U585_Board_SetGreenLed" in exp03_source, "Demo 03 must drive the green LED explicitly")
    require("exp03_log_led_state" in exp03_source, "Demo 03 must provide a per-phase LED state logger")
    require("[U585][03] led_state=" in exp03_source, "Demo 03 must log LED state")

    app_c = read_text("NonSecure/App/Src/u585_app.c")
    for demo in DEMO_SOURCES:
        require(f"U585_ACTIVE_DEMO == {demo}" in app_c, f"Dispatcher missing demo {demo}")
        require(f"U585_Demo_Exp{demo:02d}" in app_c, f"Dispatcher missing demo object {demo:02d}")
    require("U585_Demo_Unsupported" in app_c, "Dispatcher must provide unsupported-demo fallback")

    doc = read_text("U585_EXPERIMENTS.md")
    for token in [f"{demo:02d}" for demo in DEMO_SOURCES] + ["31", "32", "NonSecure", "TrustZone"]:
        require(token in doc, f"Document missing token: {token}")
    require("SECURE_UART1_WriteString" in doc, "Document must describe the Secure UART bridge")
    require("SECWM2_PSTRT=0x7F" in doc, "Document must describe Bank2 NonSecure OB setting")

    print("experiment layout check passed")


if __name__ == "__main__":
    main()
