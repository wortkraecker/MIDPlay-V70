# PSP Spotify Connect Client

This folder contains a PSP homebrew client that connects to a remote Spotify Connect relay server. The PSP downloads the live MP3 stream, decodes it on device, mixes PCM through `sceAudioOutputPannedBlocking`, and renders metadata and cover art using the PSP GU.

## Layout
```
psp_client/
  src/                # C++ sources
  minimp3/            # Embedded decoder (replace with upstream minimp3.h for full decoding)
  Makefile            # Build instructions for PSPSDK
  EBOOT.PBP           # Generated after running make
```

## Building
1. Install the [PSPSDK](https://github.com/pspdev/pspsdk) toolchain and ensure `psp-g++`, `psp-config`, and `psp-pkg-config` are on `PATH`.
2. Replace `minimp3/minimp3.h` with the official upstream header for real MP3 decoding (the bundled stub outputs silence but keeps interfaces intact for offline authoring). No additional build flags are required for minimp3.
3. Provide a JPEG decoder usable on PSP (e.g., `libjpeg` from the PSPSDK ports) and hook it where noted in `src/ui.cpp` to upload textures for the cover art.
4. Edit `src/main.cpp` and set `SERVER_HOST` to the IP or DNS of your Node.js relay; default port is `4000`.
5. Build the EBOOT:
   ```bash
   cd psp_client
   make
   ```

The build produces `EBOOT.PBP` inside the folder for deployment to `/PSP/GAME/SpotifyConnectPSP/` on a Memory Stick.

## JPEG decoding hook
In `src/ui.cpp` the cover art is currently drawn as a placeholder rectangle. Integrate a PSP-friendly JPEG decoder (e.g., `sceJpeg` or `libjpeg`) to convert the bytes downloaded from `/api/cover` into a texture uploaded with `sceGuTexImage`.

## Notes
- The audio pipeline uses four 4096-sample stereo buffers to avoid underruns.
- Networking relies on the PSP net modules; ensure a network profile is configured.
- Control mapping:
  - Triangle: Play/Pause
  - Square: Previous
  - Cross: Next
  - Circle: Reconnect
  - Up/Down: Volume
  - Start: Quit
- Metadata refreshes every second; cover art refreshes every four seconds or on track change (tracking can be extended in `main.cpp`).
