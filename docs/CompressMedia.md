# Compress Media

## Overview
The **Compress Media** tool allows users to reduce the file size of various media types (Images, Audio, Video, and PDFs) by targeting an approximate destination file size (in Megabytes).

## Implementation Details
This tool maps a user's linear target size percentage (from 10% to 100% of original bytes) into an absolute target byte ceiling.

- **Audio & Video**: The core implementation depends on `ffprobe` and `ffmpeg`. First, it makes a synchronous `QProcess` call to `ffprobe` to ascertain the exact duration of the media file. Because file size is directly correlated with duration and bitrate (`Size = Bitrate * Duration`), the internal logic algebraically determines the exact target bitrate (`-b:v`, `-b:a`) required to hit the user's requested byte limit. For mixed media (Video), 128kbps is routinely reserved for the audio channel, while the remainder is mapped to the video compression bitrate using a standard `libx264` codec container limit.
- **Images (JPEG/PNG)**: Since image compression isn't predictably linear based on file-size bytes natively, this component relies on an in-memory **binary search**. It leverages Qt's standard `QImageReader` and `QImageWriter`. The worker iteratively tests quality coefficients (1 to 100) writing into a pure RAM `QByteArray` buffer until it identifies the exact mathematical quality setting that produces a file nearest to the target bytes.
- **PDF**: Given the restrictions of post-compiled PDF formats, exact byte targeting is bypassed. Instead, the `TargetSize` string maps locally to predefined rendering endpoints through Ghostscript: `/screen`, `/ebook`, `/printer`, or `/prepress`.
