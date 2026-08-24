# Wiring — Temperature & Humidity Monitoring System

## Components
- ESP32 dev board (e.g. ESP32-WROOM-32 DevKit)
- DHT22 (AM2302) temperature/humidity sensor
- 10kΩ resistor (pull-up on data line — many DHT22 breakout modules already include this)
- Breadboard + jumper wires

## Connections

| DHT22 pin | Connects to ESP32 |
|-----------|--------------------|
| VCC       | 3V3                |
| DATA      | GPIO4 (+10kΩ pull-up to 3V3 if using the bare sensor, not a breakout) |
| GND       | GND                |

```
        ESP32                DHT22
      ┌───────┐            ┌───────┐
      │   3V3 ├────────────┤ VCC   │
      │       │      ┌─────┤ DATA  │
      │  GPIO4├──────┘     │       │
      │       │  10kΩ pull-up to 3V3 (if needed)
      │   GND ├────────────┤ GND   │
      └───────┘            └───────┘
```

## Notes
- DHT22 needs at least 2 seconds between reads — the sketch's 20s upload interval already respects this comfortably.
- ThingSpeak's free tier enforces a minimum 15-second update interval per channel.
