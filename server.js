const express = require("express");
const { spawn } = require("child_process");
const path = require("path");

const app = express();

const PORT = Number(process.env.PORT) || 4000;
const LIBRESPOT_BIN = process.env.LIBRESPOT_BIN || "/usr/local/bin/librespot";
const FFMPEG_BIN = process.env.FFMPEG_BIN || "ffmpeg";
const MP3_BITRATE = process.env.MP3_BITRATE || "160k";

// Track metadata that PSP clients can poll from /status.json
let currentTrack = {
  artist: "Spotify",
  title: "Awaiting Playback...",
};

// Keep a rolling buffer of librespot logs for remote debugging
const MAX_LOG_LINES = Number(process.env.LOG_LINES || 200);
const librespotLog = [];

const LIBRESPOT_ARGS = [
  "--name",
  "MidplayPSP",
  "--backend",
  "pipe",
  "--cache",
  "/home/midplay/cache",
  "--enable-volume-normalisation",
  "--initial-volume",
  "75",
  "--bitrate",
  "160",
  "--verbose",
];

const FFMPEG_ARGS = [
  // IMPORTANT: -re is what stops Spotify from racing through tracks.
  // It tells ffmpeg to read stdin at real-time speed instead of as fast as
  // possible, which lets librespot keep a normal playback clock.
  "-re",
  "-f",
  "s16le",
  "-ar",
  "44100",
  "-ac",
  "2",
  "-i",
  "pipe:0",
  "-vn",
  "-c:a",
  "libmp3lame",
  "-b:a",
  MP3_BITRATE,
  "-content_type",
  "audio/mpeg",
  "-f",
  "mp3",
  "pipe:1",
];

// Append a line to the log buffer, trimming to MAX_LOG_LINES
const recordLibrespotLog = (line) => {
  librespotLog.push({
    time: new Date().toISOString(),
    line,
  });

  if (librespotLog.length > MAX_LOG_LINES) {
    librespotLog.splice(0, librespotLog.length - MAX_LOG_LINES);
  }
};

app.get("/", (req, res) => {
  res.type("text/plain").send(
    "Midplay PCM→MP3 stream server is running.\nUse /stream.mp3 for audio, /status.json for metadata, and /logs.html to view librespot logs."
  );
});

// Lightweight metadata endpoint for the PSP client
app.get("/status.json", (req, res) => {
  res.writeHead(200, {
    "Content-Type": "application/json",
    "Cache-Control": "no-cache",
  });
  res.end(JSON.stringify(currentTrack));
});

// Structured log feed for remote troubleshooting
app.get("/logs.json", (req, res) => {
  res.json({
    currentTrack,
    log: librespotLog,
  });
});

// Simple HTML status page that polls /logs.json
app.get("/logs.html", (req, res) => {
  res.sendFile(path.join(__dirname, "res", "logs.html"));
});

app.get("/stream.mp3", (req, res) => {
  console.log(`Client connected to /stream.mp3 from ${req.ip}`);

  res.writeHead(200, {
    "Content-Type": "audio/mpeg",
    "Cache-Control": "no-store",
    Connection: "close",
    "Transfer-Encoding": "chunked",
  });

  const librespot = spawn(LIBRESPOT_BIN, LIBRESPOT_ARGS, {
    stdio: ["ignore", "pipe", "pipe"],
  });

  const ffmpeg = spawn(FFMPEG_BIN, FFMPEG_ARGS, {
    stdio: ["pipe", "pipe", "pipe"],
  });

  console.log(`Spawned librespot pid=${librespot.pid}, ffmpeg pid=${ffmpeg.pid}`);

  // Pipe decoded PCM into the encoder
  librespot.stdout.pipe(ffmpeg.stdin);

  // Forward encoded MP3 to HTTP response with backpressure handling
  ffmpeg.stdout.pipe(res);

  const closeStream = (reason) => {
    console.log(`Closing stream: ${reason}`);
    if (!res.writableEnded) {
      res.end();
    }
    ffmpeg.kill("SIGINT");
    librespot.kill("SIGINT");
  };

  // Surface stderr from both processes for visibility
  librespot.stderr.on("data", (data) => {
    const logLine = data.toString();

    // librespot logs PLAYING 'Title' by 'Artist' with Spotify URI ...
    const match = logLine.match(/PLAYING '(.+?)'(?: by '(.+?)')? with Spotify URI/);
    if (match) {
      const [, title, artist] = match;
      currentTrack = {
        title: (title || currentTrack.title).trim(),
        artist: (artist || currentTrack.artist).trim(),
      };
      console.log(`[Metadata Update] ${currentTrack.artist} - ${currentTrack.title}`);
    }

    logLine
      .split(/\r?\n/)
      .filter(Boolean)
      .forEach((line) => recordLibrespotLog(line));

    process.stderr.write(data);
  });
  ffmpeg.stderr.on("data", (data) => process.stderr.write(data));

  librespot.on("close", (code, signal) => {
    console.log(`librespot exited code=${code} signal=${signal}`);
    closeStream("librespot closed");
  });

  ffmpeg.on("close", (code, signal) => {
    console.log(`ffmpeg exited code=${code} signal=${signal}`);
    closeStream("ffmpeg closed");
  });

  req.on("close", () => {
    console.log("HTTP client disconnected, terminating processes");
    closeStream("client disconnected");
  });
});

app.listen(PORT, () => {
  console.log(`Midplay MP3 stream server listening on :${PORT}`);
  console.log("Open /stream.mp3 from your PSP client to start audio.");
});
