# Images to PDF

## Overview
The **Images to PDF** tool merges multiple uploaded image files across the file system into a single synchronized PDF document.

## Implementation Details
Unlike most of the dashboard tools, this feature performs perfectly natively within C++ without requiring `QProcess` interactions with Ghostscript or `ffmpeg`.

It establishes a `QPdfWriter` directed at the save-directory threshold, initializing a continuous `QPainter` canvas pipeline:
1. Every distinct file inside the user's `QListWidget` generates an independently framed PDF canvas.
2. Before injection, the script invokes `pdfWriter.newPage()` to enforce strict sequential pagination.
3. The image is parsed via `QImage` logic and safely injected precisely across the canvas geometry using `qt_painter_draw_image` and `painter.drawImage()`, dynamically scaling while retaining smooth anti-aliased output natively in the Qt renderer engine.
