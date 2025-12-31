# ClickPaste for Linux

A KDE Plasma/Wayland utility that pastes clipboard contents as simulated keystrokes.

## About

This is a reverse-engineered port of [ClickPaste for Windows](https://github.com/Collective-Software/ClickPaste) from C#/.NET to C++/Qt6. The original Windows application uses Win32 APIs and AutoIt for input simulation - this Linux version achieves the same functionality using native Wayland protocols (libei, Layer Shell) and KDE Frameworks.

**Primary use case**: Pasting text into applications that don't support traditional clipboard paste - such as KVM switches, remote desktop sessions, virtual machines, or legacy terminal applications that intercept Ctrl+V.

## Features

- **System Tray Integration**: Lives in your system tray for easy access
- **Two Operation Modes**:
  - **Target Mode**: Click on the tray icon or press the hotkey, then click on the target window to paste
  - **Just Go Mode**: Press the hotkey to immediately type to the focused window
- **Configurable Hotkey**: Default is Ctrl+Alt+V
- **Adjustable Delays**: Configure start delay and per-keystroke delay
- **Confirmation Dialog**: Optional confirmation for large text pastes
- **Escape to Cancel**: Press Escape at any time to stop typing

## Requirements

### Build Dependencies

```bash
# Fedora/RHEL
sudo dnf install cmake extra-cmake-modules qt6-qtbase-devel kf6-kglobalaccel-devel layer-shell-qt-devel libei-devel

# Arch Linux
sudo pacman -S cmake extra-cmake-modules qt6-base kglobalaccel layer-shell-qt libei

# Ubuntu/Debian (24.04+)
sudo apt install cmake extra-cmake-modules qt6-base-dev libkf6globalaccel-dev layer-shell-qt-dev libei-dev
```

### Runtime Dependencies

- KDE Plasma 6 (Wayland session)
- xdg-desktop-portal-kde
- PipeWire

## Building

```bash
cd ~/Projects/clickpaste-linux
mkdir build && cd build
cmake ..
make
```

## Installation

### Arch Linux (AUR)

```bash
# Using yay
yay -S clickpaste

# Using paru
paru -S clickpaste

# Manual
git clone https://aur.archlinux.org/clickpaste.git
cd clickpaste
makepkg -si
```

### From Source

```bash
sudo make install
```

Or run locally without installing:

```bash
./clickpaste
```

### Why not Flatpak?

ClickPaste requires deep system integration that Flatpak's sandbox prevents:
- **libei**: Input injection requires direct compositor access
- **KGlobalAccel**: Global hotkeys need system-level registration
- **Layer Shell**: Overlay windows require compositor protocols

A Flatpak would need so many sandbox holes it would defeat the purpose of sandboxing.

## Usage

1. **Start ClickPaste**: Run `clickpaste` or find it in your application menu
2. **The tray icon** appears in your system tray
3. **Copy text** to your clipboard as normal
4. **Trigger paste** by either:
   - Clicking the tray icon (Target Mode)
   - Pressing Ctrl+Alt+V (default hotkey)
5. **In Target Mode**: Click on the window where you want to type
6. **Watch it type**: ClickPaste will type the clipboard contents character by character

### Settings

Right-click the tray icon and select "Settings" to configure:

- **Delays**: Adjust timing between keystrokes
- **Confirmation**: Enable prompts for large pastes
- **Hotkey**: Change the keyboard shortcut
- **Mode**: Choose between Target and Just Go modes

## How It Works

On Wayland, ClickPaste uses:

- **libei**: Modern Wayland input emulation library for keystroke injection
- **KGlobalAccel**: KDE's global hotkey system
- **Layer Shell**: Wayland protocol for the targeting overlay
- **XDG Desktop Portal**: For permission management

## Troubleshooting

### "Failed to initialize input emulation"

Your compositor must support libei. This requires:
- KDE Plasma 6.0 or later
- xdg-desktop-portal-kde installed and running

### Hotkey not working

1. Check that no other application is using the same hotkey
2. Try changing the hotkey in Settings
3. Ensure KGlobalAccel is running: `systemctl --user status plasma-kglobalaccel.service`

### Typing is slow or unreliable

- Increase the key delay in Settings
- Some applications may need longer delays to process input

## License

BSD 3-Clause License (same as the original Windows ClickPaste)

## Credits

- Original Windows ClickPaste by Collective Software, LLC
- Linux port designed for KDE Plasma 6 / Wayland
