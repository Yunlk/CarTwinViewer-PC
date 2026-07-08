# CarTwinViewer-PC

Windows host-side viewer for a miniature car digital-twin demo.

This project renders a scrolling road scene, a controlled vehicle, and an obstacle vehicle. It communicates with the STM32 firmware over a serial port so the PC viewer can display the car position and send obstacle position data back to the board.

## Project Role

- Host-side visualization application
- SDL2/SDL2_image based Windows desktop program
- Receives the STM32 car position through UART
- Sends obstacle car position to the STM32 firmware

The matching firmware project is `MiniCar-STM32`.

## Directory Layout

```text
assets/      Image resources used by the viewer
src/         C++ source files
sdl2/        Bundled SDL2 and SDL2_image MinGW libraries
build.ps1    PowerShell build script
CMakeLists.txt
```

## Build

The project expects a MinGW g++ toolchain. By default, `build.ps1` tries:

```powershell
D:\mingw64\bin\g++.exe
```

Build with:

```powershell
.\build.ps1
```

The executable, DLLs, and copied assets are placed under `build/`.

## Runtime Notes

- Serial port defaults to `COM4`.
- Baud rate defaults to `9600`.
- The viewer expects the STM32 firmware to send car coordinates.
- The viewer sends obstacle coordinates to the STM32 firmware.

## Serial Frame

The current protocol follows the original teaching project format:

```text
%xxx\0yyy$
```

where `xxx` is the X coordinate and `yyy` is the Y coordinate. This format is intentionally documented here because both the PC and STM32 projects must stay in sync.

## Current Limitations

- Serial parsing is minimal and assumes aligned frames.
- The serial port is fixed in code.
- The protocol is fragile for negative values and values wider than the expected field.
- This repository currently documents the refactored SDL version only; no new behavior has been implemented as part of this README pass.
