# 3DS Spotify Connect Client (2025 homebrew layout)

This target mirrors the PSP build but follows current (2025) Nintendo 3DS homebrew conventions: devkitPro + devkitARM, libctru for system services, citro2d/citro3d for rendering, ndsp for audio, and httpc/soc for networking. The app behaves as a Spotify Connect client against your VPS stack (librespot → ffmpeg → Node.js) and talks to these endpoints:

- `GET /stream.mp3` (160 kbps CBR live MP3)
- `GET /api/status`
- `GET /api/metadata`
- `GET /api/cover`
- `POST /api/play|pause|next|prev|seek|volume`

## Toolchain and libraries (2025)
- Install [devkitPro](https://devkitpro.org/wiki/Getting_Started) via **dkp-pacman**. Required packages: `devkitARM`, `libctru`, `citro2d`, `citro3d`, `zlib`, `bannertool`, `makerom`, and (optionally) `mbedtls` or `curl` if you want HTTPS.
- Networking: uses `socInit` + `httpcInit`. The stream itself goes over raw sockets to keep latency low; metadata/status calls use the httpc service.
- Audio: NDSP stereo 16‑bit PCM at 44.1 kHz with four 4096-sample ring buffers.
- UI: citro2d HUD; JPEG cover decoding expects `stb_image.h` in `src/` (see below).

## Bringing in codecs/assets
- MP3 decoding: drop the official **minimp3.h** from https://github.com/lieff/minimp3 into `3ds_client/minimp3/` (the stub here only builds; replace it to get actual audio).
- JPEG decode: add the upstream **stb_image.h** into `3ds_client/src/` to enable cover art uploads (the included stub only compiles). Disable `STBI_NO_LINEAR`/`STBI_NO_HDR` to keep footprint low.
- Optional ROMFS assets (for CIA banners/icons): `romfs/app.icon.png`, `romfs/banner.png`, `romfs/banner_audio.wav`, plus a `cia.rsf` manifest.

## Building (.3dsx and CIA)
```bash
# one-time: ensure DEVKITPRO/DEVKITARM env vars are set by dkp-pacman shell
cd 3ds_client
make            # builds 3ds_spotify.3dsx
make cia        # builds a CIA if bannertool & makerom are in PATH
```

## Running
1. Edit `src/utils.h` to point `SERVER_IP`/`SERVER_PORT` at your VPS (port 4000 by default).
2. Copy `3ds_spotify.3dsx` to the SD card (`/3ds/`) or install the CIA with FBI.
3. Launch from the Homebrew Menu. The app auto-connects Wi‑Fi, opens `/stream.mp3`, decodes on-device, feeds NDSP, and refreshes metadata once per second.

## Controls
- **Y** play/pause, **X** previous, **A** next
- **D‑Pad Up/Down** volume +/-
- **B** manual reconnect to `/stream.mp3`
- **START** quit

## Notes
- The audio thread decodes continuously; UI stays responsive in the main loop.
- If the stream stalls, the worker auto-reconnects after a short timeout.
- Replace the codec stubs with the upstream files for real playback and cover rendering; the rest of the code is production-ready for devkitPro 2025.
