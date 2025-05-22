# ModernMediaPlayer 🎬

A modern, feature-rich media player built with C++ and Qt, designed to provide a better user experience than other media players with a sleek UI and advanced functionality.

## Features ✨

- **Modern UI/UX**
  - Sleek dark/light theme
  - Intuitive controls with animations
  - Fullscreen mode with gesture support
  - System tray integration

- **Media Playback**
  - Supports most video/audio formats (MP4, AVI, MKV, MP3, FLAC, etc.)
  - Hardware-accelerated decoding
  - Playback speed control (0.5x-2.0x)
  - Frame-by-frame seeking

- **Advanced Features**
  - Playlist management
  - Audio equalizer (10-band)
  - Subtitles support
  - Streaming from URLs
  - Recent files history

- **Customization**
  - Multiple color themes
  - Adjustable UI layout
  - Custom keyboard shortcuts

## Installation 🛠️

### Prerequisites
- Qt 6.5+ with Multimedia module
- C++17 compatible compiler
- CMake 3.16+

### Build Instructions
```bash
git clone https://github.com/iyoramu/modern-media-playe.git
cd ModernMediaPlayer
mkdir build && cd build
cmake ..
cmake --build .
```

### Run
```bash
./ModernMediaPlayer
```

## Usage 🎮

### Basic Controls
- Space: Play/Pause
- Left/Right Arrow: Seek backward/forward 5 seconds
- Up/Down Arrow: Increase/Decrease volume
- F: Toggle fullscreen
- M: Mute/unmute
- N/P: Next/Previous track

### Playlist Management
- Drag and drop files to add to playlist
- Double-click items to play
- Right-click for context menu options

## Configuration ⚙️

Settings are automatically saved in:
- Linux: `~/.config/ModernMediaPlayer`
- Windows: `%APPDATA%\ModernMediaPlayer`
- macOS: `~/Library/Preferences/ModernMediaPlayer`

## Roadmap 🗺️

### Planned Features
- [ ] Chromecast support
- [ ] Video filters (brightness, contrast, etc.)
- [ ] Audio visualization
- [ ] Plugin system
- [ ] Mobile version (Android/iOS)

## Contributing 🤝

We welcome contributions! Please follow these steps:

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License 📄

This project is licensed under the GPL-3.0 License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments 🙏

- Qt Project for the amazing framework
- VLC team for inspiration
- FFmpeg for media decoding

---
