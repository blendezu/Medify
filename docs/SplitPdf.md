# Split PDF

## Overview
The **Split PDF** tool isolates specific pages or sections from an existing PDF document, generating a new condensed PDF copy from those boundaries without quality loss.

## Implementation Details
Operations invoke Ghostscript (`gs` / `gswin64c`) locally through the `ProcessManager`.

- Leveraging `-sDEVICE=pdfwrite`, Ghostscript safely rebuilds and transposes the raw object binaries.
- Standard ranges supplied by the user (ex: `2-5`) are verified by QString parsers, establishing exact bounds passed to Ghostscript through the strict properties `-dFirstPage=` and `-dLastPage=`.
- The worker executes strictly in non-blocking fashion inside `QThread`, ensuring the MainUI does not halt during extensive document reconstructions.
