# 2026-11-06 待发布稿件修订补测

## 条件与边界

- 板卡：B-U585I-IOT02A；ST-LINK/V3E SN `001E003B4D46501220383832`，V3J17M10；器件 ID `0x482`、Rev X。
- 固件源码提交：`9a601ffdcc3f053106afa9cb6b379ef13a4de016`；本轮没有修改固件源码。
- 工具：本机 STM32CubeCLT 1.21.0 中的 CMake、Ninja、arm-none-eabi 工具及 STM32CubeProgrammer CLI。
- 每次使用 `Performance`，显式配置对应 `U585_ACTIVE_DEMO`；Fault 触发及自动 Sleep 均关闭。
- 每个 demo 均重新配置、构建 Secure/NonSecure，检查 NS ELF 中的 demo 标记，再以 SWD 1000 kHz 烧录并执行下载校验；两镜像均报告验证成功。
- 双镜像沿用现有 TrustZone/GTZC 配置；没有操作选项字节、读保护、密钥持久化或安全启动配置。
- 串口：COM3、115200 8N1；打开后重置，再连续拼接原始串口数据块。没有将每个接收块强行按行拆开。
- 开始重置之前可能读入已有运行的残片或横幅；分析以文件内**最后一次 `[S] Secure world ready` 之后**的报告为准，不把两次启动计数拼起来。
- 捕获秒数为主机软件窗口，串口报告没有逐条设备时间戳；不能由此校准频率、抖动或吞吐。
- 没有万用表、示波器、逻辑分析仪测量；没有人工按键、声压校准、NFC 手机、USB PD 对端、Wi-Fi/MQTT 联网测试。
- 新日志单独保存在本目录，旧 `demoXX-com3.txt` 均未覆盖。

## 结果

| demo | 捕获窗口 | 复位后观察 | 结论与未覆盖项 |
| --- | --- | --- | --- |
| 15 DAC | 30 s | ready/started=1；59 组报告；HT/TC 2/2 → 2322/2322；DOR 抽样1300、1950；CSR=131072 | 循环数字供数持续发生；没有 PA4 电压、幅度或波形测量 |
| 16 GPDMA | 8 s | ready=1、xfer_ok=1、words=16、mismatch=0；src0=dst0=2768240640 | 64 B 逐字比较通过；不证明 memcpy 加速、异步CPU释放或链表 |
| 14 ADC | 30 s | ready/started=1；59 组报告；VREFINT raw1479–1484，公式估算VDDA3300–3311 mV；HT/TC 2/2 → 2336/2335；CSR=0 | TIM2→ADC→DMA链持续运行；10 kHz为配置报告，未校准精度/抖动 |
| 12 I2C扫描 | 10 s | PF11=1、PH1=1；发现9个7位地址：0x10、0x1E、0x20、0x29、0x56、0x5D、0x5E、0x5F、0x6B | 0x29在释放LPn后可见；地址应答不等于各传感器功能完成 |
| 21 ToF | 10 s | lpn_high=1、probe_ok=1、is_alive=1、device_id=1、revision_id=240、uld_ready=0 | 01/F0只通过源码宽松存活条件，不满足ULD严格F0/02组合；未加载ULD/测距 |
| 22 ADF麦克风 | 10 s | adf_ready=1、sample_ok=1，6次采样；dma_used=0 | 轮询读到滤波结果；负样本通过uint32日志打印，不是巨大正声压；非连续PCM录音 |
| 23 Flash | 10 s | ready=1、jedec_ok=1；ID C2/85/3A；SPI 1-1-1 | JEDEC识别通过；XIP仍pending；未写擦外部Flash |
| 24 PSRAM | 10 s | ready=1、memtest_ok=0、id_or_wrote=0、readback=0 | 初始化返回成功，但本入口内存测试未通过；不得写成PSRAM读写验证成功 |
| 29 ICACHE | 10 s | ready=1、cycles_off=1、cycles_on=1；hit_on=260029、miss_on=55、faster=0、checksum=109396 | 命中计数可见；DWT计时窗口仍无效，不能计算性能加速比 |

## 结束状态与复现

最后烧录并运行的是 **demo29**，不是实验开始前的固件；未记录实验前 Flash 备份，也未声称恢复原镜像。所有构建和烧录是本地验证，没有提交、推送或上传公众号草稿。

常规复现入口是在固件根目录执行 `tools/flash_and_capture.ps1 -Demo N -Preset Performance -Seconds S`（PowerShell）。既有脚本默认覆盖根 measured 目录下的 demo 日志，且按串口块拆行；若需保留历史，应像本轮一样另存独立目录，并连续拼接接收块。未修改该通用脚本。

本轮只核对现有实验入口及结论边界。PSRAM基础入口失败、ICACHE计时无效与ToF严格识别不足应分别继续调查；不能因为构建和烧录都成功，就把功能验收标成通过。
