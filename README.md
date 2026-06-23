EnvSentinel — STM32 Environment Monitor
Embedded environmental monitoring system built on STM32 Nucleo F401RE.
The device periodically measures temperature, humidity, and atmospheric pressure using a BME280 sensor, evaluates measurements against configurable thresholds with hysteresis, and reacts through LEDs, a buzzer, and an OLED display. Measurements and configuration commands are exchanged with a PC over UART.

Project Highlights

STM32F401RE microcontroller
BME280 environmental sensor (I2C)
SSD1306 OLED display (I2C)
Interrupt-driven UART RX/TX
Ring buffer implementation
Function pointer command dispatch table
Alarm state machine with hysteresis
Runtime configuration via UART
Watchdog protection and fault recovery
Modular software architecture


Hardware
ComponentDescriptionSTM32 Nucleo F401REMain microcontroller boardBME280Temperature, humidity and pressure sensorSSD1306 OLED128×64 displayActive buzzerAudible alarmNPN transistorBuzzer driver3× LEDsNORMAL / WARNING / ERROR indication4× Push buttonsUser interactionResistorsLED current limiting and button pull-downs

System States
The system operates in three alarm states: NORMAL, WARNING, and ERROR. State transitions use configurable hysteresis to prevent alarm chattering near threshold boundaries — the alarm activates when a threshold is crossed but deactivates only after the measurement returns past the hysteresis margin.

Software Architecture
text┌─────────────────────────────────────────────┐
│              Application Layer              │
│                                             │
│  alarm.c         display_manager.c          │
│  button.c        uart_protocol.c            │
│  main.c                                     │
├─────────────────────────────────────────────┤
│                Driver Layer                 │
│                                             │
│       bme280.c         ssd1306.c            │
├──────────────────────┬──────────────────────┤
│       Hardware       │       Hardware       │
│   BME280 (0x76)      │   SSD1306 (0x3C)     │
└──────────────────────┴──────────────────────┘
The application layer is independent of hardware implementation details. All sensor and display operations are isolated inside dedicated driver modules.
Driver Layer
bme280

sensor initialization
Forced Mode triggering
calibration handling
measurement compensation

ssd1306

display initialization
framebuffer management
partial screen updates

Application Layer
alarm

threshold evaluation
hysteresis handling
alarm state management
LED and buzzer control

display_manager

measurement rendering
status visualization
configuration feedback screens

uart_protocol

UART RX ring buffer
command parsing
function pointer dispatch table
ACK / ERR responses

button

timer-based debouncing
edge detection


Communication
I2C1
Shared bus for BME280 (0x76) and SSD1306 (0x3C) at 100 kHz.
USART2
115200 baud, 8N1, full duplex. TX uses interrupt-driven transmission with a busy flag preventing overlapping transfers. RX uses interrupt-driven reception combined with a ring buffer to prevent data loss.

Features

Periodic measurements using BME280 Forced Mode
Configurable measurement interval
Independent WARNING and ERROR thresholds per parameter
Configurable alarm hysteresis
OLED status display with ERROR value blinking
Runtime configuration through UART
UART measurement streaming to PC
Alarm indication using LEDs and buzzer
Auto-scrolling settings pages on OLED
Button-controlled user interface


UART Data Format
textT:23.5;H:58.2;P:1013.1;UPT_MS:45230

T = temperature [°C]
H = humidity [%RH]
P = pressure [hPa]
UPT_MS = uptime from HAL_GetTick()


UART Protocol Example
textPC      -> CMD:SET_INTERVAL:10
STM32   -> ACK

PC      -> CMD:SET_HYSTERESIS:2
STM32   -> ACK

PC      -> CMD:SET_HYSTERESIS:100
STM32   -> ERR

UART Commands
General format: CMD:PARAMETER:VALUE
Responses: ACK or ERR
Threshold Configuration
Temperature:

CMD:SET_TEMPERATURE_MIN_ERROR:X
CMD:SET_TEMPERATURE_MAX_ERROR:X
CMD:SET_TEMPERATURE_MIN_WARNING:X
CMD:SET_TEMPERATURE_MAX_WARNING:X

Pressure:

CMD:SET_PRESSURE_MIN_ERROR:X
CMD:SET_PRESSURE_MAX_ERROR:X
CMD:SET_PRESSURE_MIN_WARNING:X
CMD:SET_PRESSURE_MAX_WARNING:X

Humidity:

CMD:SET_HUMIDITY_MIN_ERROR:X
CMD:SET_HUMIDITY_MAX_ERROR:X
CMD:SET_HUMIDITY_MIN_WARNING:X
CMD:SET_HUMIDITY_MAX_WARNING:X

System Configuration

CMD:SET_INTERVAL:X — measurement interval in seconds (min: 2)
CMD:SET_OSRS:X — BME280 oversampling (0/1/2/4/8/16)
CMD:SET_HYSTERESIS:X — alarm hysteresis in units (0–5)
CMD:SET_CONTRAST:X — OLED contrast (0–100)
CMD:SET_INVERSE_DISPLAY:ON|OFF — invert OLED display colors


Error Handling
Initialization failure — up to 5 retry attempts with 500ms delay. After 5 failures: red LED blinks for 3 seconds, then system reset.
Measurement trigger failure — retry every 5 seconds with LD2 blinking. After 5 failed retries: system reset.
Measurement read failure — consecutive failure counter incremented. On recovery: UART notification with failure count. After 10 consecutive failures: system reset.
Watchdog (IWDG) — 5-second hardware timeout, refreshed every main loop iteration as last-resort protection against software lockups.

What I Learned
This project was my first attempt at building a complete embedded system rather than a standalone peripheral driver. The biggest shift was learning to think in modules — separating what each piece of code is responsible for and making sure they don't bleed into each other. Getting alarm.c to know nothing about the OLED, and ssd1306.c to know nothing about alarms, took more discipline than I expected.
I learned interrupt-driven UART both ways — TX with a busy flag to avoid race conditions, and RX with a ring buffer to avoid losing bytes between processing cycles. Writing the command parser with a function pointer lookup table was one of the more satisfying parts — it replaced what would have been a giant switch/case with something that actually scales.
Timer-based debouncing, volatile on shared variables, and watchdog configuration were things I had read about but never implemented myself. Debugging with STM32CubeIDE — breakpoints, the Expressions window, checking whether interrupts actually fire — became a daily habit rather than a last resort.
On the hardware side I learned to drive a buzzer through a transistor, calculate resistor values for LEDs, use pull-down resistors on GPIO inputs, and integrate multiple I2C devices on a shared bus.

Future Improvements
RTC Integration — add battery-backed RTC to timestamp measurements with real date and time instead of uptime milliseconds.
Python Data Logger — PC-side script using pyserial to log measurements to CSV, calculate periodic averages, and generate matplotlib charts.
Extended OLED UI — allow threshold configuration directly on the device without requiring a PC connection.
Custom PCB — design a dedicated board in KiCad integrating all components into a compact standalone device.

License
MIT License
