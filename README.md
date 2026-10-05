# PosturePal

A posture monitor built on an ESP32 and an MPU9250 motion sensor. The device buzzes when you slouch or sit still too long, and publishes each event to a HiveMQ Cloud broker over MQTT. A web dashboard reads those events and keeps a record of your day.

The firmware is in `posturepal.c`. More detail on the hardware and wiring is in `README2.md`.

## Dashboard

`index.html` is the whole dashboard: one file, no build step, no server.

What it shows:

- Whether you are sitting fine or slouching, with a live timer while a slouch lasts
- A "Time to move" card when the device sends an idle alert
- Today's posture score, slouch count, time spent slouching, idle alerts, and monitored time
- A timeline of the day, slouches by hour, and a 7-day score history
- An event log with CSV export, and optional browser notifications

### Using it

1. Open the dashboard page (GitHub Pages: `https://maleeshapperera-sketch.github.io/PosturePal/`).
2. Tap the gear icon and enter a HiveMQ username and password. Use a user that can only subscribe to `alert/posture`.
3. The host (`...s1.eu.hivemq.cloud`), WebSocket port `8884`, path `/mqtt` and topic `alert/posture` are already filled in.

Add `?demo` to the URL to try it with simulated data and no device.

### Limits

- History is saved in the browser on the device you use. It only records while the page is open and connected.
- Slouch durations are estimates, because the firmware only reports a slouch about 5 seconds after it starts and a correction 3 seconds after you sit up.
- Credentials entered in the settings stay in that browser's local storage and are never written to the repo.
