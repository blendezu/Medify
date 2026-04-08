# Medify 🚀

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52.svg)
![FFmpeg](https://img.shields.io/badge/FFmpeg-Backend-green.svg)
![Ghostscript](https://img.shields.io/badge/Ghostscript-Backend-yellow.svg)

**Medify** is a modern desktop app built with **C++/Qt6** for processing media and PDF files.  
The focus is on a fast, clean interface and a modular tool architecture.

## Features (currently in the project)

1. **🗜️ Compress Media**  
   Compresses media (e.g., video/image/PDF) to a target size.

2. **📄 PDF to Images**  
   Converts PDF pages into images with configurable output quality (e.g., DPI).

3. **✂️ Split PDF**  
   Extracts selected page ranges from PDF files.

4. **🎵 Extract Audio**  
   Extracts audio tracks from video files and saves them as audio formats.

5. **🎧 Audio Converter**  
   Converts audio files between common formats.

6. **🎬 Video Converter**  
   Converts/transcodes video files into other formats/containers.

## Key Highlights

- **Dashboard UI with tool tiles** for quick access.
- **Modular architecture**: each tool is implemented as an independent component.
- **Dedicated specialized widgets** (e.g., for PDF workflows) alongside a generic tool view.
- **Asynchronous processing** via a worker approach to keep the UI responsive.
- **Clear Qt signal/slot communication** between UI and processing layers.

## Requirements

The following dependencies should be installed and available in your system `PATH`:

- **Qt6** (Core, Gui, Widgets)
- **CMake** (>= 3.16)
- **FFmpeg + FFprobe**
- **Ghostscript (`gs`)**

Example (macOS/Homebrew):

```bash
brew install qt6 ffmpeg ghostscript
```

## Build

```bash
git clone https://github.com/blendezu/Medify.git
cd Medify

mkdir build && cd build
cmake ..
cmake --build .

./Medify
```

## Note

Additional technical details can be documented and maintained in the corresponding tool modules and internal docs.
