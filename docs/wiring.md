# Wiring — Temperature & Humidity Monitoring System

## Components

- ESP32 dev board (e.g. ESP32-WROOM-32 DevKit)
- DHT22 (AM2302) temperature/humidity sensor
- 10 kΩ resistor (pull-up on the data line — **many breakout modules already include this**)
- Breadboard + jumper wires (handy for prototypes)

> **Bare sensor vs breakout module:** the bare DHT22 needs the 10 kΩ pull-up; breakout modules (common green/blue PCBs) usually have the resistor, an LED, and a decoupling capacitor already mounted. If you use a breakout you typically need **no extra resistor**.

## Connections

| DHT22 pin | Connects to ESP32 | Notes |
|---|---|---|
| `VCC` | `3V3` | DHT22 is rated 3.3–5.5 V; 3.3 V keeps the logic level safe for the ESP32 |
| `DATA` | `GPIO4` | Add a **10 kΩ pull-up to `3V3`** if using the bare sensor |
| `GND` | `GND` | Common ground is required |

> The data pin is fixed in the sketch as `#define DHTPIN 4`. If you move it to another GPIO, update that line at the top of `src/temp_humidity_monitor.ino`.

## Diagram

```
        ESP32                DHT22
      ┌───────┐            ┌───────┐
      │   3V3 ├────────────┤ VCC   │
      │       │   ┌──10kΩ──┤ DATA  │
      │  GPIO4 ├───┘   │    │       │
      │       │        └──3V3 (pull-up)
      │   GND ├────────────┤ GND   │
      └───────┘            └───────┘
```

The pull-up connects `DATA` to `3V3` through a 10 kΩ resistor — it goes *between* the two nodes, not in series on the data line.

## Next step

Wire the board, then follow [`setup.md`](./setup.md) to flash the sketch and configure ThingSpeak.

## Notes

- **Read interval:** the DHT22 needs at least **2 seconds** between reads. The sketch's 20 s upload interval respects this comfortably.
- **ThingSpeak rate limit:** the free tier enforces a minimum **15-second** update interval per channel.
- **Keep wires short:** the DHT22's 1-wire protocol is timing-sensitive; long, thin jumpers can cause `NaN` reads. Aim for < 20 cm (8 in) if possible.
- **Verifying a good connection:** after flashing, the Serial Monitor should show realistic `Temp:` / `Humidity:` values — see [`troubleshooting.md`](./troubleshooting.md) if it prints `Failed to read from DHT22 sensor`.