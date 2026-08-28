# Temperature & Humidity Monitoring System

A low-cost wireless environmental sensor node built on the **ESP32** microcontroller and **DHT22** sensor. It continuously samples ambient temperature and humidity and pushes the readings to **ThingSpeak** over WiFi (HTTP), where charts are rendered automatically. A built-in threshold check sets an alert flag whenever the temperature leaves a configurable range, which triggers an **email alert** through ThingSpeak's **React** automation.

```
 ESP32 ──DHT22──▶ reads temp + humidity every 20s
   │
   │  HTTP GET /update (field1=temperature, field2=humidity, field3=alert)
   ▼
 ThingSpeak ──▶ charts + stored data
   │
   │  React rule watches field3 == 1
   ▼
 Email alert to you
```

## Features

- **Continuous monitoring** — samples every 20 s (comfortably above both the DHT22's 2 s minimum read interval and ThingSpeak's 15 s write minimum).
- **Cloud logging** — every reading is stored on ThingSpeak with automatic time-series charts.
- **No cloud code** — uses ThingSpeak's plain HTTP write API; no MQTT broker or external server required.
- **Threshold-based email alerts** — a ThingSpeak **React rule** emails you when the temperature crosses the configured high/low bounds.
- **Self-healing WiFi** — reconnects automatically if the connection drops and retries failed uploads on the next cycle.
- **Battery-friendly timing** — low duty cycle (one short HTTP request every 20 s).

## Hardware

| Component | Notes |
|---|---|
| ESP32 dev board | e.g. ESP32-WROOM-32 DevKit, any compatible development board |
| DHT22 (AM2302) sensor | Digital temperature + humidity sensor |
| 10 kΩ resistor | Pull-up on the data line (many breakout modules include this) |
| Breadboard + jumper wires | Optional, handy for prototypes |

See [`docs/wiring.md`](./docs/wiring.md) for the full wiring reference and connection table.

## Cloud setup (ThingSpeak + email alerts)

Follow [`docs/setup.md`](./docs/setup.md) to:

1. Create a free ThingSpeak account and channel.
2. Create the three channel fields and copy your **Write API Key**.
3. Install the ESP32 board package and the `DHT sensor library` in the Arduino IDE.
4. Configure the **React** rule that sends the alert email.

## Flashing (quick start)

1. Install the ESP32 board package and `DHT sensor library` — see [`docs/setup.md`](./docs/setup.md).
2. Open [`src/temp_humidity_monitor.ino`](./src/temp_humidity_monitor.ino) in the Arduino IDE.
3. Fill in `WIFI_SSID`, `WIFI_PASSWORD` and `THINGSPEAK_API_KEY`.
4. Select **Board:** your ESP32 dev module and the correct **Port**.
5. Upload, then open the **Serial Monitor** at **115200 baud** to watch live readings and upload status.

## Behaviour

- Reads temperature (°C) and relative humidity (%) every 20 s.
- Publishes:
  - `field1` = temperature (°C)
  - `field2` = humidity (%)
  - `field3` = alert flag — `1` when temperature is `>= TEMP_HIGH_THRESHOLD` or `<= TEMP_LOW_THRESHOLD`, otherwise `0`.
- A ThingSpeak **React rule** watches `field3` and emails you the moment it flips to `1`. This replaces the original MATLAB Analysis-based approach with ThingSpeak's built-in React automation — simpler to set up, same outcome.

## Configuration

Edit the `USER CONFIG` block at the top of the sketch:

| Constant | Purpose | Default |
|---|---|---|
| `WIFI_SSID` | Your 2.4 GHz WiFi network name | — |
| `WIFI_PASSWORD` | Your WiFi password | — |
| `THINGSPEAK_API_KEY` | Channel **Write API Key** (not the Read key) | — |
| `THINGSPEAK_HOST` | ThingSpeak HTTP update endpoint | `http://api.thingspeak.com/update` |
| `TEMP_HIGH_THRESHOLD` | Upper alert bound (°C) | `38.0` |
| `TEMP_LOW_THRESHOLD` | Lower alert bound (°C) | `5.0` |
| `UPLOAD_INTERVAL_MS` | Read + upload interval | `20000` ms |
| `DHTPIN` | GPIO connected to the DHT22 data line | `4` |

> ESP32's built-in radio supports only the **2.4 GHz** band — a dual-band router must have a 2.4 GHz network enabled, and the SSID/password you fill in must be for that network.

## Project structure

```
temp-humidity-monitor/
├── README.md                      # this file
├── docs/
│   ├── overview.md                # architecture, data flow and code walkthrough
│   ├── setup.md                   # ThingSpeak + Arduino IDE setup
│   ├── wiring.md                  # hardware connections
│   └── troubleshooting.md         # common problems and fixes
└── src/
    └── temp_humidity_monitor.ino  # ESP32 firmware (Arduino sketch)
```

## Documentation index

| Document | What it covers |
|---|---|
| [`docs/overview.md`](./docs/overview.md) | System architecture, data flow, and a tour of the sketch code |
| [`docs/setup.md`](./docs/setup.md) | Arduino IDE, board + library installation, ThingSpeak channel and React email rule |
| [`docs/wiring.md`](./docs/wiring.md) | Component list, pin connections, and wiring diagram |
| [`docs/troubleshooting.md`](./docs/troubleshooting.md) | Diagnostics for WiFi, sensor, and ThingSpeak issues |