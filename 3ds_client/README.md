# 3DS Spotify Connect Client (Homebrew)

This homebrew targets the Nintendo 3DS family and behaves as a Spotify Connect client talking to your remote VPS stack (librespot → ffmpeg → Node.js) that exposes:

- `GET /stream.mp3` (live MP3 stream at 160kbps CBR)
- `GET /api/status`
- `GET /api/metadata`
- `GET /api/cover`
- `POST /api/play|pause|next|prev|seek|volume`

## Features
- Wi-Fi setup and connection (via `socInit` + `ndspInit`).
- Continuous MP3 streaming over HTTP with auto-reconnect, using minimp3 on-device decoding.
- Ring-buffered PCM feeding NDSP with four 4096-sample stereo buffers to avoid underruns.
- Cooperative multitasking: network + decoder + UI refresh + metadata polling.
- Citro2D UI showing cover art, track/artist, timer, transport controls, volume bar, and connection state.
- JPEG cover decoding via `stb_image` (CPU-side), turned into a Citro2D image.
- Buttons: `Y` play/pause, `X` previous, `A` next, D-Pad up/down for volume, `B` reconnect stream, `START` quits.

## Building
1. Install [devkitPro](https://devkitpro.org/wiki/Getting_Started) with devkitARM, libctru, citro2d/citro3d, and zlib. Install bannertool and makerom if you want a CIA.
2. Place `minimp3.h` inside `minimp3/` (already stubbed here; replace with upstream if desired) and drop the full upstream `stb_image.h` into `src/` to enable JPEG cover decoding (the included stub only compiles; it does not decode).
3. (Optional) Add romfs assets: `romfs/app.icon.png`, `romfs/banner.png`, `romfs/banner_audio.wav`, and `cia.rsf` for custom title info.
4. Build:
   ```bash
   cd 3ds_client
   make        # builds 3ds_spotify.3dsx
   make cia    # builds a CIA if bannertool+makerom are present
   ```

## Running
Copy `3ds_spotify.3dsx` to your SD (`/3ds/` homebrew folder) or install the CIA via FBI. Ensure the console is connected to the same network as the server and update `SERVER_IP`/`SERVER_PORT` in `src/utils.h`.

## Notes
- Audio uses NDSP in stereo 16-bit PCM, 44.1kHz.
- Metadata polling is once per second; cover art is refreshed on track change only.
- The code avoids dynamic allocations in the hot path; network dropouts trigger a reconnect to `/stream.mp3` while UI stays responsive.
