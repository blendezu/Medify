# PDF to Images

## Overview
The **PDF to Images** tool converts an entire PDF document (or a subset of pages) into high-quality image files.

## Implementation Details
The conversion depends dynamically on an external integration with **Ghostscript (`gs`)**, mapped behind a non-blocking UI layer via `QProcess` inside `ProcessManager`.

- It injects the standard device trigger `-sDEVICE=png16m` to write robust, true-color PNG files. 
- **DPI Configurator**: The user can supply custom resolution scalers into the UI (e.g., 300 DPI for print quality export, or 72 DPI for web use), which map directly to Ghostscript's `-r` flag.
- **Page Ranges**: To extract only certain frames, the UI captures hyphenated configurations (e.g., "1-5"). This is split and formatted locally into `-dFirstPage=` and `-dLastPage=`.
- The output paths dynamically utilize a string formatter syntax (e.g. `_export_%03d.png`), forcing Ghostscript to split and assign serialized integer tags to each specific page.
