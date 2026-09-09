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
    33: "exp33_ospi_otfdec.c",
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
    "docs/PERFORMANCE_BASELINE_05_15.md",
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
    require("App/Src/u585_tim2_trgo.c" in cmake,
            "NonSecure CMake must compile the shared TIM2 TRGO helper")

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
    require("PWR.PowerMode=PWR_SMPS_SUPPLY" in ioc, "IOC must select SMPS for the performance baseline")
    for clock in [
        "RCC.SYSCLKFreq_VALUE=160000000",
        "RCC.HCLKFreq_Value=160000000",
        "RCC.APB1Freq_Value=160000000",
        "RCC.APB2Freq_Value=160000000",
        "RCC.APB3Freq_Value=160000000",
    ]:
        require(clock in ioc, f"IOC must retain 160 MHz performance clock: {clock}")
    require("VP_ICACHE_VS_ICACHE.Mode=DefaultMode" in ioc,
            "IOC must use the default 2-way ICACHE mode")
    require("SPI2.VirtualNSS=VM_NSSSOFT" in ioc,
            "IOC SPI2 NSS must match the NonSecure software-NSS driver")
    require("PB12.PinAttribute=CortexM33NS" in ioc,
            "IOC must assign the software SPI2 NSS pin to NonSecure")
    require("PB12.Signal=GPIO_Output" in ioc,
            "IOC must configure PB12 as a GPIO software NSS")
    secure_spi = read_text("Secure/Core/Src/spi.c")
    require("hspi2.Init.NSS = SPI_NSS_SOFT" in secure_spi,
            "CubeMX Secure SPI2 init must match the IOC software-NSS selection")
    require("hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE" in secure_spi,
            "CubeMX Secure SPI2 init must not pulse a software NSS")
    require("GPIO_PIN_12" not in secure_spi,
            "CubeMX Secure SPI2 init must not configure PB12 as an SPI alternate function")
    require("WRLS_SPI2_NSS" not in secure_spi,
            "CubeMX Secure SPI2 init must not reference the NonSecure GPIO NSS pin")

    for setting in [
        "ADC1.ExternalTrigConv=ADC_EXTERNALTRIG_T2_TRGO",
        "ADC1.ConversionDataManagement=ADC_CONVERSIONDATA_DMA_CIRCULAR",
        "ADC1.DMAContinuousRequests=ENABLE",
        "DAC1.DAC_Trigger-DAC_OUT1=DAC_TRIGGER_T2_TRGO",
        "TIM2.TIM_MasterOutputTrigger=TIM_TRGO_UPDATE",
        "GPDMA1.LINKEDLISTMODE_GPDMACH1=DMA_LINKEDLIST_CIRCULAR",
        "GPDMA1.LINKEDLISTMODE_GPDMACH2=DMA_LINKEDLIST_CIRCULAR",
        "PA4.PinAttribute=CortexM33NS",
    ]:
        require(setting in ioc, f"IOC must describe timer-paced circular DMA: {setting}")

    dma_source = read_text("NonSecure/App/Src/u585_gpdma.c")
    for token in [
        "hdma_gpdma1_ch1_adc_ns",
        "hdma_gpdma1_ch2_dac_ns",
        "GPDMA1_Channel1",
        "GPDMA1_Channel2",
        "GPDMA1_REQUEST_ADC1",
        "GPDMA1_REQUEST_DAC1_CH1",
        "DMA_LINKEDLIST_CIRCULAR",
        "HAL_DMAEx_List_BuildNode",
        "HAL_DMAEx_List_SetCircularMode",
        "HAL_DMAEx_List_LinkQ",
        "DMA_MEMORY_TO_PERIPH,\n                                   DMA_SINC_INCREMENTED,\n                                   DMA_DINC_FIXED,\n                                   DMA_SRC_DATAWIDTH_WORD,\n                                   DMA_DEST_DATAWIDTH_WORD",
    ]:
        require(token in dma_source, f"GPDMA helper missing circular stream support: {token}")

    adc_source = read_text("NonSecure/App/Src/u585_adc1.c")
    for token in [
        "U585_ADC1_InitTimerDma",
        "U585_ADC1_StartTimerDma",
        "ADC_EXTERNALTRIG_T2_TRGO",
        "ADC_CONVERSIONDATA_DMA_CIRCULAR",
        "__HAL_LINKDMA(&hadc1_ns, DMA_Handle, hdma_gpdma1_ch1_adc_ns)",
    ]:
        require(token in adc_source, f"ADC stream configuration missing: {token}")

    dac_source = read_text("NonSecure/App/Src/u585_dac1.c")
    for token in [
        "U585_DAC1_InitTimerDma",
        "U585_DAC1_StartTimerDma",
        "DAC_TRIGGER_T2_TRGO",
        "DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC",
        "__HAL_LINKDMA(&hdac1_ns, DMA_Handle1, hdma_gpdma1_ch2_dac_ns)",
    ]:
        require(token in dac_source, f"DAC stream configuration missing: {token}")

    exp14_source = read_text("NonSecure/App/Src/exp14_adc_dma.c")
    for token in [
        "HAL_ADC_ConvHalfCpltCallback",
        "HAL_ADC_ConvCpltCallback",
        "U585_ADC1_StartTimerDma",
        "U585_TIM2_TRGO_Start",
        "dma_used = 1U",
        "vref_cal <= 0x3FFFU",
        "vref_raw << 2U",
    ]:
        require(token in exp14_source, f"Demo 14 must report a live DMA stream: {token}")

    exp15_source = read_text("NonSecure/App/Src/exp15_dac_output.c")
    for token in [
        "HAL_DAC_ConvHalfCpltCallbackCh1",
        "HAL_DAC_ConvCpltCallbackCh1",
        "U585_DAC1_StartTimerDma",
        "U585_TIM2_TRGO_Start",
        "dma_used = 1U",
        "static __ALIGNED(32) uint32_t s_exp15_wave",
    ]:
        require(token in exp15_source, f"Demo 15 must report a live DMA stream: {token}")

    secure_main = read_text("Secure/Core/Src/main.c")
    for token in [
        "GPDMA1_Channel1",
        "GPDMA1_Channel2",
        "NVIC_SetTargetState(ADC1_IRQn)",
        "NVIC_SetTargetState(DAC1_IRQn)",
        "NVIC_SetTargetState(GPDMA1_Channel1_IRQn)",
        "NVIC_SetTargetState(GPDMA1_Channel2_IRQn)",
    ]:
        require(token in secure_main, f"Secure startup must release DMA stream IRQ resource: {token}")

    ns_irq = read_text("NonSecure/Core/Src/stm32u5xx_it.c")
    for token in [
        "GPDMA1_Channel1_IRQHandler",
        "GPDMA1_Channel2_IRQHandler",
        "HAL_DMA_IRQHandler(&hdma_gpdma1_ch1_adc_ns)",
        "HAL_DMA_IRQHandler(&hdma_gpdma1_ch2_dac_ns)",
        "ADC1_IRQHandler",
        "DAC1_IRQHandler",
    ]:
        require(token in ns_irq, f"NonSecure IRQ routing missing: {token}")

    root_presets = read_text("CMakePresets.json")
    require('"name": "Performance"' in root_presets,
            "Root CMake presets must expose a Performance build")
    require('"CMAKE_BUILD_TYPE": "Performance"' in root_presets,
            "Performance preset must select the custom performance build type")
    toolchain = read_text("gcc-arm-none-eabi.cmake")
    require('set(CMAKE_C_FLAGS_PERFORMANCE "-O2 -g3 -DNDEBUG")' in toolchain,
            "Toolchain must define the -O2 performance C flags")
    for path in ["Secure/CMakePresets.json", "NonSecure/CMakePresets.json"]:
        child_presets = read_text(path)
        require('"name": "Performance"' in child_presets,
                f"{path} must expose the Performance preset")
        require('"CMAKE_BUILD_TYPE": "Performance"' in child_presets,
                f"{path} must select the custom performance build type")
    mx_generated = read_text("mx-generated.cmake")
    require(mx_generated.count("-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}") == 2,
            "Root external-project configuration must forward build type to both images")

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

    exp05_source = read_text("NonSecure/App/Src/exp05_clock_tree.c")
    for field in ["smps_selected", "voltage_range", "icache_enabled", "icache_2ways"]:
        require(field in exp05_source, f"Demo 05 must expose performance field: {field}")

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
    require("PERFORMANCE_BASELINE_05_15.md" in doc,
            "Experiment overview must link to the 05--15 performance baseline")
    require("unmeasured\nStop/Standby and ADC/DAC streaming-DMA work" not in doc,
            "Experiment overview must not describe the implemented ADC/DAC streams as unmeasured")
    require("ADC1 VREFINT/TEMP poll" not in doc,
            "Experiment overview must not retain the Demo 14 polling-only status")
    require("DAC1 CH1 PA4 DC code" not in doc,
            "Experiment overview must not retain the Demo 15 DC-only status")
    for token in ["TIM2 TRGO", "GPDMA1 CH1", "GPDMA1 CH2", "PA4 stepped waveform"]:
        require(token in doc, f"Experiment overview missing stream status: {token}")
    performance_doc = read_text("docs/PERFORMANCE_BASELINE_05_15.md")
    for token in ["160 MHz", "SMPS", "VOS1", "ICACHE", "Performance", "U585_ACTIVE_DEMO=5",
                  "TIM2", "GPDMA", "linked-list circular", "PA4"]:
        require(token in performance_doc, f"Performance document missing token: {token}")

    flash_script = read_text("tools/flash_and_capture.ps1")
    require('$root = (Resolve-Path (Join-Path $PSScriptRoot ".."))' in flash_script,
            "Flash script must derive the project root from its own location")
    require('[string]$Preset = "Performance"' in flash_script,
            "Flash script must default to the performance build preset")
    require('cmake --preset $Preset "-DU585_ACTIVE_DEMO:STRING=$Demo"' in flash_script,
            "Flash script must configure the selected preset")
    require('$line = $line.TrimEnd()' in flash_script,
            "Flash script must trim serial line trailing whitespace before saving logs")

    print("experiment layout check passed")


if __name__ == "__main__":
    main()
