# Setup Guide

Two independent sides need to be prepared before the system runs: the **hardware toolchain** (Arduino IDE + ESP32 board support + DHT22 library) to flash the sketch, and the **cloud side** (ThingSpeak channel + React email rule) to receive the data.

---

## 1. Prerequisites

- [Arduino IDE 2.x](https://www.arduino.cc/en/software) (or 1.8.x)
- An ESP32 dev board and a DHT22 sensor (see [`wiring.md`](./wiring.md))
- A WiFi network (2.4 GHz — see notes)
- A free [ThingSpeak](https://thingspeak.com) account

---

## 2. Install the ESP32 board package

1. Open **File → Preferences** (Arduino IDE 2: **Settings** tab).
2. In **Additional Boards Manager URLs**, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   (separate multiple URLs with commas)
3. Open **Tools → Board → Boards Manager**, search for **esp32**, and install **ESP32 by Espressif Systems**.
4. After install, connect the ESP32 and select **Tools → Board → ESP32 Arduino →** your dev module (e.g. `ESP32 Dev Module`), then pick the **Port**.

> **Note:** the ESP32 WiFi radio is **2.4 GHz only**. If your router is dual-band, make sure a 2.4 GHz SSID is available and use that one in the sketch.

---

## 3. Install the DHT sensor library

1. Open **Sketch → Include Library → Manage Libraries** (Arduino IDE 2: the library manager icon on the left).
2. Search for and install:
   - **DHT sensor library** (by Adafruit)
   - The library pulls in **Adafruit Unified Sensor** as a dependency (installs automatically).

---

## 4. Create the ThingSpeak channel

1. Sign in at [thingspeak.com](https://thingspeak.com) → **Channels → My Channels → New Channel**.
2. Name the channel (e.g. `Temperature Humidity Monitor`) and define **exactly three fields**:
   - **Field 1:** `Temperature (°C)`
   - **Field 2:** `Humidity (%)`
   - **Field 3:** `Alert Flag (0/1)`
3. Click **Save Channel**.
4. Open the **API Keys** tab and copy the **Write API Key** — this is what the sketch uses.

> If the channel already exists with fewer fields, edit the channel and add missing fields. Without all three fields the corresponding HTTP parameters are ignored silently.

---

## 5. Configure and flash the sketch

1. Open [`src/temp_humidity_monitor.ino`](../src/temp_humidity_monitor.ino) in the Arduino IDE.
2. In the `USER CONFIG` block at the top, set:
   - `WIFI_SSID` — your 2.4 GHz network name
   - `WIFI_PASSWORD`
   - `THINGSPEAK_API_KEY` — the **Write** API key from step 4 (not the Read key)
3. Optionally adjust `TEMP_HIGH_THRESHOLD` / `TEMP_LOW_THRESHOLD` (°C) and `UPLOAD_INTERVAL_MS`.
4. Select your board model and port (step 2.4), then click **Upload**.
5. Open **Serial Monitor** at **115200 baud**. You should see:
   ```
   Connecting to WiFi...
   Connected. IP: 192.168.1.23
   Temp: 24.50 C  |  Humidity: 55.20 %
   ThingSpeak update sent. Entry ID: 12345678
   ```
6. Back in ThingSpeak, open your channel — the three charts start filling with every 20 s sample.

---

## 6. Email alerts (ThingSpeak React rule)

The ESP32 writes `field3 = 1` whenever the temperature is above `TEMP_HIGH_THRESHOLD` or below `TEMP_LOW_THRESHOLD`. A **React** app turns that into an email.

1. Go to **Apps → React** (in ThingSpeak).
2. Click **New React** and configure:
   - **React Name:** `Temperature alert`
   - **Condition Type:** `Numeric`
   - **Test Frequency:** `On Data Insertion`
   - **Condition:** Channel → your channel, Field → **Field 3**, operator → **equal to**, value → **1**
   - **Action:** `Send Email` — destination address, subject (e.g. `Temperature Alert`), and message.
3. Click **Save React**.
4. Test: either wait for a real threshold breach, or temporarily lower `TEMP_HIGH_THRESHOLD` to e.g. `23.0`, upload, and confirm the email arrives. Revert afterwards.

> This replaces the older MATLAB Analysis-based alert: the same outcome (mail on threshold breach) with no MATLAB code to maintain. MATLAB Analysis remains available if you later want custom cloud-side processing, e.g. storing `field3` pulses as distinct events.

---

## Verification checklist

- [ ] ESP32 shows `Connected. IP: ...` in the Serial Monitor
- [ ] Live readings print every ~20 s (temperature + humidity)
- [ ] ThingSpeak channel charts update and the channel shows `Last entry`
- [ ] Manually breaching a threshold sets `field3 = 1` and fires the email