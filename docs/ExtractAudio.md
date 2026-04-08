# Extract Audio

## Overview
The **Extract Audio** tool analyzes video containers (`mp4`, `mkv`, `avi`, etc.) and isolates the internal audio track, discarding the video stream to produce smaller, standalone audio files.

## Implementation Details
Video decoding tasks are routed linearly out to `ffmpeg`.

- The UI dropdown lets the user specify an output format requirement (usually `.mp3` or `.wav`).
- To drop tracking, the `-vn` (*no video*) argument is supplied immediately to FFmpeg.
- **MP3 vs. WAV Transcoding**: If passing to standard high-quality MP3 (`libmp3lame`), it encodes directly over `-q:a 2` which delivers exceptional dynamic bitrate optimization. If the user desires lossless extraction (`wav`), it remaps cleanly against `-acodec pcm_s16le`.
