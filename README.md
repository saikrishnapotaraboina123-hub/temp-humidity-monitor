# Temperature & Humidity Monitoring System

Wireless sensor node on **ESP32** + **DHT22** that continuously monitors ambient temperature and humidity and pushes readings to **ThingSpeak** over WiFi/HTTP, with threshold-based email alerting.

## Hardware
- ESP32 dev board
- DHT22 sensor

See [`docs/wiring.md`](./docs/wiring.md) for connections.

## Cloud setup
See [`docs/setup.md`](./docs/setup.md) for creating the ThingSpeak channel, API key, and the email-alert React rule.

## Flashing
1. Install the ESP32 board package and `DHT sensor library` (see `docs/setup.md`).
2. Open `src/temp_humidity_monitor.ino`.
3. Fill in `WIFI_SSID`, `WIFI_PASSWORD`, and `THINGSPEAK_API_KEY`.
4. Select **Board:** your ESP32 dev module, correct **Port**.
5. Upload, then open Serial Monitor at 115200 baud to see live readings and upload status.

## Behaviour
- Reads temperature + humidity every 20 seconds (respects both the DHT22's 2s minimum and ThingSpeak's 15s minimum update interval).
- Posts `field1` = temperature, `field2` = humidity, `field3` = alert flag (1 if temperature crosses configured high/low thresholds).
- A ThingSpeak "React" rule watches `field3` and emails you when it flips to 1 — this replaces the original MATLAB Analysis-based alert with ThingSpeak's built-in React automation (simpler to set up, same outcome).

## Tuning
| Constant | Purpose | Default |
|---|---|---|
| `TEMP_HIGH_THRESHOLD` | Upper alert bound (°C) | 38.0 |
| `TEMP_LOW_THRESHOLD` | Lower alert bound (°C) | 5.0 |
| `UPLOAD_INTERVAL_MS` | How often to read + upload | 20000 ms |
