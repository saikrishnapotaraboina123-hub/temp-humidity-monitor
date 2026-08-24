# ThingSpeak Setup

1. Create a free account at https://thingspeak.com
2. **New Channel** with 3 fields:
   - Field 1: Temperature (°C)
   - Field 2: Humidity (%)
   - Field 3: Alert Flag (0/1)
3. Under **API Keys** tab, copy the **Write API Key** into `THINGSPEAK_API_KEY` in the sketch.
4. Under the channel's **Visualizations**, the default line charts will auto-update every time the ESP32 posts data.

## Threshold email alert (MATLAB Analysis / React)

1. Go to **Apps → React** in ThingSpeak.
2. Create a new React:
   - **Condition Type:** Numeric
   - **Test Frequency:** On Data Insertion
   - **Condition:** `Channel <your channel> Field 3` is `equal to` `1`
   - **Action:** Send Email → enter your address, subject "Temperature Alert"
3. Save. Now, whenever the ESP32 detects `temperature >= TEMP_HIGH_THRESHOLD` or `<= TEMP_LOW_THRESHOLD`, it posts `field3 = 1`, which triggers the React and sends you an email — matching the "threshold-based email alert" behaviour using ThingSpeak's analysis/automation tools.

## Library installation

In Arduino IDE: **Sketch → Include Library → Manage Libraries**, install:
- `DHT sensor library` (by Adafruit)
- `Adafruit Unified Sensor` (dependency, installs automatically)

Board package: **esp32 by Espressif Systems** (Boards Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)
