# PosturePal

A posture monitor built on an ESP32 and an MPU9250 motion sensor. The device buzzes when you slouch or sit still too long, and publishes each event to a HiveMQ Cloud broker over MQTT. A web dashboard reads those events and keeps a record of your day.

The firmware is in `posturepal.c`. More detail on the hardware and wiring is in `README2.md`.

## Dashboard

`dashboard/index.html` is the whole dashboard: one file, no build step, no server. Open it in a browser on your laptop.

What it shows:

- A headline that says whether the problem right now is slouching, not moving, or both
- Separate Posture and Movement cards
- Today's slouch alerts, long slouches (20 s+), idle alerts, time spent slouching, and average time to correct
- Alerts per hour for today
- An event log with CSV export, and optional desktop notifications

Setup steps and limits are in [`dashboard/README.md`](dashboard/README.md).
