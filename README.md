# MIDPlay

A feature-rich online music player application developed using J2ME (Java Micro Edition) for mobile devices.

![AppIcon](/res/Icon.png)

## Features

- Stream music from multiple sources (NCT, SoundCloud, YouTube Music, Spotify)
- Browse music by categories and playlists
- Search for songs, artists, and albums
- Create and manage favorites
- Multi-language support

## System Requirements

- Device supporting J2ME MIDP 2.0
- CLDC 1.1
- Network connectivity for streaming

## Installation

1. Download the latest `.jar` file from the [Releases](https://github.com/phd051199/MIDPlay/releases) page
2. Install on your J2ME compatible device

## Usage

1. Launch the application
2. Navigate using the main menu
3. Browse categories or search for music
4. Select a song to play
5. Use player controls to manage playback

## Building from Source

```bash
# Requires NetBeans with Mobility Pack or equivalent J2ME development tools
ant build
```

## Running the streaming server as a service

The included `server.js` can be kept alive in the background with a systemd unit so it restarts automatically if it crashes or you disconnect your SSH session.

1. Install the Node dependency once on the server:
   ```bash
   npm install --production
   ```
2. Copy the service template and edit paths/users to match your deployment (the defaults assume the repo lives in `/opt/midplay` and runs as user `midplay`):
   ```bash
   sudo cp tools/midplay-stream.service /etc/systemd/system/midplay-stream.service
   sudo nano /etc/systemd/system/midplay-stream.service
   ```
3. Reload systemd, enable on boot, and start now. The unit uses `Restart=always` and `RestartSec=10` so it will restart 10 seconds after any crash:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable --now midplay-stream.service
   ```
4. Check status and logs:
   ```bash
   systemctl status midplay-stream.service
   journalctl -u midplay-stream.service -f
   ```

   The running server also exposes a lightweight status page at `http://<host>:<port>/logs.html` (or JSON at `/logs.json`) so you can view librespot logs and the current track without SSH.

Modify the `Environment=` lines in the service file if you need to point at different binaries or change the listening port/bitrate.

## Technologies Used

- Java ME (J2ME) MIDP 2.0
- CLDC 1.1
- Record Management System (RMS) for local storage
- RESTful API connections for music streaming

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Features and Code Contributions

1. Fork the repository.
2. Create a feature branch:  
   `git checkout -b feature/amazing-feature`
3. Commit your changes:  
   `git commit -m "Add some amazing feature"`
4. Push to the branch:  
   `git push origin feature/amazing-feature`
5. Open a Pull Request.

### Language Contributions

If you want to add a new language, you can do so by following these steps:

- Duplicate the existing **`en.json`** file as a base
- Edit the file as needed for your language
- Submit it either:
  - As an issue with the `[Enhancement]` tag and a detailed description, or
  - As a Pull Request (as described in the previous section)

## Author and Contributors

**Author:** Duy Pham

**Contributors:** symbuzzer, GoldenDragon
