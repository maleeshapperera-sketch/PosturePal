# PosturePal Dashboard

A laptop dashboard that tells you, in plain words, whether the problem right now is
**slouching** or **not moving**. It connects to the same HiveMQ Cloud broker the ESP32
publishes to, so the laptop and the device only need internet access. They don't have to
be on the same WiFi network.

It's a single file (`index.html`), so there's nothing to install.

## What it shows

- **Headline**: "You're slouching. Sit up straight", "You haven't moved in a while",
  "Posture looks good", or both problems together.
- **Posture** and **Movement** cards, each with its own status.
- **Today's stats**: slouch alerts, long slouches (20 s+), idle alerts, approximate time
  spent slouching, and average time to correct.
- **Alerts per hour** chart for today.
- **Event log** with time, event and tilt angle. You can export it as CSV.
- **Desktop notifications** (and an optional beep) when an alert arrives.

## Setup

1. Open `index.html` in Chrome, Edge or Firefox (double-click it).
2. The **Settings** dialog opens. Fill in:
   | Field | Value |
   |---|---|
   | HiveMQ Cloud host | your cluster host, e.g. `xxxx.s1.eu.hivemq.cloud` |
   | WebSocket port | `8884` (browsers use WebSockets; the ESP32 uses 8883) |
   | Username / Password | a HiveMQ Cloud credential with permission to **subscribe** to the topic |
   | Topic | `alert/posture` |
3. Click **Save & connect**. The pill at the top turns green ("Connected to broker").
4. Click **Enable notifications** and allow them.

Settings and the event history are saved in this browser only.

### If notifications don't appear

Some browsers block notifications for pages opened as a file. If that happens, serve the
folder locally instead:

```bash
cd dashboard
python -m http.server 8000
```

Then open <http://localhost:8000>. Settings are stored per address, so you'll need to
enter them once more.

## Things to know

- **Keep the page open** (a background tab is fine). The device doesn't store messages,
  so events sent while the dashboard is closed are not recorded.
- **Idle status clears on its own.** The firmware sends "Idle for too long" but never
  sends a "moving again" message. The dashboard shows *Not moving* for 10 minutes after
  an idle alert, or until you click **I've moved**.
- **Times are approximate.** The firmware sends no timestamps, so each event is stamped
  when the dashboard receives it. Time spent slouching is estimated from the alert
  (fired ~5 s into a slouch) to "Posture Corrected" (sent ~3 s after you sit up).
- **Testing without the device:** open the browser console and run
  `PosturePal.simulate('{"message":"Posture Alert! Incorrect Posture Detected","angle":62}')`.
