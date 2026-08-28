# System Overview & Architecture

This document explains how the whole system fits together, the data flow end-to-end, and how the firmware works on the inside.

## 1. What the system does

The system is a self-contained monitoring node:

- An **ESP32** reads temperature and humidity from a **DHT22** sensor.
- Every **20 seconds** it uploads the readings to **ThingSpeak** over WiFi using ThingSpeak's HTTP write API.
- When the measured temperature falls outside a configurable range, the ESP32 sets an **alert flag** that ThingSpeak watches. A **React** automation then sends you an **email alert**.

There is no separate server, database, or hub in the middle — the ESP32 talks directly to the ThingSpeak cloud.

## 2. Block diagram

```
                 ┌─────────────────────┐
                 │      DHT22 sensor   │
                 │  (temperature/rel.) │
                 └──────────┬──────────┘
                            │ 1-wire data on GPIO4 (10 kΩ pull-up to 3V3)
                 ┌──────────▼──────────┐
                 │        ESP32        │
                 │  - reads sensor     │
                 │  - checks thresholds│
                 │  - builds HTTP GET  │
                 └──────────┬──────────┘
                            │ WiFi (2.4 GHz)
                 ┌──────────▼──────────┐
                 │     ThingSpeak      │
                 │  channel stores     │
                 │  field1/field2/     │
                 │  field3 + charts    │
                 └──────────┬──────────┘
                            │ React rule: field3 == 1
                 ┌──────────▼──────────┐
                 │   Email alert sent  │
                 │   to your address   │
                 └─────────────────────┘
```

## 3. Data flow, step by step

| # | Step | Where | What happens |
|---|---|---|---|
| 1 | Sample | ESP32 + DHT22 | `loop()` fires every `UPLOAD_INTERVAL_MS` (20 s) and calls `dht.readHumidity()` / `dht.readTemperature()`. |
| 2 | Validate | ESP32 | If either reading is `NaN`, the DHT22 failed — the cycle is aborted and retried next loop. |
| 3 | Decide | ESP32 | Temperature is compared against `TEMP_HIGH_THRESHOLD` (default 38.0 °C) and `TEMP_LOW_THRESHOLD` (default 5.0 °C). `alertFlag` becomes `1` when out of range, else `0`. |
| 4 | Transmit | ESP32 → ThingSpeak | `sendToThingSpeak()` issues an HTTP GET to `http://api.thingspeak.com/update?api_key=...&field1=...&field2=...&field3=...`. ThingSpeak stores one channel entry. |
| 5 | Chart | ThingSpeak | The channel's visualizations plot the three fields automatically. |
| 6 | Alert | ThingSpeak React → Email | A React rule triggers "on data insertion" whenever `field3` equals `1`, firing a notification email. |

> **Why 20 s?** The DHT22 requires ≥ 2 s between reads, and ThingSpeak's free tier enforces ≥ 15 s between writes. A 20 s interval safely satisfies both — and it also keeps a 24-h day at 4 320 samples per field, far inside free-tier limits.

## 4. The update request

The sketch builds a GET request on the ThingSpeak update endpoint:

```
http://api.thingspeak.com/update
  ?api_key = <WRITE_API_KEY>
  &field1  = <temperature, e.g. 24.50>
  &field2  = <humidity,    e.g. 55.20>
  &field3  = <alert flag,  0 or 1>
```

On success ThingSpeak returns the **entry ID** of the stored update (a number), which the sketch prints to the Serial Monitor. ThingSpeak replies to every write (even rejected ones), so watch for a **non-numeric "Entry ID"** — that means the write was rejected rather than accepted.

### Channel fields

| Field | Content | Values |
|---|---|---|
| `field1` | Temperature | °C, 2 decimal places |
| `field2` | Humidity | relative %, 2 decimal places |
| `field3` | Alert flag | `0` = in range, `1` = threshold breached |

## 5. Alerting model

The alert is **edge-triggered at the source**: the ESP32 decides whether the temperature is out of range and encodes that decision in `field3`. ThingSpeak doesn't do any math — it just reacts to the value.

Why this design?

- **Simple** — no MATLAB Analysis app or condition logic in the cloud.
- **Robust** — the same `field3` value would also work with a ThingSpeak **TalkBack** or **TimeControl** resource if you later want hardware responses, not just emails.
- **Visible** — every reading is stored, so you always get a permanent record of alerts alongside the raw data.

## 6. Code walkthrough

Everything lives in [`src/temp_humidity_monitor.ino`](../src/temp_humidity_monitor.ino).

### Global configuration (`USER CONFIG` block)

The only things you should normally edit: WiFi credentials, the ThingSpeak write API key, thresholds, and the upload interval. Everything else is board wiring.

### `setup()`

- Opens the Serial Monitor at 115200 baud.
- Initialises the DHT22.
- Connects to WiFi.

### `loop()` (the main cycle)

1. If WiFi dropped, reconnect.
2. If `UPLOAD_INTERVAL_MS` has elapsed, take a measurement:
   - Read temperature + humidity.
   - Bail out silently if the sensor returns `NaN`.
   - Compute the alert flag from the thresholds.
   - Print a human-readable line, e.g. `Temp: 24.50 C  |  Humidity: 55.20 %`.
   - Call `sendToThingSpeak()`.

### `connectWiFi()`

- Sets station mode, calls `WiFi.begin(SSID, PASS)`.
- Waits up to **15 s** for a connection, printing dots.
- Prints the assigned IP on success; on failure simply returns — `loop()` retries next cycle. Nothing is uploaded while disconnected.

### `sendToThingSpeak()`

- Guards against sending while disconnected.
- Builds the update URL with the API key and three fields.
- Issues the GET. If an HTTP reply was received it prints the response body as the "Entry ID" (a numeric entry ID means success); a negative `httpCode` (e.g. `-1`, connection failed) is printed as a failure.
- Closes the connection cleanly (`http.end()`).

> **Quirk:** the sketch treats *any* HTTP reply as "sent". If the write is rejected — e.g. HTTP `401` (bad key) — it still prints `ThingSpeak update sent. Entry ID:` followed by a **non-numeric** body. Treat a non-numeric entry ID as a failure (see [`troubleshooting.md`](./troubleshooting.md)).

## 7. Failure modes (summary)

| Failure | Behaviour |
|---|---|
| WiFi down | Upload skipped; reconnect attempted every loop. Most recent reading is **not** queued locally. |
| DHT22 read fails (`NaN`) | Sample skipped and retried next 20 s. |
| ThingSpeak rejects write | HTTP code printed to Serial Monitor; retried next 20 s. |
| Threshold breached | `field3 = 1`, React fires the email. It stays `1` until temperature returns in range. |

See [`troubleshooting.md`](./troubleshooting.md) for diagnostics and fixes.