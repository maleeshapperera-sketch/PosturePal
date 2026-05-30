# 🪑 PosturePal

> A wearable posture monitoring device built on the ESP32 and MPU9250 IMU. PosturePal detects slouching in real time, alerts you via buzzer, and publishes posture events over MQTT so you can track your sitting habits from anywhere.

![ESP32](https://img.shields.io/badge/ESP32-Microcontroller-purple)
![MPU9250](https://img.shields.io/badge/MPU9250-IMU-teal)
![MQTT](https://img.shields.io/badge/MQTT-HiveMQ%20Cloud-blue)
![Arduino](https://img.shields.io/badge/Platform-Arduino-green)

---

## Features

- **Real-time detection** — Reads tilt angles every second via MPU9250
- **Buzzer alerts** — Rings when a slouch is held for 5+ seconds
- **MQTT publishing** — Sends posture events to HiveMQ Cloud over secure TLS
- **Idle detection** — Alerts after 1 minute of no movement detected
- **Posture verification** — Confirms correction is held for 3 seconds before resetting
- **WiFiManager** — Configure WiFi via captive portal on first boot, no hardcoding needed

---

## Hardware Requirements

| Component       | Details                        |
|-----------------|--------------------------------|
| Microcontroller | ESP32 (any variant)            |
| IMU             | MPU9250 (I²C)                  |
| Alert output    | Buzzer or LED on GPIO 4        |
| I²C pins        | SDA → GPIO 21, SCL → GPIO 22  |

---

## Wiring

```
MPU9250    →   ESP32
VCC        →   3.3V
GND        →   GND
SDA        →   GPIO 21
SCL        →   GPIO 22

Buzzer (+) →   GPIO 4
Buzzer (-) →   GND
```

---

## Dependencies

Install the following via the Arduino Library Manager:

- `MPU9250_WE` by Wolfgang Ewald
- `PubSubClient` by Nick O'Leary
- `ArduinoJson` by Benoit Blanchon
- `WiFiManager` by tzapu

---

## Configuration

Update the MQTT credentials at the top of the sketch:

```cpp
const char* brokerAddress  = "YOUR_HIVEMQ_HOST";
const int   brokerPort     = 8883;
const char* brokerUser     = "YOUR_USERNAME";
const char* brokerPass     = "YOUR_PASSWORD";
const char* mqttTopic      = "alert/posture";
```

> **WiFi setup:** On first boot, the device creates a hotspot called **PostureMonitorAP**. Connect to it from your phone or laptop, enter your WiFi credentials via the captive portal, and the device will connect automatically — no recompilation needed.

---

## MQTT Events

All messages are published to the configured topic as JSON:

```json
{ "message": "Posture Alert! Incorrect Posture Detected", "angle": 65.3 }
```

| Message                                    | Trigger                        |
|--------------------------------------------|--------------------------------|
| `Posture Alert! Incorrect Posture Detected`| Slouch held for 5 s            |
| `Posture Alert! Continued Incorrect Posture`| Slouch held for 20 s          |
| `Posture Corrected`                        | Good posture held for 3 s      |
| `Idle for too long`                        | No movement detected for 1 min |

---

## Posture Threshold

A Z-axis tilt angle below **70°** is treated as a slouch. Adjust this value in `evaluatePosture()` to suit your sensor mounting position and preference.

---

## How It Works

1. On boot, the MPU9250 auto-calibrates — keep the sensor flat during this step.
2. Every second, the device reads the Z-axis tilt angle.
3. If the angle drops below 70° for more than 5 seconds, the buzzer rings and an MQTT alert is sent.
4. If the bad posture continues past 20 seconds, a follow-up alert is sent.
5. When good posture is resumed and held for 3 seconds, a correction event is published.
6. If no movement is detected for 1 minute, an idle alert is sent.

---

## License

MIT — free to use, modify, and distribute.
