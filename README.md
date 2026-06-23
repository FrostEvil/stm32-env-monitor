# EnvSentinel — STM32 Environment Monitor

Embedded environmental monitoring system built on STM32 Nucleo F401RE.

The device periodically measures temperature, humidity, and atmospheric pressure using a BME280 sensor, evaluates measurements against configurable thresholds with hysteresis, and reacts through LEDs, a buzzer, and an OLED display. Measurements and configuration commands are exchanged with a PC over UART.

---

## Project Highlights

* STM32F401RE microcontroller
* BME280 environmental sensor (I2C)
* SSD1306 OLED display (I2C)
* Interrupt-driven UART RX/TX
* Ring buffer implementation
* Function pointer command dispatch table
* Alarm state machine with hysteresis
* Runtime configuration via UART
* Watchdog protection
* Modular software architecture

---

## Hardware

| Component           | Description                                   |
| ------------------- | --------------------------------------------- |
| STM32 Nucleo F401RE | Main microcontroller board                    |
| BME280              | Temperature, humidity and pressure sensor     |
| SSD1306 OLED        | 128×64 display                                |
| Active buzzer       | Audible alarm                                 |
| NPN transistor      | Buzzer driver                                 |
| 3× LEDs             | Green (NORMAL), Yellow (WARNING), Red (ERROR) |
| 4× Push buttons     | User interaction                              |
| Resistors           | LED current limiting and button pull-downs    |

---

## System States

```text
NORMAL
   ↓
WARNING
   ↓
ERROR
```

State transitions use configurable hysteresis to prevent alarm chattering near threshold boundaries.

---

## Software Architecture

```text
┌─────────────────────────────────────────────┐
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
│    BME280 (0x76)     │    SSD1306 (0x3C)    │
└──────────────────────┴──────────────────────┘
```

The application layer is independent of hardware implementation details. All sensor and display operations are isolated inside dedicated driver modules.

### Driver Layer

#### bme280

* Sensor initialization
* Forced Mode triggering
* Calibration handling
* Measurement compensation

#### ssd1306

* Display initialization
* Framebuffer management
* Partial screen updates

### Application Layer

#### alarm

* Threshold evaluation
* Hysteresis handling
* Alarm state management
* LED and buzzer control

#### display_manager

* Measurement rendering
* Status visualization
* Configuration feedback screens

#### uart_protocol

* UART RX ring buffer
* Command parsing
* Function pointer dispatch table
* ACK / ERR responses

#### button

* Timer-based debouncing
* Edge detection

---

## Communication

### I2C1

Shared bus for:

* BME280 (0x76)
* SSD1306 (0x3C)

Bus speed: **100 kHz**

### USART2

Configuration:

* 115200 baud
* 8 data bits
* No parity
* 1 stop bit
* Full duplex

TX uses interrupt-driven transmission with a busy flag preventing overlapping transfers.

RX uses interrupt-driven reception combined with a ring buffer to prevent data loss.

---

## Features

* Periodic measurements using BME280 Forced Mode
* Configurable measurement interval
* Independent WARNING and ERROR thresholds
* Configurable alarm hysteresis
* OLED status display
* Runtime configuration through UART
* UART measurement streaming
* Alarm indication using LEDs and buzzer
* Auto-scrolling settings pages
* Button-controlled user interface

---

## UART Data Format

Example measurement frame:

```text
T:23.5;H:58.2;P:1013.1;UPT_MS:45230
```

Where:

* T = temperature [°C]
* H = humidity [%RH]
* P = pressure [hPa]
* UPT_MS = uptime returned by HAL_GetTick()

---

## UART Protocol Example

```text
PC      -> CMD:SET_INTERVAL:10
STM32   -> ACK

PC      -> CMD:SET_HYSTERESIS:2
STM32   -> ACK

PC      -> CMD:SET_HYSTERESIS:100
STM32   -> ERR
```

---

## UART Commands

General format:

```text
CMD:PARAMETER:VALUE
```

Responses:

```text
ACK
```

or

```text
ERR
```

### Threshold Configuration

#### Temperature

* CMD:SET_TEMPERATURE_MIN_ERROR:X
* CMD:SET_TEMPERATURE_MAX_ERROR:X
* CMD:SET_TEMPERATURE_MIN_WARNING:X
* CMD:SET_TEMPERATURE_MAX_WARNING:X

#### Pressure

* CMD:SET_PRESSURE_MIN_ERROR:X
* CMD:SET_PRESSURE_MAX_ERROR:X
* CMD:SET_PRESSURE_MIN_WARNING:X
* CMD:SET_PRESSURE_MAX_WARNING:X

#### Humidity

* CMD:SET_HUMIDITY_MIN_ERROR:X
* CMD:SET_HUMIDITY_MAX_ERROR:X
* CMD:SET_HUMIDITY_MIN_WARNING:X
* CMD:SET_HUMIDITY_MAX_WARNING:X

### System Configuration

* CMD:SET_INTERVAL:X
* CMD:SET_OSRS:X
* CMD:SET_HYSTERESIS:X
* CMD:SET_CONTRAST:X
* CMD:SET_INVERSE_DISPLAY:ON
* CMD:SET_INVERSE_DISPLAY:OFF

---

## Error Handling

### Initialization Failure

* Up to 5 retry attempts
* 500 ms delay between retries
* System reset after unsuccessful recovery

### Measurement Trigger Failure

* Retry on subsequent measurement cycles
* Failure indication via status LED
* System reset after repeated failures

### Measurement Read Failure

* Consecutive failure counter
* Successful measurement resets the counter
* System reset after configured failure limit

### Independent Watchdog (IWDG)

* Hardware-level protection against software lockups
* Automatic recovery through system reset

---

## What I Learned

This project was my first attempt at building a complete embedded application rather than a standalone peripheral driver.

Key topics explored during development:

* Modular software design
* Interrupt-driven UART communication
* Ring buffer implementation
* Function pointer dispatch tables
* Alarm state machines
* Hysteresis implementation
* Timer-based debouncing
* Watchdog configuration
* Embedded fault handling
* STM32CubeIDE debugging techniques
* I2C peripheral integration

On the hardware side I learned how to:

* Drive external loads using a transistor stage
* Select resistor values for LEDs
* Use pull-down resistors on GPIO inputs
* Integrate multiple devices on a shared I2C bus
* Debug mixed hardware/software issues

---

## Future Improvements

### RTC Integration

Add battery-backed RTC support to timestamp measurements with real date and time.

### Python Data Logger

Create a PC application that:

* Logs measurements to CSV
* Calculates averages
* Generates charts using matplotlib

### Extended OLED UI

Allow threshold configuration directly from the device without requiring UART commands.

### Custom PCB

Design a dedicated PCB integrating all system components into a compact standalone device.

---

