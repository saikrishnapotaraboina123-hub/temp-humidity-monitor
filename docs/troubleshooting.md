# Troubleshooting

Diagnostics for the most common problems, organised by symptom. Always observe behaviour through the **Serial Monitor at 115200 baud** first — the sketch prints everything you need: WiFi status, live readings, and the ThingSpeak response.

---

## No serial output / garbled characters

**Symptoms:** nothing appears in the Serial Monitor, or text looks like `ÿÿÿÿ`.

| Check | Fix |
|---|---|
| Baud rate | Set the Serial Monitor to **115200 baud** (boards menu bottom-right). |
| Correct port | Under **Tools → Port**, select the port your ESP32 enumerates as. |
| USB driver | ESP32 dev boards need a USB-UART driver (CP210x or CH340, depending on the board). Install the one matching your board's chip. |
| Upload vs open | Open the Serial Monitor *after* the upload completes (or reconnect the board). |

---

## `WiFi connection failed - will retry in main loop.`

**Symptoms:** the sketch prints dots then `WiFi connection failed...`, and no data is ever uploaded.

| Check | Fix |
|---|---|
| SSID / password | Confirm `WIFI_SSID` and `WIFI_PASSWORD` are exactly right — check for trailing spaces and case sensitivity. |
| 2.4 GHz only | The ESP32's radio is **2.4 GHz only**. If your router is dual-band, enable the 2.4 GHz network and use its SSID. A "5G" network will never connect. |
| Guest / isolation | If you're on a guest or isolated VLAN, the ESP32 may reach the network but not the internet. Connect it to your main network. |
| Signal strength | Keep the ESP32 within range; the sketch waits only **15 s** per attempt and simply retries next loop. |
| Power supply | Some boards brown out over USB adapters. Use a decent USB cable/port; avoid laptop power-saving USB ports. |

---

## `Failed to read from DHT22 sensor. Retrying next cycle.`

**Symptoms:** the sketch prints this every 20 s; readings are `NaN`.

| Check | Fix |
|---|---|
| Wiring | Re-verify against [`wiring.md`](./wiring.md): VCC → 3V3, GND → GND, DATA → GPIO4. |
| Pull-up resistor | The bare DHT22 **needs** a 10 kΩ pull-up from DATA to 3V3. Breakout modules usually include it — check your board. |
| Power | DHT22 needs a stable ~3.3 V. Long or thin jumper wires cause brown-outs; keep wires short. |
| Errors returned | The `isnan()` check treats a failed read as a skip — the sensor may still be running but marginal. A firmware restart (`reset()` on the board) can clear a wedged sensor. |
| Wrong pin | If you moved the data wire, update `#define DHTPIN 4` at the top of the sketch to match. |

---

## `ThingSpeak update failed, HTTP code: -1` (or a suspicious "Entry ID")

**Symptoms:** reads look fine on Serial Monitor, but uploads fail — either `HTTP code: -1` or a `ThingSpeak update sent. Entry ID: <garbage>` line whose "entry ID" is not a number.

> **Why "sent" but garbage?** The sketch prints the response body whenever the server replies at all. A rejected write (e.g. HTTP `401`) still prints `ThingSpeak update sent. Entry ID: ...` followed by the error body. A **non-numeric "Entry ID" is a failure**, not a success.

| Code / reply | Meaning | Fix |
|---|---|---|
| `-1` | No connection reached (typically WiFi or DNS issue) | Re-check 2.4 GHz networking above. |
| `400` | Bad request | URL malformed — check the sketch wasn't edited wrongly. |
| `401` | Unauthorised | Wrong **write** API key, or the key was copied from the wrong channel. |
| `404` | Channel/endpoint not found | Confirm the channel exists and has **3 fields**. |
| `429` (or `Entry ID: -1`) | Too many writes / throttled | ThingSpeak free tier allows ≥ **15 s** between updates. `UPLOAD_INTERVAL_MS` is 20000 ms — don't lower it below 15000. |

After any fix, the same code path retries on the next 20 s cycle — there is nothing else to restart.

---

## No email alert when the threshold is crossed

**Symptoms:** `field3` visibly flips to `1` in ThingSpeak charts/variables, but no email arrives.

| Check | Fix |
|---|---|
| React rule exists | Create it under **Apps → React** per [`setup.md`](./setup.md). |
| Condition matches | Condition must be: channel `<your channel>`, **Field 3**, *equal to* **1**. |
| Test frequency | Set to **On Data Insertion** — the rule must fire as data arrives, not on a schedule. |
| Email verified | ThingSpeak sends React emails to the account email; confirm it's an address you check, and check spam. |
| Alert spam window | Some email providers throttle. If alerts are constant (sensor always out of range), the email may be temporarily suppressed — fix the environmental cause. |
| Wrong field | `field3` is the alert field — make sure the rule doesn't point at `field1`/`field2`. |

---

## Data looks wrong (temperatures in the hundreds, or stale)

| Symptom | Reason |
|---|---|
| Temperature off by ~40 °C | Unit confusion: the sketch always reports **Celsius** (`readTemperature()`). Compare against a known-accuracy thermometer. |
| Humidity stuck / slowly changing | DHT22 responds slowly to humidity changes by design; allow 10+ minutes in a new environment. |
| Old data after reboot | There is no local storage — ThingSpeak only retains what was uploaded. Entries are timestamped server-side when written. |

---

## Still stuck?

1. Re-run through the checklists above with the Serial Monitor open.
2. Confirm the three ingredients independently:
   - WiFi works (see `Connected. IP: ...`)
   - Sensor works (sensible readings printed)
   - ThingSpeak accepts writes (entry ID returned)
3. Open an issue on the repository with the Serial Monitor output pasted, plus your board model and sensor type (bare DHT22 vs breakout).