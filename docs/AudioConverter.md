# Audio Converter

## Overview
The **Audio Converter** facilitates format bridging between multiple audio codecs like `mp3`, `wav`, `flac`, and `ogg`.

## Implementation Details
This tool actually shadows the `processExtractAudio()` function identically behind the scenes.
By feeding audio inputs into `ffmpeg` alongside standard extension adjustments from the UI components' `QComboBox`, FFmpeg natively identifies the container shift and effectively transcodes the audio bits cleanly to the requested destination framework under standard settings.
