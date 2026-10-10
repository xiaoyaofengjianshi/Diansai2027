# 2027 电赛备赛工程

引脚仍按去年核心板编写。代码编写过程中有ai参与，本readme由ai美化。
## 模块结构

| 文件 | 内容 |
|---|---|
| `bsp/selfhrtim.c/.h` | HRTIM 发波。`HRTIM_PWM_Init()` / `HRTIM_SetDuty(timer, duty)`，自带枚举校验和 0~1 限幅 |
| `bsp/selfadc.c/.h` | ADC 采样与滤波。DMA 搬运 + 滑动平均 + 阶跃检测 + k/b 标定。`ADC_GetVoltage(ch)` |
| `bsp/pid.c/.h` | PI 控制器。多实例、dt 缩放、积分限幅、输出限幅。 |
| `bsp/vofa.c/.h` | 串口打印。VOFA JustFloat 协议，中断发送，发送忙时丢帧 |
| `bsp/control.c/.h` | 控制框架：拓扑对象、调度、按键、显示、遥测 |
| `bsp/buckboost.c/.h` | buckboost开环、闭环实现 |
| `bsp/interrupt.c/.h` | 中断回调 |
| `bsp/OLED.c/.h` | OLED 显示 |
| `bsp/led.c/.h` | LED |
| `bsp/key.c/.h` | 按键（单击/双击/长按/连发） |

### 分层

```
装配层   buckboost.c      
  │                       
框架层   control.c        
  │                      
算法层   pid.c            注意buckboost为PI闭环
驱动层   selfhrtim / selfadc / vofa / OLED / key / led
```


---

## 各模块现状

### HRTIM
- 25kHz，死区 300
- 硬件配置 Master + A~F
- **A** → 开环 buck-boost
- **B** → 闭环 buck-boost
- **C~F** → 暂未使用

### ADC
- ADC1，8 通道（IN1/2/3/4/6/7/8/9），TIM7 的 TRGO 触发，**10kHz**
- DMA1_Channel1 循环搬运到 `values[8]`
- 每通道 10 点滑动平均 + 阶跃检测（跳变超 28 LSB 且连续 2 次就清空历史重灌）
- `voltage = k * 平均值 + b`，**k/b 沿用去年省赛值**，换板必须重标



### PID
- 当前为通用 **PI**：多实例、积分项按 dt 缩放、积分限幅、输出限幅

### 串口打印
- VOFA JustFloat，UART5，波特率 115200
- 每路发 2 个通道（反馈电压、实际占空比），最多发 2 路 = 4 通道

### 按键
- 状态机每 **20ms** 发送一次
- 当前约定：**KEY1/KEY2** 调第 0 路的开环占空比（±2%，长按连发）；**KEY3** 切第 0 路的开环/闭环
- KEY4 为复位键


### 时序
- TIM20 的 1ms 节拍里依次做：刷新电压 → `Key_Tick()` → `Control_Update()`
- `Update_All_Voltages()` 在第二周从主循环搬进了这个节拍（见下）




---

## 第二周完成任务

### 1. 建立拓扑模板，把控制算法和底层外设解耦

将去年的 `control.c` 现在拆成三层：

- `control.c/h` —— 框架。定义 `Topology_t`（一路变换器 = 一个 HRTIM 定时器 + 一个 ADC 通道 + 一个模式），
  以及 `Topology_Init/Update/SetMode/SetOpenDuty/SetTarget`。
  `Control_Bind(list, count)` 接收装配层交来的对象表，`Control_Update()` 遍历更新。
框架里不出现任何具体拓扑的字眼。
- `buckboost.c/h` —— 装配。只回答"这一版用哪几路、什么参数"，约 40 行。
- `pid.c` —— 不动。

加一个新拓扑（boost / buck）的成本：新建一个装配文件 + `main.c` 改一行 + Keil 加进 `bsp` 组，
**`control.c` 一行不改**。

### 2. buck-boost 开环
`topos[0]` → Timer A，`CTRL_OPEN`。KEY1/KEY2 在 5%~95% 之间调，OLED 和 VOFA 都能看到。

### 3. buck-boost 闭环
`topos[1]` → Timer B，`CTRL_CLOSED`。PI 把 ADC1_IN1（PA0）稳到 5V。
两路同时运行、互不干扰（硬件上 A/B 各接一套独立半桥）。

### 4. 占空比窗口与积分限幅
- `DUTY_MIN/DUTY_MAX = 0.05/0.95`，开环闭环共用。
  下限保自举充电和死区余量，上限因为 buck-boost 的 `Vout/Vin = D/(1-D)`
  在 D→1 时增益爆炸（D=0.95 时 dG/dD 已达 400）
- 积分限幅改成按 `Ki` 反算，改 `Ki` 时不用手调

### 5. 反馈进入 1ms 节拍
`Update_All_Voltages()` 原来和阻塞式 OLED 一起放在主循环里，刷新率被拖到几十 Hz，
控制环读到的是几十毫秒前、而且随 OLED 负载抖动的值。
搬进 TIM20 的 1ms 回调后，滑动平均窗口变成稳定可预期的 **10ms**。



### 7. 修正启动顺序
`HAL_TIM_Base_Start_IT(&htim20)` 原来在 `Control_Init()` 之前，会让未初始化的
`Topology_t`（`pwm` 字段为 0 = Timer A）被驱动。现在改为：

```c
BuckBoost_Build();                       /* 先建这一版的拓扑对象 */
Control_Init();                          /* 再启动 HRTIM */
HAL_TIM_Base_Start_IT(&htim20);          /* 最后才放开节拍 */
```

---




