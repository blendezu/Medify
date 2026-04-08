# Video Converter

## Overview
The **Video Converter** converts video files seamlessly between standard container boundaries like `mp4`, `mkv`, `avi`, and `mov`. 

## Implementation Details
The `ProcessManager` intercepts the selected container and overrides the original extension format in a synchronous `ffmpeg` push execution.

- Video codecs are completely standardized automatically pushing universally accepted standard `libx264` via the generic parameter configuration `-c:v libx264`.
- Audio streams are identically standard-verified mapped into `-c:a aac` utilizing 128kbps limits ensuring high playback consistency.
- Quality boundaries are locked to a visually pristine `crf` coefficient of `21`, guaranteeing that container shifts encode seamlessly without artifacts.
