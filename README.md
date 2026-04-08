# Medify 🚀
![C++](https://img.shields.io/badge/C++-17-blue.svg) ![Qt6](https://img.shields.io/badge/Qt-6.x-41CD52.svg) ![FFmpeg](https://img.shields.io/badge/FFmpeg-Backend-green.svg) ![Ghostscript](https://img.shields.io/badge/Ghostscript-Backend-yellow.svg)

**Medify** is a sleek, dashboard-driven desktop application built with C++ and Qt6 that provides a massive suite of tools for processing and manipulating media and PDFs. It features a fully asynchronous, modular architecture to ensure the UI stays buttery smooth while executing resource-intensive compression, extraction, and formatting tasks.

## 🛠️ Features / Tools

1. **🗜️ Compress Media:** Dynamically compress Videos, PDFs, and Images down to a specific target byte size using precision binary search and target-bitrate heuristics.
2. **📄 PDF to Images:** Rasterize and split mult-page PDFs into high-quality images with customized DPI settings.
3. **🖼️ Images to PDF:** Merge an ordered list of various visual formats (.jpg, .png, etc.) sequentially into a single PDF document.
4. **✂️ Split PDF:** Extract exact page-ranges from monolithic PDFs without quality loss.
5. **🎵 Extract Audio:** Strips out the audio tracks of video containers (MP4, MKV) and exports them cleanly to `.mp3` or `.wav`.
6. **🎧 Audio Converter:** Convert between major high-fidelity audio formats seamlessly.
7. **🎬 Video Converter:** Multiplex and transcode video files between different container formats.

## 🔌 Architecture

Medify runs completely on a highly extensible **Plugin-Oriented Architecture**:
- **`ITool` Interface:** Every tool acts as an autonomous UI and settings-generation widget.
- **`BaseWorker` Abstract Component:** Every tool possesses an isolated background threaded worker interacting securely with the user interface via signal/slot emissions, ensuring no cross-tool contamination.

Detailed technical logic for each component can be found in the `/docs` folder.

## ⚙️ Prerequisites
To build and run Medify, ensure you have the following system dependencies installed and accessible in your system `PATH`:
- **Qt6** (Core, Gui, Widgets modules)
- **CMake** (v3.16+)
- **FFmpeg & FFprobe** (For core Audio/Video handling)
- **Ghostscript (`gs`)** (For optimal PDF formatting and modification)

_If you are on macOS, these dependencies can be installed easily using Homebrew:_
```bash
brew install qt6 ffmpeg ghostscript
```

## 🚀 Build Instructions
Clone the repository and build the project utilizing CMake:

```bash
git clone https://github.com/blendezu/Medify.git
cd Medify

# Generate Build System
mkdir build && cd build
cmake ..

# Compile the Engine
cmake --build .

# Run the Toolkit
./Medify
```
